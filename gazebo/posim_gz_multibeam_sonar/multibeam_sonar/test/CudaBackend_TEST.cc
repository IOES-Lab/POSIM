// SPDX-License-Identifier: Apache-2.0
#include <cmath>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include "sonar_compute_backend.hh"

namespace
{
void Require(bool condition, const char * message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

struct Scene
{
  cv::Mat depth, normals, reflectivity;
  std::vector<float> window, correction;
  std::vector<float *> rows;
  gz::sensors::SonarComputeInput input;

  Scene(int beams, int rays, int frequencies, int stride)
  : depth(rays, beams, CV_32FC1, cv::Scalar(16)),
    normals(rays, beams, CV_32FC3, cv::Scalar(0, 0, -1)),
    reflectivity(rays, beams, CV_32FC1, cv::Scalar(1)),
    window(frequencies, 1),
    correction(beams * beams, 0),
    rows(beams)
  {
    for (int beam = 0; beam < beams; ++beam)
    {
      rows[beam] = correction.data() + beam * beams;
      rows[beam][beam] = 1;
    }
    input.depthImage = &depth;
    input.normalImage = &normals;
    input.reflectivityImage = &reflectivity;
    input.nBeams = beams;
    input.nRays = rays;
    input.nFreq = frequencies;
    input.raySkips = stride;
    input.hFOV = input.vFOV = .3;
    input.maxDistance = 15;
    input.window = window.data();
    input.beamCorrector = rows.data();
    input.beamCorrectorSum = 1;
    input.elevation_angles.resize(rays);
    input.bandwidth = 20000;
  }

  void Return(bool enabled)
  {
    const int lastRay = (input.nRays - 1) / input.raySkips * input.raySkips;
    depth.at<float>(lastRay, input.nBeams - 1) = enabled ? 5.f : 16.f;
  }
};

auto Backend(const Scene & scene)
{
  auto backend = gz::sensors::CreateComputeBackend("cuda", scene.input);
  Require(backend && std::string(backend->Name()) == "cuda", "CUDA backend unavailable");
  return backend;
}

void Check(gz::sensors::ComputeBackend & backend, Scene & scene, bool hasReturn)
{
  scene.Return(hasReturn);
  gz::sensors::SonarComputeOutput output;
  Require(backend.Compute(scene.input, output), "CUDA computation failed");
  Require(
    output.nBeams == scene.input.nBeams && output.nFreq == scene.input.nFreq &&
      output.beamSpectrum.size() == size_t(output.nBeams) * output.nFreq,
    "Incorrect output shape");
  double energy = 0;
  for (int beam = 0; beam < output.nBeams; ++beam)
  {
    for (int frequency = 0; frequency < output.nFreq; ++frequency)
    {
      const auto value = output.At(beam, frequency);
      Require(std::isfinite(value.real()) && std::isfinite(value.imag()), "Nonfinite output");
      if (!hasReturn || beam != output.nBeams - 1)
      {
        Require(value == std::complex<float>{}, "Return escaped its beam or empty frame");
      }
      energy += std::norm(std::complex<double>(value));
    }
  }
  Require(hasReturn ? energy > 0 : energy == 0, "Missing return or stale empty output");
}
}  // namespace

int main()
{
  try
  {
    Scene small(32, 32, 400, 10), large(65, 37, 599, 1), sparse(17, 13, 63, 20);
    auto first = Backend(small);
    Check(*first, small, true);
    first.reset();
    // Initialization without a calculation must also be safe after prior use.
    Backend(small).reset();
    first = Backend(small);
    auto second = Backend(large);
    for (Scene * scene : {&small, &large, &sparse, &small})
    {
      Check(*first, *scene, true);
      Check(*first, *scene, false);
      Check(*first, *scene, true);
    }
    second.reset();
    Check(*first, small, true);

    // Separate sensor workers can compute and destroy backends concurrently.
    auto worker = [](int beams, int rays, int frequencies, int stride)
    {
      Scene scene(beams, rays, frequencies, stride);
      for (int repeat = 0; repeat < 3; ++repeat)
      {
        auto backend = Backend(scene);
        Check(*backend, scene, true);
        Check(*backend, scene, false);
        Check(*backend, scene, true);
      }
    };
    auto a = std::async(std::launch::async, worker, 32, 32, 400, 10);
    auto b = std::async(std::launch::async, worker, 65, 37, 599, 1);
    a.get();
    b.get();
    Check(*first, small, true);
    first.reset();
    std::cout
      << "PASS CUDA shape changes, last-ray returns, empty/recovery and backend lifetimes\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL " << error.what() << '\n';
    return 1;
  }
}
