#include <cassert>
#include <iostream>
#include "SensorResampler.hh"

std::string frame(double time, double position, double yaw, double range = 1)
{
  const double half = yaw * std::acos(-1.0) / 360;
  return "{\"timestamp\":" + std::to_string(time) + ",\"imu\":{\"gyro\":[0,0," +
         std::to_string(position) + "],\"accel_body\":[0,0,-9.81]},\"position\":[" +
         std::to_string(position) + ",0,0],\"velocity\":[1,0,0],\"quaternion\":[" +
         std::to_string(std::cos(half)) + ",0,0," + std::to_string(std::sin(half)) +
         "],\"rng_1\":" + std::to_string(range) + "}";
}

int main()
{
  for (double rate : {200.0, 1000.0})
  {
    // Follow changing physics steps, including steps that do not divide 1/rate.
    posim::SensorResampler changing;
    double stamp = 1;
    changing.Samples(frame(stamp, 0, 0), rate);
    for (double step : {.0005, .001, .0025, .007, .01, .02, .03, .05, .1})
    {
      const double start = stamp;
      stamp += step;
      const auto end = frame(stamp, 1, 0);
      const auto values = changing.Samples(end, rate);
      const unsigned count = std::max(1.0, std::ceil(step * rate - 1e-6));
      assert(values.size() == count && values.back() == end);
      double previous = start;
      for (const auto & sample : values)
      {
        rapidjson::Document value;
        value.Parse(sample.c_str());
        const double time = value["timestamp"].GetDouble();
        assert(time > previous && time - previous <= 1 / rate + 1e-9);
        previous = time;
      }
      assert(std::abs(previous - stamp) < 1e-9);
    }
    const auto oversized = frame(stamp + .100001, 2, 0);
    assert(changing.Samples(oversized, rate) == std::vector<std::string>{oversized});

    posim::SensorResampler resampler;
    const auto first = frame(1, 0, 179);
    const auto last = frame(1.02, 2, -179, 7);
    assert(resampler.Samples(first, rate) == std::vector<std::string>{first});
    const auto samples = resampler.Samples(last, rate);
    assert(samples.size() == static_cast<unsigned>(rate * .02));
    assert(samples.back() == last);
    double previousTime = 1;
    for (unsigned i = 0; i < samples.size(); ++i)
    {
      rapidjson::Document value;
      value.Parse(samples[i].c_str());
      double time = value["timestamp"].GetDouble();
      assert(time > previousTime && time <= 1.02);
      assert(std::abs(time - previousTime - 1 / rate) < 1e-9);
      previousTime = time;
      const double fraction = static_cast<double>(i + 1) / samples.size();
      assert(std::abs(value["position"][0].GetDouble() - 2 * fraction) < 1e-9);
      assert(std::abs(value["imu"]["gyro"][2].GetDouble() - 2 * fraction) < 1e-9);
      const auto & q = value["quaternion"];
      double yaw = 2 * std::atan2(q[3].GetDouble(), q[0].GetDouble());
      // The shortest interpolation crosses 180 degrees, never zero degrees.
      assert(std::abs(yaw) > 3.12);
      if (i + 1 < samples.size())
      {
        assert(
          std::abs(q[0].GetDouble() * q[0].GetDouble() + q[3].GetDouble() * q[3].GetDouble() - 1) <
          1e-10);
        assert(value["rng_1"].GetDouble() == 1);
      }
    }
    // Clock resets and long gaps do not invent intermediate samples.
    const auto reset = frame(0, 0, 0);
    assert(resampler.Samples(reset, rate) == std::vector<std::string>{reset});
    const auto gap = frame(.5, 1, 0);
    assert(resampler.Samples(gap, rate) == std::vector<std::string>{gap});
    resampler.Reset();
    assert(resampler.Samples(first, rate) == std::vector<std::string>{first});
  }
  posim::SensorResampler disabled;
  assert(disabled.Samples("unchanged", 0) == std::vector<std::string>{"unchanged"});
  assert(disabled.Samples("still unchanged", 0) == std::vector<std::string>{"still unchanged"});
  std::cout << "PASS: variable timesteps, clock alignment, 200/1000 Hz, vector interpolation, "
               "quaternion wrap, exact "
               "endpoint, held discrete sensor, reset/gap, disabled\n";
}
