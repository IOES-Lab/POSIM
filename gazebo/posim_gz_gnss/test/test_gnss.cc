// Copyright 2026 POSIM contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <chrono>
#include <condition_variable>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <sdf/Root.hh>
#include <thread>
#include "../Gnss.cc"

using namespace std::chrono_literals;

class GnssTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    static unsigned sequence = 0;
    topic = "/gnss_test/receiver_" + std::to_string(++sequence);
    auto world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    ecm.CreateComponent(world, gz::sim::components::SphericalCoordinates());
    model = ecm.CreateEntity();
    ecm.CreateComponent(model, gz::sim::components::Model());
    ecm.CreateComponent(model, gz::sim::components::Name("receiver"));
    ecm.CreateComponent(model, gz::sim::components::Pose());
    ecm.CreateComponent(model, gz::sim::components::ParentEntity(world));
    link = ecm.CreateEntity();
    ecm.CreateComponent(link, gz::sim::components::Link());
    ecm.CreateComponent(link, gz::sim::components::Name("gnss_link"));
    ecm.CreateComponent(link, gz::sim::components::Pose(gz::math::Pose3d(0, 0, 1, 0, 0, 0)));
    ecm.CreateComponent(link, gz::sim::components::ParentEntity(model));
    ecm.SetParentEntity(link, model);
    std::function<void(const gz::msgs::Double_V &)> capture = [this](const auto & message)
    {
      std::lock_guard<std::mutex> lock(mutex);
      samples.emplace_back(message.data().begin(), message.data().end());
      received.notify_all();
    };
    ASSERT_TRUE(observer.Subscribe(topic, capture));
  }

  std::shared_ptr<sdf::Element> Config(const std::string & extra = "")
  {
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
      "<topic>" +
      topic +
      "</topic><update_rate>10</update_rate>"
      "<reacquire_seconds>0.1</reacquire_seconds><latency>0</latency>"
      "<horizontal_stddev>0</horizontal_stddev><vertical_stddev>0</vertical_stddev>"
      "<velocity_stddev>0</velocity_stddev>" +
      extra + "</plugin></world></sdf>");
    EXPECT_TRUE(errors.empty());
    return root.Element()->GetElement("world")->GetElement("plugin");
  }

  void Advance(double seconds)
  {
    gz::sim::UpdateInfo info;
    info.paused = false;
    info.simTime = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(seconds));
    receiver.PostUpdate(info, ecm);
  }

  std::vector<double> Sample(double seconds)
  {
    size_t before;
    {
      std::lock_guard<std::mutex> lock(mutex);
      before = samples.size();
    }
    Advance(seconds);
    std::unique_lock<std::mutex> lock(mutex);
    if (!received.wait_for(lock, 2s, [&] { return samples.size() > before; }))
    {
      ADD_FAILURE() << "No GNSS sample at simulation time " << seconds;
      return {};
    }
    return samples.back();
  }

  template <typename Message>
  void Send(const std::string & destination, const Message & message)
  {
    auto publisher = observer.Advertise<Message>(destination);
    auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!publisher.HasConnections() && std::chrono::steady_clock::now() < deadline)
    {
      std::this_thread::sleep_for(5ms);
    }
    ASSERT_TRUE(publisher.HasConnections());
    ASSERT_TRUE(publisher.Publish(message));
    std::this_thread::sleep_for(50ms);
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity model, link;
  std::string topic;
  std::mutex mutex;
  std::condition_variable received;
  std::vector<std::vector<double>> samples;
  posim::Gnss receiver;
  gz::transport::Node observer;
};

TEST_F(GnssTest, RejectsNonfiniteWaterLevelAndReceiverParameters)
{
  for (const auto * key :
       {"water_level", "update_rate", "latency", "reacquire_seconds", "dry_margin",
        "horizontal_stddev", "vertical_stddev", "velocity_stddev"})
  {
    for (const double invalid :
         {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
    {
      SCOPED_TRACE(key);
      auto config = Config();
      if (!config->HasElement(key))
      {
        config = Config(std::string("<") + key + ">0</" + key + ">");
      }
      ASSERT_TRUE(config->GetElement(key)->Set(invalid));
      posim::Gnss invalidReceiver;
      EXPECT_THROW(invalidReceiver.Configure(model, config, ecm, events), std::runtime_error);
    }
  }
}

TEST_F(GnssTest, DryFixSubmersionRecoveryAndReset)
{
  receiver.Configure(model, Config(), ecm, events);
  auto row = Sample(0);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 2);
  EXPECT_TRUE(std::isnan(row[4]));
  row = Sample(0.1);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 3);
  EXPECT_NEAR(row[4], 0, 1e-9);
  EXPECT_NEAR(row[5], 0, 1e-9);
  EXPECT_NEAR(row[6], 1, 1e-5);
  EXPECT_EQ(row[13], 1);
  ecm.SetComponentData<gz::sim::components::Pose>(link, gz::math::Pose3d(0, 0, -1, 0, 0, 0));
  row = Sample(0.2);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 1);
  EXPECT_TRUE(std::isnan(row[4]));
  ecm.SetComponentData<gz::sim::components::Pose>(link, gz::math::Pose3d(0, 0, 1, 0, 0, 0));
  row = Sample(0.4);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 2);
  row = Sample(0.6);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 3);
  receiver.Reset(gz::sim::UpdateInfo(), ecm);
  row = Sample(0);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[1], 1);
  EXPECT_EQ(row[3], 2);
}

TEST_F(GnssTest, ObstructionClearsFixAndRequiresReacquisition)
{
  receiver.Configure(model, Config(), ecm, events);
  ASSERT_EQ(Sample(0).size(), 15u);
  auto row = Sample(0.1);
  ASSERT_EQ(row.size(), 15u);
  ASSERT_EQ(row[3], 3);
  gz::msgs::Boolean blocked;
  blocked.set_data(true);
  Send(topic + "/blocked", blocked);
  row = Sample(0.2);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 4);
  EXPECT_TRUE(std::isnan(row[4]));
  blocked.set_data(false);
  Send(topic + "/blocked", blocked);
  row = Sample(0.4);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 2);
  row = Sample(0.6);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 3);
}

TEST_F(GnssTest, InvalidWaveMetadataCannotProduceAValidFix)
{
  receiver.Configure(model, Config("<wave_topic>" + topic + "/waves</wave_topic>"), ecm, events);
  const std::vector<double> valid = {1, 2, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0};
  double now = 0;
  for (const auto & invalid : std::vector<std::pair<size_t, double>>{
         {1, 2.5},
         {2, std::numeric_limits<double>::infinity()},
         {3, std::numeric_limits<double>::infinity()},
         {7, std::numeric_limits<double>::quiet_NaN()}})
  {
    SCOPED_TRACE(invalid.first);
    auto values = valid;
    values[7] = now;
    values[invalid.first] = invalid.second;
    gz::msgs::Double_V wave;
    for (const auto value : values)
    {
      wave.add_data(value);
    }
    Send(topic + "/waves", wave);
    auto row = Sample(now);
    ASSERT_EQ(row.size(), 15u);
    EXPECT_EQ(row[3], 5);
    EXPECT_TRUE(std::isnan(row[4]));
    now += 0.2;
  }
}

TEST_F(GnssTest, ValidPeriodicSurfaceAndStaleOrFutureSamples)
{
  receiver.Configure(model, Config("<wave_topic>" + topic + "/waves</wave_topic>"), ecm, events);
  gz::msgs::Double_V wave;
  for (const double value : {1., 2., 10., 10., 0., 0., 0., 0., 0., 0., 0., 0.})
  {
    wave.add_data(value);
  }
  Send(topic + "/waves", wave);
  auto row = Sample(0);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 2);
  row = Sample(0.2);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 3);
  EXPECT_EQ(row[14], 0);
  row = Sample(0.8);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 5);
  wave.set_data(7, 10);
  Send(topic + "/waves", wave);
  row = Sample(1);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 5);
}

TEST_F(GnssTest, DestroyingReceiverDuringWaveTrafficIsSafe)
{
  const auto destination = topic + "/waves";
  auto publisher = observer.Advertise<gz::msgs::Double_V>(destination);
  std::atomic<bool> done{false};
  std::thread traffic(
    [&]
    {
      gz::msgs::Double_V message;
      for (int i = 0; i < 12; ++i)
      {
        message.add_data(i);
      }
      while (!done)
      {
        publisher.Publish(message);
        std::this_thread::sleep_for(100us);
      }
    });
  for (int iteration = 0; iteration < 20; ++iteration)
  {
    posim::Gnss temporary;
    temporary.Configure(model, Config("<wave_topic>" + destination + "</wave_topic>"), ecm, events);
    std::this_thread::sleep_for(2ms);
  }
  done = true;
  traffic.join();
}

TEST_F(GnssTest, LatencyPreservesAcquisitionStampAndLossDiscardsQueuedFixes)
{
  auto config = Config();
  ASSERT_TRUE(config->GetElement("latency")->Set(0.2));
  receiver.Configure(model, config, ecm, events);
  ASSERT_EQ(Sample(0).size(), 15u);
  Advance(0.1);
  std::this_thread::sleep_for(50ms);
  {
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_EQ(samples.size(), 1u);
  }
  gz::msgs::Boolean blocked;
  blocked.set_data(true);
  Send(topic + "/blocked", blocked);
  auto row = Sample(0.2);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 4);
  blocked.set_data(false);
  Send(topic + "/blocked", blocked);
  row = Sample(0.3);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 2);
  Advance(0.5);
  row = Sample(0.7);
  ASSERT_EQ(row.size(), 15u);
  EXPECT_EQ(row[3], 3);
  EXPECT_NEAR(row[2], 0.5, 1e-9);
  // Loss must discard the pre-obstruction sample acquired at t=0.1.
  {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto & sample : samples)
    {
      if (sample[3] == 3)
      {
        EXPECT_GE(sample[2], 0.5);
      }
    }
  }
}
