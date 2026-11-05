// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <array>
#include <cmath>
#include <stdexcept>
namespace posim
{
// Stable wire order; schema and revision precede user settings.
using Appearance = std::array<double, 13>;
inline Appearance Defaults() { return {1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 10, 5}; }
inline bool Valid(const Appearance & a)
{
  for (auto v : a)
  {
    if (!std::isfinite(v))
    {
      return false;
    }
  }
  if (a[0] != 1 || a[1] < 0 || a[1] > 9007199254740991. || std::floor(a[1]) != a[1])
  {
    return false;
  }
  for (unsigned i = 2; i <= 10; ++i)
  {
    if (a[i] != 0 && a[i] != 1)
    {
      return false;
    }
  }
  return a[11] >= 1 && a[11] <= 500 && a[12] >= 1 && a[12] <= 50;
}
inline double WaterPath(double cameraZ, double surfaceZ, double distance)
{
  if (distance <= 0)
  {
    return 0;
  }
  if (cameraZ <= 0 && surfaceZ <= 0)
  {
    return distance;
  }
  if (cameraZ >= 0 && surfaceZ >= 0)
  {
    return 0;
  }
  return distance * (cameraZ < 0 ? -cameraZ : -surfaceZ) / std::abs(surfaceZ - cameraZ);
}
}  // namespace posim
