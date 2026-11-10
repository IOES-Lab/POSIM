// Copyright 2026 POSIM contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <gz/msgs/fluid_pressure.pb.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/transport/Node.hh>
#include <limits>
#include <sdf/Root.hh>
#include <thread>
#include <vector>
#include "posim_gz_sensor_plugins/sea_pressure_sensor.hh"

class PressureBehavior : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    }
    static unsigned serial = 0;
    ns = "pressure_behavior_" + std::to_string(++serial);
    observer = std::make_shared<rclcpp::Node>(ns + "_observer");
    pressureSub = observer->create_subscription<sensor_msgs::msg::FluidPressure>(
      "/model/" + ns + "/sea_pressure", 10,
      [&](sensor_msgs::msg::FluidPressure::SharedPtr msg) { pressure.push_back(*msg); });
    depthSub = observer->create_subscription<geometry_msgs::msg::PointStamped>(
      "/model/" + ns + "/sea_pressure_depth", 10,
      [&](geometry_msgs::msg::PointStamped::SharedPtr msg) { depth.push_back(*msg); });
    model = ecm.CreateEntity();
    ecm.CreateComponent(model, gz::sim::components::Model());
    ecm.CreateComponent(model, gz::sim::components::Name(ns));
    ecm.CreateComponent(model, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -5, 0, 0, 0)));
  }

  std::shared_ptr<sdf::Element> Config(const std::string & extra = "")
  {
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
      "<namespace>" +
      ns + "</namespace>" + extra + "</plugin></world></sdf>");
    EXPECT_TRUE(errors.empty());
    return root.Element()->GetElement("world")->GetElement("plugin");
  }

  template <typename Predicate>
  bool Wait(Predicate predicate, std::chrono::milliseconds timeout = std::chrono::seconds(3))
  {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!predicate() && std::chrono::steady_clock::now() < deadline)
    {
      rclcpp::spin_some(observer);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return predicate();
  }

  void Tick(int milliseconds, bool paused = false)
  {
    gz::sim::UpdateInfo info;
    info.simTime = std::chrono::milliseconds(milliseconds);
    info.iterations = milliseconds + 1;
    info.paused = paused;
    sensor.PreUpdate(info, ecm);
    sensor.PostUpdate(info, ecm);
  }

  void ExpectSilence()
  {
    const auto count = pressure.size();
    EXPECT_FALSE(Wait([&]() { return pressure.size() != count; }, std::chrono::milliseconds(200)));
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity model;
  std::string ns;
  std::shared_ptr<rclcpp::Node> observer;
  rclcpp::Subscription<sensor_msgs::msg::FluidPressure>::SharedPtr pressureSub;
  rclcpp::Subscription<geometry_msgs::msg::PointStamped>::SharedPtr depthSub;
  std::vector<sensor_msgs::msg::FluidPressure> pressure;
  std::vector<geometry_msgs::msg::PointStamped> depth;
  posim_gz_sensor_plugins::SubseaPressureSensorPlugin sensor;
};

TEST_F(PressureBehavior, NestedModelUsesWorldDepth)
{
  const auto parent = ecm.CreateEntity();
  ecm.CreateComponent(parent, gz::sim::components::Model());
  ecm.CreateComponent(parent, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -10, 0, 0, 0)));
  ecm.CreateComponent(model, gz::sim::components::ParentEntity(parent));
  ecm.SetParentEntity(model, parent);
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(
    Wait([&]() { return pressureSub->get_publisher_count() && depthSub->get_publisher_count(); }));
  Tick(1125);
  ASSERT_TRUE(Wait([&]() { return !pressure.empty() && !depth.empty(); }));
  EXPECT_NEAR(pressure.back().fluid_pressure, 101325.0 + 15.0 * 9806.38, 1e-6);
  EXPECT_NEAR(depth.back().point.z, 15.0, 1e-9);
}

TEST_F(PressureBehavior, UsesAttachedModelWhenNamespaceIsAnOutputPrefix)
{
  ecm.SetComponentData<gz::sim::components::Name>(model, "actual_model");
  const auto unrelated = ecm.CreateEntity();
  ecm.CreateComponent(unrelated, gz::sim::components::Name(ns));
  ecm.CreateComponent(unrelated, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -2, 0, 0, 0)));
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  Tick(125);
  ASSERT_TRUE(Wait([&]() { return !pressure.empty(); }));
  EXPECT_NEAR(pressure.back().fluid_pressure, 101325.0 + 5.0 * 9806.38, 1e-6);
}

TEST_F(PressureBehavior, GazeboAndRosUseTheSameFullTimestamp)
{
  std::atomic<int> seconds{-1};
  std::atomic<int> nanoseconds{-1};
  gz::transport::Node transport;
  std::function<void(const gz::msgs::FluidPressure &)> capture =
    [&](const gz::msgs::FluidPressure & msg)
  {
    seconds = msg.header().stamp().sec();
    nanoseconds = msg.header().stamp().nsec();
  };
  ASSERT_TRUE(transport.Subscribe("/model/" + ns + "/sea_pressure", capture));
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  Tick(1125);
  ASSERT_TRUE(Wait([&]() { return !pressure.empty() && seconds >= 0; }));
  EXPECT_EQ(seconds.load(), pressure.back().header.stamp.sec);
  EXPECT_EQ(nanoseconds.load(), pressure.back().header.stamp.nanosec);
  transport.Unsubscribe("/model/" + ns + "/sea_pressure");
}

TEST_F(PressureBehavior, PauseAndRepeatedTimeDoNotRepublishAndRewindRecovers)
{
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  Tick(125);
  ASSERT_TRUE(Wait([&]() { return pressure.size() == 1; }));
  Tick(125);
  ExpectSilence();
  Tick(325, true);
  ExpectSilence();
  Tick(250);
  ASSERT_TRUE(Wait([&]() { return pressure.back().header.stamp.nanosec == 250000000u; }));
  Tick(50);
  ASSERT_TRUE(Wait([&]() { return pressure.back().header.stamp.nanosec == 50000000u; }));
}

TEST_F(PressureBehavior, ConfiguredRateAndSurfacePressureWorkWithDepthDisabled)
{
  ecm.SetComponentData<gz::sim::components::Pose>(model, gz::math::Pose3d(0, 0, 5, 0, 0, 0));
  sensor.Configure(
    model,
    Config("<standard_pressure>100</standard_pressure><kPa_per_meter>10</kPa_per_meter>"
           "<update_rate>10</update_rate><estimate_depth_on>false</estimate_depth_on>"),
    ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  EXPECT_EQ(depthSub->get_publisher_count(), 0u);
  Tick(100);
  ASSERT_TRUE(Wait([&]() { return pressure.size() == 1; }));
  EXPECT_DOUBLE_EQ(pressure.back().fluid_pressure, 100000.0);
  Tick(150);
  ExpectSilence();
  Tick(200);
  ASSERT_TRUE(Wait([&]() { return pressure.size() == 2; }));
  Tick(50);
  ASSERT_TRUE(Wait([&]() { return pressure.size() == 3; }));
}

TEST_F(PressureBehavior, PressureOverflowIsSuppressedAndValidDepthRecovers)
{
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  ecm.SetComponentData<gz::sim::components::Pose>(
    model, gz::math::Pose3d(0, 0, -std::numeric_limits<double>::max(), 0, 0, 0));
  Tick(125);
  ExpectSilence();
  ecm.SetComponentData<gz::sim::components::Pose>(model, gz::math::Pose3d(0, 0, -5, 0, 0, 0));
  Tick(250);
  ASSERT_TRUE(Wait([&]() { return !pressure.empty(); }));
  EXPECT_NEAR(pressure.back().fluid_pressure, 101325.0 + 5.0 * 9806.38, 1e-6);
}

TEST_F(PressureBehavior, MissingAndNonfinitePoseDoNotPublishSurfacePressure)
{
  sensor.Configure(model, Config(), ecm, events);
  ASSERT_TRUE(Wait([&]() { return pressureSub->get_publisher_count() > 0; }));
  ecm.RemoveComponent<gz::sim::components::Pose>(model);
  Tick(125);
  ExpectSilence();
  ecm.CreateComponent(
    model, gz::sim::components::Pose(
             gz::math::Pose3d(0, 0, std::numeric_limits<double>::quiet_NaN(), 0, 0, 0)));
  Tick(250);
  ExpectSilence();
  ecm.SetComponentData<gz::sim::components::Pose>(model, gz::math::Pose3d(0, 0, -5, 0, 0, 0));
  Tick(375);
  ASSERT_TRUE(Wait(
    [&]() { return !pressure.empty() && pressure.back().header.stamp.nanosec == 375000000u; }));
  EXPECT_NEAR(pressure.back().fluid_pressure, 101325.0 + 5.0 * 9806.38, 1e-6);
}

TEST_F(PressureBehavior, RejectsInvalidNamesAndNonModelAttachmentWithoutThrowing)
{
  auto config = Config("<topic>invalid topic</topic>");
  EXPECT_NO_THROW(sensor.Configure(model, config, ecm, events));
  Tick(125);
  ExpectSilence();
  ecm.RemoveComponent<gz::sim::components::Model>(model);
  EXPECT_NO_THROW(sensor.Configure(model, Config(), ecm, events));
  Tick(250);
  ExpectSilence();
}

TEST_F(PressureBehavior, MissingNamespaceAndInvalidPressureParametersStayInactive)
{
  auto config = Config();
  config->RemoveChild(config->GetElement("namespace"));
  EXPECT_NO_THROW(sensor.Configure(model, config, ecm, events));
  Tick(125);
  ExpectSilence();
  for (const auto & extra :
       {"<kPa_per_meter>0</kPa_per_meter>", "<kPa_per_meter>-1</kPa_per_meter>",
        "<kPa_per_meter>nan</kPa_per_meter>", "<standard_pressure>inf</standard_pressure>",
        "<standard_pressure>-1</standard_pressure>", "<update_rate>nan</update_rate>",
        "<kPa_per_meter>invalid</kPa_per_meter>"})
  {
    posim_gz_sensor_plugins::SubseaPressureSensorPlugin invalid;
    EXPECT_NO_THROW(invalid.Configure(model, Config(extra), ecm, events));
    gz::sim::UpdateInfo info;
    info.simTime = std::chrono::milliseconds(250);
    info.paused = false;
    invalid.PreUpdate(info, ecm);
    invalid.PostUpdate(info, ecm);
    ExpectSilence();
  }
}

class InvalidPressureText : public PressureBehavior,
                            public ::testing::WithParamInterface<std::string>
{
};

TEST_P(InvalidPressureText, DoesNotAdvertiseOrPublish)
{
  sensor.Configure(model, Config(GetParam()), ecm, events);
  EXPECT_FALSE(
    Wait([&]() { return pressureSub->get_publisher_count() > 0; }, std::chrono::milliseconds(300)));
  Tick(125);
  ExpectSilence();
}

INSTANTIATE_TEST_SUITE_P(
  Parameters, InvalidPressureText,
  ::testing::Values(
    "<standard_pressure>invalid</standard_pressure>", "<update_rate>invalid</update_rate>",
    "<estimate_depth_on>invalid</estimate_depth_on>"));
