// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mavros_msgs/msg/gpsinput.hpp>

namespace posim
{
// The marine receiver's atomic Double_V protocol. Keep invalid measurements
// intact: underwater reception must never fall back to the last dry fix.
class GnssInput
{
public:
  using Packet = std::array<double, 15>;
  bool Receive(const Packet & value)
  {
    if (
      value[0] != 1 || !std::isfinite(value[1]) || value[1] < 0 ||
      std::floor(value[1]) != value[1] || !std::isfinite(value[2]) || value[2] < 0 ||
      value[2] >= std::numeric_limits<int32_t>::max() || !std::isfinite(value[3]) || value[3] < 0 ||
      value[3] > 5 || std::floor(value[3]) != value[3])
    {
      Invalidate();
      return false;
    }
    if (ordered && (value[1] < packet[1] || (value[1] == packet[1] && value[2] <= packet[2])))
    {
      return false;
    }
    packet = value;
    ordered = true;
    received = true;
    return true;
  }
  void Invalidate() { received = false; }
  double Epoch() const { return received ? packet[1] : -1; }
  bool HasFix(double now, double maxAge) const
  {
    if (
      !received || packet[3] != 3 || !std::isfinite(now) || packet[2] > now ||
      now - packet[2] > maxAge)
    {
      return false;
    }
    for (size_t i = 4; i < packet.size(); ++i)
    {
      if (!std::isfinite(packet[i]))
      {
        return false;
      }
    }
    if (
      std::abs(packet[4]) > 90 || std::abs(packet[5]) > 180 || std::abs(packet[6]) > 20000000 ||
      packet[13] <= packet[14])
    {
      return false;
    }
    for (size_t i = 7; i <= 12; ++i)
    {
      if (std::abs(packet[i]) > std::numeric_limits<float>::max() || (i >= 10 && packet[i] < 0))
      {
        return false;
      }
    }
    return true;
  }
  const Packet & Measurement() const { return packet; }
  mavros_msgs::msg::GPSINPUT Message(double now, double maxAge, double geoidHeight) const
  {
    mavros_msgs::msg::GPSINPUT result;
    // Satellite geometry, dilution and GPS calendar time are not modeled.
    // Zero week disables ArduPilot's GPS-week jitter correction; header time
    // remains the receiver's simulation acquisition time.
    result.fix_type = result.GPS_FIX_TYPE_NO_FIX;
    result.ignore_flags = 255;
    result.satellites_visible = 0;
    if (!HasFix(now, maxAge) || !std::isfinite(geoidHeight))
    {
      return result;
    }
    result.fix_type = result.GPS_FIX_TYPE_3D_FIX;
    result.ignore_flags = 2 | 4;      // HDOP and VDOP unknown.
    result.satellites_visible = 255;  // Unknown, not a fabricated satellite count.
    result.header.stamp.sec = static_cast<int32_t>(packet[2]);
    result.header.stamp.nanosec =
      static_cast<uint32_t>((packet[2] - result.header.stamp.sec) * 1000000000);
    result.lat = static_cast<int32_t>(std::llround(packet[4] * 1e7));
    result.lon = static_cast<int32_t>(std::llround(packet[5] * 1e7));
    result.alt = packet[6] - geoidHeight;  // WGS84 ellipsoid -> EGM96 MSL.
    result.vn = packet[8];
    result.ve = packet[7];
    result.vd = -packet[9];  // ENU -> NED.
    result.horiz_accuracy = std::sqrt(packet[10]);
    result.vert_accuracy = std::sqrt(packet[11]);
    result.speed_accuracy = std::sqrt(packet[12]);
    return result;
  }

private:
  Packet packet{};
  bool received = false;
  bool ordered = false;
};
}  // namespace posim
