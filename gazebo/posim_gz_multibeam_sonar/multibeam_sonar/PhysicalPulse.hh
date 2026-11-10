// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace gz::sensors
{

// A unit-peak baseband Hamming pulse. Output spacing is c/(2B); frequency
// quadrature is independent of the published range interval. The longer
// period reduces circular replicas, not the physical pulse's sidelobes.
struct PhysicalPulse
{
  std::vector<float> ranges;
  std::vector<float> spectrum;  // dimensionless B*S(f); integrated with 1/M
};

inline PhysicalPulse MakePhysicalPulse(double maxRange, double soundSpeed, double bandwidth)
{
  if (
    !std::isfinite(maxRange) || !std::isfinite(soundSpeed) || !std::isfinite(bandwidth) ||
    maxRange <= 0 || soundSpeed <= 0 || bandwidth <= 0)
  {
    throw std::invalid_argument("Invalid physical pulse parameters");
  }
  const double intervals = std::ceil(2. * maxRange * bandwidth / soundSpeed);
  // Keep a complete output interval as guard, including the far-range cell.
  // Device-dependent limits are checked by the backend, not a fixed FFT cap.
  if (
    !std::isfinite(intervals) || intervals < 1 || intervals >= std::numeric_limits<int>::max() / 2)
  {
    throw std::invalid_argument("Physical pulse sample count is not representable");
  }
  const int outputCount = static_cast<int>(intervals) + 1;
  int count = 1;
  while (count < 2 * outputCount)
  {
    if (count > std::numeric_limits<int>::max() / 2)
    {
      throw std::invalid_argument("Physical pulse transform size is not representable");
    }
    count *= 2;
  }
  PhysicalPulse pulse;
  pulse.ranges.resize(outputCount);
  for (int i = 0; i < outputCount; ++i)
  {
    pulse.ranges[i] = i * soundSpeed / (2. * bandwidth);
  }
  pulse.spectrum.resize(count);
  for (int i = 0; i < count; ++i)
  {
    const double frequency = (i + .5) / count - .5;
    pulse.spectrum[i] = (.54 + .46 * std::cos(2. * M_PI * frequency)) / .54;
  }
  return pulse;
}

}  // namespace gz::sensors
