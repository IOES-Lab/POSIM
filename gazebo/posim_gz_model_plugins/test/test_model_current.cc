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
#include <chrono>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/World.hh>
#include <optional>
#include <sdf/Root.hh>
#include <thread>
#include "posim_gz_model_plugins/OceanCurrentModelPlugin.hh"

using posim_gz_model_plugins::OceanCurrentModelPlugin;
using Database = posim_interfaces::msg::StratifiedCurrentDatabase;
using Twist = geometry_msgs::msg::TwistStamped;

class ModelCurrent : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    }
    world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    model = ecm.CreateEntity();
    ecm.CreateComponent(model, gz::sim::components::Model());
    ecm.CreateComponent(model, gz::sim::components::Name("vehicle"));
    ecm.CreateComponent(model, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -5, 0, 0, 0)));
    observer = std::make_shared<rclcpp::Node>("current_test_observer");
    subscriber = observer->create_subscription<Twist>(
      "/model/vehicle/ocean_current", 10, [this](Twist::ConstSharedPtr msg) { latest = *msg; });
  }

  sdf::ElementPtr Config(const std::string & settings = "", bool tide = false)
  {
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
      "<namespace>vehicle</namespace><transient_current><velocity_north>" +
      settings + "</velocity_north></transient_current><tide_oscillation>" +
      (tide ? "true" : "false") + "</tide_oscillation></plugin></world></sdf>");
    EXPECT_TRUE(errors.empty());
    return root.Element()->GetElement("world")->GetElement("plugin");
  }

  std::shared_ptr<Database> Profile()
  {
    auto data = std::make_shared<Database>();
    data->depths = {0, 10};
    data->velocities.resize(2);
    data->velocities[0].x = 0.2;
    data->velocities[1].x = 0.8;
    return data;
  }

  Twist Sample(OceanCurrentModelPlugin & plugin, double seconds)
  {
    latest.reset();
    gz::sim::UpdateInfo info;
    info.paused = false;
    info.simTime = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(seconds));
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!latest && std::chrono::steady_clock::now() < deadline)
    {
      plugin.Update(info, ecm);
      plugin.PostUpdate(info, ecm);
      rclcpp::spin_some(observer);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(latest.has_value());
    return latest.value_or(Twist());
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity world;
  gz::sim::Entity model;
  rclcpp::Node::SharedPtr observer;
  rclcpp::Subscription<Twist>::SharedPtr subscriber;
  std::optional<Twist> latest;
};

TEST_F(ModelCurrent, IncompleteConfigurationIsSafeToUpdate)
{
  auto config = Config();
  config->RemoveChild(config->GetElement("namespace"));
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, config, ecm, events);
  gz::sim::UpdateInfo info;
  info.paused = false;
  plugin.Update(info, ecm);
  plugin.PostUpdate(info, ecm);
  SUCCEED();
}

TEST_F(ModelCurrent, UsesSecondsForNoiseAndPreservesSubsecondStamp)
{
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, Config("<mu>0.5</mu><noiseAmp>1</noiseAmp>"), ecm, events);
  plugin.UpdateDatabase(Profile());
  Sample(plugin, 1.0);
  const auto message = Sample(plugin, 1.5);
  EXPECT_NEAR(message.twist.linear.x, 0.375, 1e-6);
  EXPECT_EQ(message.header.stamp.sec, 1);
  EXPECT_EQ(message.header.stamp.nanosec, 500000000u);
}

TEST_F(ModelCurrent, ClampsOutsideTheDepthProfile)
{
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, Config(), ecm, events);
  auto data = Profile();
  data->depths = {10, 20};
  plugin.UpdateDatabase(data);
  EXPECT_NEAR(Sample(plugin, 1).twist.linear.x, 0.2, 1e-6);
  ecm.Component<gz::sim::components::Pose>(model)->Data().Pos().Z(-30);
  EXPECT_NEAR(Sample(plugin, 2).twist.linear.x, 0.8, 1e-6);
}

TEST_F(ModelCurrent, MalformedDatabaseKeepsLastValidProfile)
{
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, Config(), ecm, events);
  plugin.UpdateDatabase(Profile());
  EXPECT_NEAR(Sample(plugin, 1).twist.linear.x, 0.5, 1e-6);
  auto data = Profile();
  data->velocities[0].x = 99;
  data->depths[1] = -1;
  plugin.UpdateDatabase(data);
  EXPECT_NEAR(Sample(plugin, 2).twist.linear.x, 0.5, 1e-6);
  data->velocities.clear();
  plugin.UpdateDatabase(data);
  EXPECT_NEAR(Sample(plugin, 3).twist.linear.x, 0.5, 1e-6);
}

TEST_F(ModelCurrent, UsesAttachedModelPoseAndHoldsAtRepeatedTime)
{
  ecm.Component<gz::sim::components::Name>(model)->Data() = "different_model_name";
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, Config(), ecm, events);
  plugin.UpdateDatabase(Profile());
  EXPECT_NEAR(Sample(plugin, 1).twist.linear.x, 0.5, 1e-6);
  ecm.Component<gz::sim::components::Pose>(model)->Data().Pos().Z(-10);
  EXPECT_NEAR(Sample(plugin, 1).twist.linear.x, 0.5, 1e-6);
  EXPECT_NEAR(Sample(plugin, 2).twist.linear.x, 0.8, 1e-6);
  EXPECT_NEAR(Sample(plugin, 0).twist.linear.x, 0.8, 1e-6);
}

TEST_F(ModelCurrent, TidalDatabaseUsesSecondsAndRejectsPartialUpdates)
{
  OceanCurrentModelPlugin plugin;
  plugin.Configure(model, Config("", true), ecm, events);
  auto data = Profile();
  data->world_start_time_year = 2020;
  data->world_start_time_month = 1;
  data->world_start_time_day = 1;
  data->time_gmt_year = {2020, 2020};
  data->time_gmt_month = {1, 1};
  data->time_gmt_day = {1, 1};
  data->time_gmt_hour = {0, 1};
  data->time_gmt_minute = {0, 0};
  data->tidevelocities = {100, 300};
  plugin.UpdateDatabase(data);
  EXPECT_NEAR(Sample(plugin, 1800).twist.linear.x, -1, 1e-6);
  data->time_gmt_month.clear();
  data->velocities[0].x = 99;
  plugin.UpdateDatabase(data);
  EXPECT_NEAR(Sample(plugin, 3600).twist.linear.x, -1.5, 1e-6);
}
