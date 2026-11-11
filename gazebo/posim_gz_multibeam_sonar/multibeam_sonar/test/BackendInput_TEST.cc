// SPDX-License-Identifier: Apache-2.0
#include "sonar_compute_backend.hh"

#include <cmath>
#include <iostream>
#include <stdexcept>

int main(int argc, char ** argv)
{
  try
  {
    const std::string name = argc == 2 ? argv[1] : "cpu";
    auto backend = gz::sensors::CreateComputeBackend(name);
    if (!backend || backend->Name() != name)
    {
      throw std::runtime_error("requested backend unavailable: " + name);
    }
    cv::Mat depth(4, 4, CV_32FC1, cv::Scalar(2));
    cv::Mat normal(4, 4, CV_32FC3, cv::Scalar(0, 0, 1));
    cv::Mat reflectivity(4, 4, CV_32FC1, cv::Scalar(1));
    gz::sensors::SonarComputeInput input;
    input.depthImage = &depth;
    input.normalImage = &normal;
    input.reflectivityImage = &reflectivity;
    input.nBeams = input.nRays = 4;
    input.nFreq = 64;
    input.maxDistance = 10;
    input.bandwidth = 3000;
    input.hFOV = input.vFOV = .1;
    float correction[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    float * rows[] = {correction[0], correction[1], correction[2], correction[3]};
    input.beamCorrector = rows;
    input.beamCorrectorSum = 1;
    gz::sensors::SonarComputeOutput output;
    auto reject = [&](const char * description)
    {
      std::cout << "Checking " << description << std::endl;
      if (backend->Compute(input, output))
      {
        throw std::runtime_error(std::string("accepted invalid ") + description);
      }
    };
    for (const auto & shape : {cv::Size(1, 1), cv::Size(4, 1), cv::Size(1, 4)})
    {
      normal = cv::Mat(shape, CV_32FC3, cv::Scalar(0, 0, 1));
      reject("normal shape");
    }
    normal = cv::Mat(4, 4, CV_8UC3, cv::Scalar(0, 0, 1));
    reject("normal type");
    normal = cv::Mat(4, 4, CV_32FC3, cv::Scalar(0, 0, 1));
    reflectivity = cv::Mat(1, 1, CV_32FC1, cv::Scalar(1));
    reject("reflectivity shape");
    reflectivity = cv::Mat(4, 4, CV_8UC1, cv::Scalar(1));
    reject("reflectivity type");
    reflectivity = cv::Mat(4, 4, CV_32FC1, cv::Scalar(1));
    rows[2] = nullptr;
    reject("correction row");
    rows[2] = correction[2];
    depth = cv::Mat(4, 4, CV_8UC1, cv::Scalar(2));
    reject("depth type");

    // A valid non-contiguous ROI is supported: these backends read per row.
    cv::Mat largeDepth(6, 6, CV_32FC1, cv::Scalar(2));
    cv::Mat largeNormal(6, 6, CV_32FC3, cv::Scalar(0, 0, 1));
    cv::Mat largeReflectivity(6, 6, CV_32FC1, cv::Scalar(1));
    depth = largeDepth(cv::Rect(1, 1, 4, 4));
    normal = largeNormal(cv::Rect(1, 1, 4, 4));
    reflectivity = largeReflectivity(cv::Rect(1, 1, 4, 4));
    if (!backend->Compute(input, output) || output.nBeams != 4 || output.nFreq != 64)
    {
      throw std::runtime_error("valid strided images rejected after invalid input");
    }
    double energy = 0;
    for (const auto & value : output.beamSpectrum)
    {
      if (!std::isfinite(value.real()) || !std::isfinite(value.imag()))
      {
        throw std::runtime_error("nonfinite valid output");
      }
      energy += std::norm(value);
    }
    if (!(energy > 0))
    {
      throw std::runtime_error("valid return lost");
    }
    std::cout << "PASS: " << name << " rejects mismatched buffers and preserves strided input\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
