// SPDX-License-Identifier: Apache-2.0
#include "sonar_compute_backend.hh"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
void Require(bool condition, const std::string & message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

double Energy(
  gz::sensors::ComputeBackend & backend, float distance, double angularAreaRatio = 1.,
  double attenuation = 0.)
{
  constexpr int beams = 3, rays = 3, bins = 64;
  cv::Mat depth = cv::Mat::zeros(rays, beams, CV_32FC1);
  cv::Mat normals = cv::Mat::zeros(rays, beams, CV_32FC3);
  cv::Mat reflectivity = cv::Mat::ones(rays, beams, CV_32FC1);
  depth.at<float>(1, 1) = distance;
  // RaySurfaceNormals supplies a ray-local normal; z already is cos(incidence).
  normals.at<cv::Vec3f>(1, 1) = cv::Vec3f(0, 0, 1);
  float correction[beams][beams] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  float * rows[beams] = {correction[0], correction[1], correction[2]};
  float window[bins];
  for (float & value : window)
  {
    value = 1.f / std::sqrt(float(bins));
  }
  gz::sensors::SonarComputeInput input;
  input.depthImage = &depth;
  input.normalImage = &normals;
  input.reflectivityImage = &reflectivity;
  input.nBeams = beams;
  input.nRays = rays;
  input.nFreq = bins;
  input.maxDistance = 15.;
  input.soundSpeed = 1500.;
  input.bandwidth = 3150.;
  input.hFOV = 0.1 * angularAreaRatio;
  input.vFOV = 0.1;
  input.sourceLevel = 220.;
  input.attenuation = attenuation;
  input.sensorGain = 1.;
  input.window = window;
  input.beamCorrector = rows;
  input.beamCorrectorSum = 1.;
  input.elevation_angles = {-0.05f, 0.f, 0.05f};
  input.seed = 73;
  input.frameIndex = 20;
  gz::sensors::SonarComputeOutput output;
  Require(backend.Compute(input, output), "backend computation failed");
  Require(output.nBeams == beams && output.nFreq == bins, "unexpected output grid");
  double energy = 0.;
  for (const auto & value : output.beamSpectrum)
  {
    Require(std::isfinite(value.real()) && std::isfinite(value.imag()), "nonfinite signal");
    energy += std::norm(std::complex<double>(value.real(), value.imag()));
  }
  return energy;
}

void Ratio(double actual, double expected, const std::string & name)
{
  Require(
    std::abs(actual / expected - 1.) < 5e-4,
    name + ": measured " + std::to_string(actual) + ", expected " + std::to_string(expected));
}
}  // namespace

int main(int argc, char ** argv)
{
  try
  {
    const std::string name = argc == 2 ? argv[1] : "cpu";
    auto backend = gz::sensors::CreateComputeBackend(name);
    Require(backend && backend->Name() == name, "requested backend is unavailable: " + name);
    gz::sensors::SonarComputeInput prototype;
    Require(backend->Initialize(prototype), "backend initialization failed");
    const double baseline = Energy(*backend, 5.f);
    Require(baseline > 0., "empty baseline signal");
    // Projected area of a fixed angular cell grows as r^2; two-way pressure
    // spreading is r^-2. Consequently integrated power scales as r^-2.
    for (float distance : {3.f, 10.f, 15.f})
    {
      const double ratio = 5. / distance;
      Ratio(Energy(*backend, distance) / baseline, ratio * ratio, "fixed angular cell");
    }
    // A fixed physical patch subtends one quarter the solid angle at twice
    // the range. Its received power therefore scales as r^-4.
    Ratio(Energy(*backend, 10.f, .25) / baseline, 1. / 16., "fixed physical patch");
    Ratio(
      Energy(*backend, 10.f, 1., .05) / Energy(*backend, 5.f, 1., .05), .25 * std::exp(-1.),
      "two-way absorption with projected area");
    Require(Energy(*backend, 0.f) == 0., "no return must have zero energy");
    std::cout << "PASS: " << name << " projected-area, fixed-patch and absorption controls\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
