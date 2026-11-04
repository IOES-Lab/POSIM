// Copyright 2026 POSIM contributors
// SPDX-License-Identifier: Apache-2.0

#ifndef POSIM_GZ_SENSOR_PLUGINS__USBL_ACOUSTICS_HH_
#define POSIM_GZ_SENSOR_PLUGINS__USBL_ACOUSTICS_HH_

#include <algorithm>
#include <cmath>
#include <limits>

namespace posim_gz_sensor_plugins::detail
{
inline double UsblSoundSpeed(double reference, double temperature, double worldZ)
{
  // Retain the simple depth/temperature approximation, with positive depth
  // below z=0 and the configured reference speed at 10 degrees Celsius.
  const double speed = reference + 0.017 * std::max(0.0, -worldZ) + 4.0 * (temperature - 10.0);
  return std::isfinite(worldZ) && std::isfinite(speed) && speed > 0.0
           ? speed
           : std::numeric_limits<double>::quiet_NaN();
}
}  // namespace posim_gz_sensor_plugins::detail

#endif  // POSIM_GZ_SENSOR_PLUGINS__USBL_ACOUSTICS_HH_
