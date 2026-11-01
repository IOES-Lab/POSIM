// SPDX-License-Identifier: Apache-2.0
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include "sonar_compute_backend.hh"

int main()
{
  constexpr int beams = 3, rays = 7, bins = 64;
  float correction[beams][beams] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  float * rows[beams] = {correction[0], correction[1], correction[2]};
  float window[bins];
  for (float & value : window)
  {
    value = 1.f / std::sqrt(float(bins));
  }
  std::cout << "backend,row,stride,energy\n" << std::setprecision(17);
  for (const auto * name : {"cpu", "wgpu"})
  {
    auto backend = gz::sensors::CreateComputeBackend(name);
    if (!backend || std::string(backend->Name()) != name)
    {
      return 1;
    }
    gz::sensors::SonarComputeInput input;
    if (!backend->Initialize(input))
    {
      return 2;
    }
    for (int stride : {1, 2, 3})
    {
      for (int row = 0; row < rays; ++row)
      {
        cv::Mat depth = cv::Mat::zeros(rays, beams, CV_32FC1);
        cv::Mat normals = cv::Mat::zeros(rays, beams, CV_32FC3);
        cv::Mat reflectivity = cv::Mat::ones(rays, beams, CV_32FC1);
        depth.at<float>(row, 1) = 5.f;
        normals.at<cv::Vec3f>(row, 1) = cv::Vec3f(0, 0, 1);
        input.depthImage = &depth;
        input.normalImage = &normals;
        input.reflectivityImage = &reflectivity;
        input.nBeams = beams;
        input.nRays = rays;
        input.nFreq = bins;
        input.raySkips = stride;
        input.maxDistance = 15.;
        input.soundSpeed = 1500.;
        input.bandwidth = 3200.;
        input.hFOV = .1;
        input.vFOV = .1;
        input.sourceLevel = 220.;
        input.attenuation = 0.;
        input.sensorGain = 1.;
        input.window = window;
        input.beamCorrector = rows;
        input.beamCorrectorSum = 1.;
        input.seed = 73;
        input.frameIndex = 20;
        gz::sensors::SonarComputeOutput output;
        if (!backend->Compute(input, output))
        {
          return 3;
        }
        double energy = 0.;
        for (const auto & value : output.beamSpectrum)
        {
          if (!std::isfinite(value.real()) || !std::isfinite(value.imag()))
          {
            return 4;
          }
          energy += std::norm(std::complex<double>(value.real(), value.imag()));
        }
        std::cout << name << ',' << row << ',' << stride << ',' << energy << '\n';
      }
    }
  }
}
