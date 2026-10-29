#include "RangeSampling.hh"
#include "sonar_compute_backend.hh"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
void Require(bool condition, const char * message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}
}  // namespace

int main()
{
  using namespace gz::sensors;
  try
  {
    Require(RangeSampleCount(10, 1500, 29900) == 400, "fractional endpoint");
    Require(RangeSampleCount(10, 1500, 38400) == 513, "integer endpoint");
    Require(RangeSampleCount(10, 1500, 29925) == 400, "odd integer endpoint");
    Require(RangeSampleCount(10, 1500, 38325) == 512, "avoid intermediate quotient rounding");
    Require(RangeSampleCount(10, 1500, 75) == 2, "single interval");
    Require(RangeSampleCount(10, 1500, 307125) == 4096, "last supported WGPU grid");
    // Do not silently clamp the grid to a backend's limit: the backend must
    // reject 4097 rather than publish an axis that cannot cover the endpoint.
    Require(RangeSampleCount(10, 1500, 307200) == 4097, "backend limit remains explicit");
    Require(
      RangeSampleCount(1, 2, std::numeric_limits<int>::max() - 1.0) ==
        std::numeric_limits<int>::max(),
      "largest representable count");
    Require(RangeSampleCount(1, 2, std::numeric_limits<int>::max()) == 0, "count overflow");
    for (double invalid :
         {0.0, -1.0, std::numeric_limits<double>::infinity(),
          std::numeric_limits<double>::quiet_NaN()})
    {
      Require(RangeSampleCount(invalid, 1500, 29900) == 0, "invalid maximum");
      Require(RangeSampleCount(10, invalid, 29900) == 0, "invalid speed");
      Require(RangeSampleCount(10, 1500, invalid) == 0, "invalid bandwidth");
    }
    Require(RangeSampleCount(1e300, 1, 1e300) == 0, "overflow");
    auto cpu = CreateComputeBackend("cpu");
    Require(static_cast<bool>(cpu), "CPU factory");
    int cases = 0;
    for (double bw : {29900.0, 29925.0, 38400.0})
    {
      for (double maximum : {5.0, 10.0, 20.0})
      {
        const double dr = 1500.0 / (2.0 * bw);
        const int n = RangeSampleCount(maximum, 1500, bw);
        Require((n - 1) * dr >= maximum - 1e-12, "endpoint coverage");
        for (float d :
             {0.25f, 1.0f, static_cast<float>(0.4 * maximum), static_cast<float>(maximum - 0.001),
              static_cast<float>(maximum), static_cast<float>(maximum + 0.001), -1.0f,
              std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()})
        {
          cv::Mat depth(3, 17, CV_32FC1, cv::Scalar(d));
          cv::Mat normals(3, 17, CV_32FC3, cv::Scalar(0, 0, 1));
          SonarComputeInput input;
          input.depthImage = &depth;
          input.normalImage = &normals;
          input.nBeams = 17;
          input.nRays = 3;
          input.nFreq = n;
          input.bandwidth = bw;
          input.maxDistance = maximum;
          input.hFOV = 0.1;
          input.vFOV = 0.1;
          SonarComputeOutput output;
          Require(cpu->Compute(input, output), "CPU computation");
          Require(output.nFreq == n && output.nBeams == input.nBeams, "CPU output shape");
          for (int beam = 0; beam < input.nBeams; ++beam)
          {
            float amplitude = 0;
            int peak = -1;
            for (int bin = 0; bin < n; ++bin)
            {
              const float value = std::abs(output.At(beam, bin));
              Require(std::isfinite(value), "finite CPU output");
              if (value > amplitude)
              {
                amplitude = value;
                peak = bin;
              }
            }
            if (std::isfinite(d) && d > 0 && d <= maximum)
            {
              Require(peak >= 0, "valid point signal");
              Require(std::abs(peak * dr - d) <= dr / 2 + 1e-5, "CPU physical range grid");
            }
            else
            {
              Require(peak == -1, "invalid point contributes no signal");
            }
          }
          ++cases;
        }
      }
    }
    std::cout << "PASS: grid/invalid-input checks and " << cases << " CPU point cases x17 beams\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
