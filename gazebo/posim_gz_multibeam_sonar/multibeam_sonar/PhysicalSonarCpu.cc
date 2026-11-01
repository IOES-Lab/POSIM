// SPDX-License-Identifier: Apache-2.0
#include "PhysicalSonarInput.hh"

#include <array>
#include <chrono>

namespace gz::sensors
{
namespace
{

// The same Philox4x32-10 counter and Box-Muller convention as backscatter.wgsl.
// This is numerical backend parity, not a surface-anchored temporal speckle model.
std::complex<float> ScatterNoise(uint64_t seed, uint64_t frame, uint32_t sample)
{
  std::array<uint32_t, 4> c{uint32_t(frame), uint32_t(frame >> 32), sample, 0};
  uint32_t k0 = uint32_t(seed), k1 = uint32_t(seed >> 32);
  for (int round = 0; round < 10; ++round)
  {
    const uint64_t a = uint64_t(0xD2511F53u) * c[0];
    const uint64_t b = uint64_t(0xCD9E8D57u) * c[2];
    c = {uint32_t(b >> 32) ^ c[1] ^ k0, uint32_t(b), uint32_t(a >> 32) ^ c[3] ^ k1, uint32_t(a)};
    k0 += 0x9E3779B9u;
    k1 += 0xBB67AE85u;
  }
  const float u1 = float(c[0]) * (1.f / 4294967296.f) + (0.5f / 4294967296.f);
  const float u2 = float(c[1]) * (1.f / 4294967296.f) + (0.5f / 4294967296.f);
  const float radius = std::sqrt(-2.f * std::log(u1));
  const float angle = 2.f * float(M_PI) * u2;
  return {radius * std::sin(angle), radius * std::cos(angle)};
}

}  // namespace

bool ComputePhysicalSonarCpu(const SonarComputeInput & input, SonarComputeOutput & output)
{
  const auto start = std::chrono::steady_clock::now();
  PackedPhysicalSonar cells;
  if (!PackPhysicalSonar(input, cells))
  {
    return false;
  }
  const int columns = cells.geometryColumns;
  const int rows = cells.selectedRows;
  const int count = input.nFreq;
  cv::Mat real = cv::Mat::zeros(columns, count, CV_32F);
  cv::Mat imag = cv::Mat::zeros(columns, count, CV_32F);
  const float source = std::sqrt(std::pow(10.f, float(input.sourceLevel) / 10.f)) * 1e-6f;
  if (!std::isfinite(source))
  {
    return false;
  }
  const float delta = float(input.bandwidth) / count;
  for (int column = 0; column < columns; ++column)
  {
    for (int row = 0; row < rows; ++row)
    {
      const size_t i = size_t(column) * rows + row;
      const float range = cells.depth[i];
      if (!(range > 0))
      {
        continue;
      }
      // Explicit projected-area / cosine-squared power convention, matching
      // the WGPU model. Material reflectivity and solid angle stay separate.
      const float magnitude = source * std::exp(-2.f * float(input.attenuation) * range) /
                              (range * range) * std::sqrt(cells.reflectivity[i]) *
                              std::abs(cells.normal[i * 3 + 2]) * range *
                              std::sqrt(cells.angularArea[i]) * input.receiveGain;
      const auto amplitude =
        ScatterNoise(input.seed, input.frameIndex, uint32_t(i)) / std::sqrt(2.f) * magnitude;
      for (int f = 0; f < count; ++f)
      {
        const float frequency = delta * (f + .5f - .5f * count);
        const float phase = 2.f * range * (2.f * float(M_PI) * frequency / float(input.soundSpeed));
        const auto value = amplitude * std::complex<float>(std::cos(phase), std::sin(phase));
        real.at<float>(column, f) += value.real();
        imag.at<float>(column, f) += value.imag();
      }
    }
    for (int f = 0; f < count; ++f)
    {
      real.at<float>(column, f) *= input.window[f];
      imag.at<float>(column, f) *= input.window[f];
    }
  }
  // Each output direction integrates geometry contributions through its own
  // signed pressure response. There is no sample-count-dependent normalization.
  const cv::Mat response(
    input.nBeams, columns, CV_32F,
    const_cast<float *>(input.physicalReadout->pressureResponse.data()));
  cv::Mat outputReal, outputImag, spectrum, transformed;
  cv::gemm(response, real, 1, cv::Mat(), 0, outputReal);
  cv::gemm(response, imag, 1, cv::Mat(), 0, outputImag);
  std::vector<cv::Mat> channels{outputReal, outputImag};
  cv::merge(channels, spectrum);
  cv::dft(spectrum, transformed, cv::DFT_ROWS);
  if (!cv::checkRange(transformed))
  {
    return false;
  }
  output.nBeams = input.nBeams;
  output.nFreq = input.nOutputRanges ? input.nOutputRanges : count;
  output.beamSpectrum.resize(size_t(input.nBeams) * output.nFreq);
  for (int beam = 0; beam < input.nBeams; ++beam)
  {
    for (int f = 0; f < output.nFreq; ++f)
    {
      const auto value = transformed.at<cv::Vec2f>(beam, f);
      // Undo the midpoint frequency-grid origin. Apply normalized quadrature
      // after accumulation to retain fixed-point backend precision.
      const float phase = -float(M_PI) * f / count;
      const float sign = (f % 2) ? -1.f : 1.f;
      output.At(beam, f) = std::complex<float>(value[0], value[1]) *
                           std::complex<float>(std::cos(phase), std::sin(phase)) * (sign / count);
    }
  }
  output.computeMicros =
    std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count();
  return true;
}

}  // namespace gz::sensors
