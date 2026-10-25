// SPDX-License-Identifier: Apache-2.0
// Simplified satellite receiver. No acoustic positioning or RF propagation.
#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/double_v.pb.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <deque>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/SphericalCoordinates.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>
#include <limits>
#include <mutex>
#include <random>
#include <stdexcept>
#include <vector>

namespace posim
{
class Gnss final : public gz::sim::System,
                   public gz::sim::ISystemConfigure,
                   public gz::sim::ISystemPostUpdate,
                   public gz::sim::ISystemReset
{
public:
  void Configure(
    const gz::sim::Entity & entity, const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager & ecm, gz::sim::EventManager &) override
  {
    auto number = [&](const char * key, double fallback)
    { return sdf->Get<double>(key, fallback).first; };
    enabled = sdf->Get<bool>("enabled", true).first;
    hz = number("update_rate", 5);
    latency = number("latency", .2);
    reacquire = number("reacquire_seconds", 3);
    margin = number("dry_margin", .05);
    horizontal = number("horizontal_stddev", .7);
    vertical = number("vertical_stddev", 1.2);
    speed = number("velocity_stddev", .1);
    level = number("water_level", 0);
    if (!(hz > 0 && hz <= 100 && latency >= 0 && latency <= 10 && reacquire >= 0 && margin >= 0 &&
          horizontal >= 0 && vertical >= 0 && speed >= 0))
    {
      throw std::runtime_error("Invalid GNSS configuration");
    }
    rng.seed(sdf->Get<unsigned>("seed", 42).first);
    link =
      gz::sim::Model(entity).LinkByName(ecm, sdf->Get<std::string>("link_name", "gnss_link").first);
    if (link == gz::sim::kNullEntity)
    {
      throw std::runtime_error("GNSS antenna link missing");
    }
    const auto topic =
      sdf->Get<std::string>("topic", "/model/" + gz::sim::Model(entity).Name(ecm) + "/gnss").first;
    publisher = node.Advertise<gz::msgs::Double_V>(topic);
    node.Subscribe(topic + "/blocked", &Gnss::Block, this);
    const auto wave = sdf->Get<std::string>("wave_topic", "").first;
    requireWave = !wave.empty();
    if (requireWave)
    {
      node.Subscribe(wave, &Gnss::Wave, this);
    }
  }
  void Reset(const gz::sim::UpdateInfo &, gz::sim::EntityComponentManager &) override { Clear(); }
  void PostUpdate(
    const gz::sim::UpdateInfo & info, const gz::sim::EntityComponentManager & ecm) override
  {
    if (info.paused)
    {
      return;
    }
    const double now = std::chrono::duration<double>(info.simTime).count();
    if (now < previousTime)
    {
      Clear();
    }
    previousTime = now;
    auto pose = gz::sim::worldPose(link, ecm);
    const auto p = pose.Pos();
    double water = level;
    bool surfaceValid = true;
    if (requireWave)
    {
      std::lock_guard<std::mutex> guard(mutex);
      surfaceValid = Surface(p.X(), p.Y(), now, water);
    }
    // Loss is immediate at the wet boundary. Recovery needs continuous dry
    // clearance above a separate threshold, followed by acquisition time.
    int reason = !enabled ? 0 : !surfaceValid ? 5 : blocked ? 4 : p.Z() <= water ? 1 : 2;
    if (reason != 2)
    {
      drySince = -1;
      locked = false;
      pending.clear();
    }
    else if (!locked)
    {
      if (p.Z() > water + margin)
      {
        if (drySince < 0)
        {
          drySince = now;
        }
        locked = now - drySince >= reacquire;
      }
      else
      {
        drySince = -1;
      }
    }
    if (locked)
    {
      reason = 3;
    }
    if (now + 1e-9 < next)
    {
      return;
    }
    next = now + 1 / hz;
    const auto world = ecm.EntityByComponents(gz::sim::components::World());
    const auto coords = ecm.Component<gz::sim::components::SphericalCoordinates>(world);
    gz::math::Vector3d velocity = gz::math::Vector3d::Zero;
    if (lastSample >= 0 && now > lastSample)
    {
      velocity = (p - lastPosition) / (now - lastSample);
    }
    else if (reason == 3)
    {
      reason = 2;
    }
    lastPosition = p;
    lastSample = now;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> row = {
      1.,
      double(epoch),
      now,
      double(reason),
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      horizontal * horizontal,
      vertical * vertical,
      speed * speed,
      p.Z(),
      water};
    if (reason == 3 && coords)
    {
      const auto noisy =
        p + gz::math::Vector3d(Noise(horizontal), Noise(horizontal), Noise(vertical));
      auto geo =
        coords->Data().SphericalFromLocalPosition(gz::math::CoordinateVector3::Metric(noisy));
      auto vel =
        coords->Data().GlobalFromLocalVelocity(gz::math::CoordinateVector3::Metric(velocity));
      if (geo && vel && vel->AsMetricVector())
      {
        auto v = *vel->AsMetricVector();
        row[4] = geo->Lat()->Degree();
        row[5] = geo->Lon()->Degree();
        row[6] = *geo->Z();
        row[7] = v.X() + Noise(speed);
        row[8] = v.Y() + Noise(speed);
        row[9] = v.Z() + Noise(speed);
      }
      else
      {
        row[3] = 5;
      }
    }
    else if (reason == 3)
    {
      row[3] = 5;
    }
    if (row[3] != 3)
    {
      pending.clear();
      Publish(row);
      return;
    }
    pending.push_back({now + latency, row});
    while (!pending.empty() && pending.front().first <= now + 1e-9)
    {
      Publish(pending.front().second);
      pending.pop_front();
    }
  }

private:
  void Clear()
  {
    ++epoch;
    next = 0;
    previousTime = -1;
    lastSample = -1;
    drySince = -1;
    locked = false;
    pending.clear();
    std::lock_guard<std::mutex> guard(mutex);
    wave.clear();
  }
  double Noise(double sigma) { return sigma * normal(rng); }
  void Publish(const std::vector<double> & row)
  {
    gz::msgs::Double_V msg;
    for (auto v : row)
    {
      msg.add_data(v);
    }
    publisher.Publish(msg);
  }
  void Block(const gz::msgs::Boolean & msg) { blocked = msg.data(); }
  void Wave(const gz::msgs::Double_V & msg)
  {
    std::lock_guard<std::mutex> guard(mutex);
    wave.assign(msg.data().begin(), msg.data().end());
  }
  bool Surface(double x, double y, double now, double & height)
  {
    if (
      wave.size() < 8 || wave[0] != 1 || !std::isfinite(wave[1]) || wave[1] < 2 || wave[1] > 256 ||
      !(wave[2] > 0 && wave[3] > 0) || now - wave[7] > .6 || now < wave[7])
    {
      return false;
    }
    const int n = int(wave[1]);
    if (wave.size() != size_t(8 + n * n))
    {
      return false;
    }
    auto index = [&](double p, double size)
    {
      double c = (p / size + .5) * n - .5;
      return c - std::floor(c / n) * n;
    };
    double u = index(x, wave[2]), v = index(y, wave[3]);
    int ix = int(u), iy = int(v);
    double a = u - ix, b = v - iy;
    auto h = [&](int i, int j) { return wave[8 + (j % n) * n + i % n]; };
    height = (1 - b) * ((1 - a) * h(ix, iy) + a * h(ix + 1, iy)) +
             b * ((1 - a) * h(ix, iy + 1) + a * h(ix + 1, iy + 1)) + level;
    return std::isfinite(height);
  }
  gz::transport::Node node;
  gz::transport::Node::Publisher publisher;
  gz::sim::Entity link = gz::sim::kNullEntity;
  std::mutex mutex;
  std::vector<double> wave;
  std::atomic<bool> blocked{false};
  bool enabled = true, requireWave = false, locked = false;
  unsigned epoch = 0;
  double hz = 5, latency = .2, reacquire = 3, margin = .05, horizontal = .7, vertical = 1.2,
         speed = .1, level = 0;
  double next = 0, previousTime = -1, lastSample = -1, drySince = -1;
  gz::math::Vector3d lastPosition;
  std::deque<std::pair<double, std::vector<double>>> pending;
  std::mt19937 rng{42};
  std::normal_distribution<double> normal{0, 1};
};
}  // namespace posim
GZ_ADD_PLUGIN(
  posim::Gnss, gz::sim::System, posim::Gnss::ISystemConfigure, posim::Gnss::ISystemPostUpdate,
  posim::Gnss::ISystemReset)
GZ_ADD_PLUGIN_ALIAS(posim::Gnss, "posim::Gnss")
