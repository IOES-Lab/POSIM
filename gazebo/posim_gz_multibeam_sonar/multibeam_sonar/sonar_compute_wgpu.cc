#include "sonar_compute_wgpu.hh"
#include "PhysicalSonarInput.hh"

#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdint>
#include <iostream>
#include <vector>

extern "C"
{
  float * sonar_wgpu_compute_v2(
    const float * depth, const float * normal, const float * reflectivity,
    const float * angular_area, const float * pressure_response, const float * window,
    uint32_t n_geometry, uint32_t n_selected_rays, uint32_t n_receivers, uint32_t n_freq,
    float sound_speed, float max_distance, float source_level, float attenuation,
    float receive_gain, float bandwidth, uint64_t frame_index, uint64_t seed, bool debug_flag);

  float * sonar_wgpu_compute(
    const float * depth_flat, const float * normal_flat, const float * reflectivity_flat,
    const float * beam_corrector_flat, const float * window_flat, uint32_t n_beams, uint32_t n_rays,
    uint32_t n_freq, uint32_t ray_skips, float sound_speed, float max_distance, float source_level,
    float attenuation, float sensor_gain, float bandwidth, float beam_corrector_sum,
    float area_scaler, float h_fov, float v_fov, uint64_t frame_index, uint64_t seed,
    bool debug_flag);

  void sonar_wgpu_free(float * ptr, size_t len);
}

namespace gz
{
namespace sensors
{

const char * WgpuComputeBackend::Name() const { return "wgpu"; }

bool WgpuComputeBackend::Initialize(const SonarComputeInput & prototype)
{
  if (prototype.physicalReadout)
  {
    // Exercise v2 with its actual shape and transform length. An adapter-only
    // legacy probe cannot establish physical-model or exact-length support.
    this->initialized = true;
    SonarComputeOutput probeOutput;
    this->initialized = this->Compute(prototype, probeOutput);
    if (!this->initialized)
    {
      std::cerr << "[sonar_wgpu] Physical readout probe failed (geometry "
                << prototype.physicalReadout->geometryColumns << " x " << prototype.nRays
                << ", receivers " << prototype.nBeams << ", frequencies " << prototype.nFreq
                << "). Check backend transform support and input parameters." << std::endl;
    }
    return this->initialized;
  }
  // Probe the Rust FFI to check if a GPU adapter is available. If it is not, report failure
  // so the factory can pick another backend instead of silently computing on the CPU.
  std::vector<float> depth(1, 0.0f);
  std::vector<float> normal(3, 0.0f);
  std::vector<float> reflectivity(1, 0.0f);
  std::vector<float> beamCorrector(1, 1.0f);
  std::vector<float> window(4, 1.0f);

  float * probe = sonar_wgpu_compute(
    depth.data(), normal.data(), reflectivity.data(), beamCorrector.data(), window.data(), 1u, 1u,
    4u, 0u, 1500.0f, 10.0f, 220.0f, 0.0f, 1.0f, 29500000.0f, 1.0f, 1e-6f, 0.0f, 0.0f, 0u, 1u,
    false);

  if (!probe)
  {
    std::cerr << "[sonar_wgpu] GPU probe failed: no usable WGPU adapter." << std::endl;
    return false;
  }
  sonar_wgpu_free(probe, 1u * 4u * 2u);

  this->initialized = true;
  return true;
}

bool WgpuComputeBackend::Compute(const SonarComputeInput & input, SonarComputeOutput & output)
{
  if (!this->initialized)
  {
    return false;
  }

  if (input.physicalReadout)
  {
    const auto start = std::chrono::steady_clock::now();
    PackedPhysicalSonar cells;
    if (!PackPhysicalSonar(input, cells))
    {
      return false;
    }
    float * result = sonar_wgpu_compute_v2(
      cells.depth.data(), cells.normal.data(), cells.reflectivity.data(), cells.angularArea.data(),
      input.physicalReadout->pressureResponse.data(), input.window, cells.geometryColumns,
      cells.selectedRows, input.nBeams, input.nFreq, input.soundSpeed, input.maxDistance,
      input.sourceLevel, input.attenuation, input.receiveGain, input.bandwidth, input.frameIndex,
      input.seed, input.debugFlag);
    if (!result)
    {
      return false;
    }
    output.nBeams = input.nBeams;
    output.nFreq = input.nOutputRanges ? input.nOutputRanges : input.nFreq;
    output.beamSpectrum.resize(size_t(input.nBeams) * output.nFreq);
    for (int beam = 0; beam < input.nBeams; ++beam)
    {
      for (int range = 0; range < output.nFreq; ++range)
      {
        const size_t i = size_t(beam) * input.nFreq + range;
        output.At(beam, range) = {result[2 * i], result[2 * i + 1]};
      }
    }
    sonar_wgpu_free(result, size_t(input.nBeams) * input.nFreq * 2);
    output.computeMicros =
      std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count();
    return true;
  }

  if (!input.depthImage || !input.normalImage || !input.reflectivityImage)
  {
    return false;
  }

  const cv::Mat & depth = *input.depthImage;
  const cv::Mat & normal = *input.normalImage;
  const cv::Mat & reflectivity = *input.reflectivityImage;

  if (depth.empty() || normal.empty() || reflectivity.empty())
  {
    return false;
  }

  const int nBeams = input.nBeams;
  const int nRays = input.nRays;
  const int nFreq = input.nFreq;

  if (nBeams <= 0 || nRays <= 0 || nFreq <= 0)
  {
    return false;
  }

  auto start = std::chrono::high_resolution_clock::now();

  // Compute area_scaler matching CUDA: ray_azimuthAngleWidth * ray_elevationAngleWidth
  // where ray_azimuthAngleWidth = hPixelSize
  //       ray_elevationAngleWidth = vPixelSize * (raySkips + 1)
  const float hPixelSize = static_cast<float>(input.hFOV) / std::max(1, nBeams - 1);
  const float vPixelSize = static_cast<float>(input.vFOV) / std::max(1, nRays - 1);

  const int raySkipsFactor = input.raySkips + 1;
  const float area_scaler = hPixelSize * vPixelSize * raySkipsFactor;

  // This transposes from OpenCV's [row=ray][col=beam] to [beam][ray] for WGPU.
  // Flatten as [beam][ray].
  std::vector<float> depthFlat(static_cast<size_t>(nBeams) * nRays, 0.0f);
  std::vector<float> reflFlat(static_cast<size_t>(nBeams) * nRays, 0.0f);
  std::vector<float> normalFlat(static_cast<size_t>(nBeams) * nRays * 3, 0.0f);

  for (int beam = 0; beam < nBeams; ++beam)
  {
    for (int ray = 0; ray < nRays; ++ray)
    {
      if (ray >= depth.rows || beam >= depth.cols)
      {
        continue;
      }
      const size_t idx = static_cast<size_t>(beam) * nRays + ray;
      depthFlat[idx] = depth.at<float>(ray, beam);
      reflFlat[idx] = reflectivity.at<float>(ray, beam);
      const cv::Vec3f n = normal.at<cv::Vec3f>(ray, beam);
      normalFlat[idx * 3 + 0] = n[0];
      normalFlat[idx * 3 + 1] = n[1];
      normalFlat[idx * 3 + 2] = n[2];
    }
  }

  std::vector<float> beamCorrectorFlat(static_cast<size_t>(nBeams) * nBeams, 0.0f);
  if (input.beamCorrector)
  {
    for (int r = 0; r < nBeams; ++r)
    {
      for (int c = 0; c < nBeams; ++c)
      {
        beamCorrectorFlat[static_cast<size_t>(r) * nBeams + c] = input.beamCorrector[r][c];
      }
    }
  }

  std::vector<float> windowFlat(static_cast<size_t>(nFreq), 1.0f);
  if (input.window)
  {
    for (int i = 0; i < nFreq; ++i)
    {
      windowFlat[static_cast<size_t>(i)] = input.window[i];
    }
  }

  float * result = sonar_wgpu_compute(
    depthFlat.data(), normalFlat.data(), reflFlat.data(), beamCorrectorFlat.data(),
    windowFlat.data(), static_cast<uint32_t>(nBeams), static_cast<uint32_t>(nRays),
    static_cast<uint32_t>(nFreq), static_cast<uint32_t>(std::max(0, input.raySkips)),
    static_cast<float>(input.soundSpeed), static_cast<float>(input.maxDistance),
    static_cast<float>(input.sourceLevel), static_cast<float>(input.attenuation), input.sensorGain,
    static_cast<float>(input.bandwidth), input.beamCorrectorSum, area_scaler,
    static_cast<float>(input.hFOV), static_cast<float>(input.vFOV), input.frameIndex, input.seed,
    input.debugFlag);

  if (!result)
  {
    return false;
  }

  output.nBeams = nBeams;
  output.nFreq = nFreq;
  output.beamSpectrum.assign(static_cast<size_t>(nBeams) * nFreq, std::complex<float>(0.0f, 0.0f));

  for (int b = 0; b < nBeams; ++b)
  {
    for (int f = 0; f < nFreq; ++f)
    {
      const size_t base = (static_cast<size_t>(b) * nFreq + f) * 2;
      output.At(b, f) = std::complex<float>(result[base], result[base + 1]);
    }
  }

  sonar_wgpu_free(result, static_cast<size_t>(nBeams) * nFreq * 2);

  auto stop = std::chrono::high_resolution_clock::now();
  output.computeMicros = static_cast<double>(
    std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count());
  return true;
}

}  // namespace sensors
}  // namespace gz
