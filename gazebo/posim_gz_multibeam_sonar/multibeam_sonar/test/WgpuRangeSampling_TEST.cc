#include "RangeSampling.hh"
#include "sonar_compute_backend.hh"

#include <cmath>
#include <iostream>
#include <stdexcept>

int main()
{
  using namespace gz::sensors;
  try
  {
    auto gpu = CreateComputeBackend("wgpu");
    if (!gpu || std::string(gpu->Name()) != "wgpu")
    {
      throw std::runtime_error("WGPU adapter required");
    }
    constexpr int beams = 17;
    std::vector<std::vector<float>> matrix(beams, std::vector<float>(beams, 0));
    std::vector<float *> rows(beams);
    for (int beam = 0; beam < beams; ++beam)
    {
      matrix[beam][beam] = 1;
      rows[beam] = matrix[beam].data();
    }
    int cases = 0;
    for (int n : {4096, 4097, 8192, 8193})
    {
      SonarComputeInput input;
      input.nBeams = beams;
      input.nRays = 3;
      input.maxDistance = 10;
      input.bandwidth = (n - 1) * 75.0;
      input.nFreq = RangeSampleCount(10, 1500, input.bandwidth);
      input.hFOV = 0.1;
      input.vFOV = 0.1;
      input.sensorGain = 1;
      input.beamCorrector = rows.data();
      input.beamCorrectorSum = 1;
      input.frameIndex = 20;
      input.seed = 73;
      if (input.nFreq != n || !gpu->Initialize(input))
      {
        throw std::runtime_error("automatic FFT initialization");
      }
      const double dr = 1500.0 / (2.0 * input.bandwidth);
      for (float distance : {1.0f, 9.999f, 10.0f, 10.001f})
      {
        cv::Mat depth(3, beams, CV_32FC1, cv::Scalar(0));
        cv::Mat normals(3, beams, CV_32FC3, cv::Scalar(0, 0, 1));
        cv::Mat reflection(3, beams, CV_32FC1, cv::Scalar(1));
        depth.at<float>(1, beams - 1) = distance;
        input.depthImage = &depth;
        input.normalImage = &normals;
        input.reflectivityImage = &reflection;
        SonarComputeOutput output;
        if (!gpu->Compute(input, output) || output.nFreq != n || output.nBeams != beams)
        {
          throw std::runtime_error("full-size WGPU output required");
        }
        int peak = -1;
        float strongest = 0;
        for (int bin = 0; bin < n; ++bin)
        {
          const float amplitude = std::abs(output.At(beams - 1, bin));
          if (!std::isfinite(amplitude))
          {
            throw std::runtime_error("non-finite WGPU output");
          }
          if (amplitude > strongest)
          {
            strongest = amplitude;
            peak = bin;
          }
        }
        if (distance <= input.maxDistance)
        {
          if (peak < 0 || std::abs(peak * dr - distance) > dr / 2 + 1e-5)
          {
            throw std::runtime_error("WGPU physical range grid");
          }
        }
        else if (strongest != 0)
        {
          throw std::runtime_error("out-of-range return");
        }
        ++cases;
      }
    }
    std::cout << "PASS: " << cases << " WGPU endpoint cases, including 4097 and 8193 samples\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
