// Interpolate JSON sensor states inside a completed physics interval.
// The last sample is the unchanged measured state; physics still runs once.
#pragma once
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace posim
{
class SensorResampler
{
  std::string previous;

  static void Vector(
    rapidjson::Value & out, const rapidjson::Value & from, const rapidjson::Value & to,
    double fraction)
  {
    for (rapidjson::SizeType i = 0; i < from.Size(); ++i)
    {
      out[i].SetDouble(from[i].GetDouble() + fraction * (to[i].GetDouble() - from[i].GetDouble()));
    }
  }

  static void Quaternion(
    rapidjson::Value & out, const rapidjson::Value & from, const rapidjson::Value & to,
    double fraction)
  {
    std::array<double, 4> a, b, value;
    double dot = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
      a[i] = from[i].GetDouble();
      b[i] = to[i].GetDouble();
      dot += a[i] * b[i];
    }
    if (dot < 0)
    {
      dot = -dot;
      for (auto & component : b)
      {
        component = -component;
      }
    }
    dot = std::min(1.0, dot);
    double left = 1 - fraction, right = fraction;
    if (dot < 0.9995)
    {
      const double angle = std::acos(dot);
      left = std::sin((1 - fraction) * angle) / std::sin(angle);
      right = std::sin(fraction * angle) / std::sin(angle);
    }
    double norm = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
      value[i] = left * a[i] + right * b[i];
      norm += value[i] * value[i];
    }
    for (unsigned i = 0; i < 4; ++i)
    {
      out[i].SetDouble(value[i] / std::sqrt(norm));
    }
  }

public:
  void Reset() { previous.clear(); }

public:
  std::vector<std::string> Samples(const std::string & current, double rate)
  {
    std::vector<std::string> samples;
    if (rate > 0 && !previous.empty())
    {
      rapidjson::Document from, to;
      from.Parse(previous.c_str());
      to.Parse(current.c_str());
      const double start = from["timestamp"].GetDouble();
      const double end = to["timestamp"].GetDouble();
      const double duration = end - start;
      // A reset or long gap is not evidence for interpolated motion.
      if (duration > 0 && duration <= 0.1)
      {
        const unsigned count = std::max(1.0, std::ceil(duration * rate - 1e-6));
        for (unsigned i = 1; i < count; ++i)
        {
          const double fraction = static_cast<double>(i) / count;
          rapidjson::Document frame;
          frame.CopyFrom(from, frame.GetAllocator());
          frame["timestamp"].SetDouble(start + duration * fraction);
          Vector(frame["imu"]["gyro"], from["imu"]["gyro"], to["imu"]["gyro"], fraction);
          Vector(
            frame["imu"]["accel_body"], from["imu"]["accel_body"], to["imu"]["accel_body"],
            fraction);
          Vector(frame["position"], from["position"], to["position"], fraction);
          Vector(frame["velocity"], from["velocity"], to["velocity"], fraction);
          Quaternion(frame["quaternion"], from["quaternion"], to["quaternion"], fraction);
          rapidjson::StringBuffer buffer;
          rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
          frame.Accept(writer);
          samples.push_back("\n" + std::string(buffer.GetString()) + "\n");
        }
      }
    }
    samples.push_back(current);
    previous = current;
    return samples;
  }
};
}  // namespace posim
