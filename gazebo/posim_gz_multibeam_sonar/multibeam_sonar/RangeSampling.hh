#pragma once

#include <cmath>
#include <limits>

namespace gz
{
namespace sensors
{
/// Number of uniformly spaced range samples, including the maximum-range cell.
/// N DFT samples cover [0, (N-1)*dr], not [0, N*dr]. Reserving the endpoint
/// prevents valid far-range echoes from wrapping into the zero-range bin.
inline int RangeSampleCount(double maxRange, double soundSpeed, double bandwidth)
{
  if (
    !std::isfinite(maxRange) || !std::isfinite(soundSpeed) || !std::isfinite(bandwidth) ||
    maxRange <= 0.0 || soundSpeed <= 0.0 || bandwidth <= 0.0)
  {
    return 0;
  }
  const double intervals = std::ceil(2.0 * maxRange * bandwidth / soundSpeed);
  if (!std::isfinite(intervals) || intervals >= std::numeric_limits<int>::max())
  {
    return 0;
  }
  return static_cast<int>(intervals) + 1;
}
}  // namespace sensors
}  // namespace gz
