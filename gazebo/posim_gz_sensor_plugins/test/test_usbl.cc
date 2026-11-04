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
#include <cmath>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <sdf/Root.hh>
#include <thread>
#include "posim_gz_sensor_plugins/UsblTransceiver.hh"
#include "posim_gz_sensor_plugins/UsblTransponder.hh"

using posim_gz_sensor_plugins::UsblTransceiver;
using posim_gz_sensor_plugins::UsblTransponder;

class Usbl : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    }
    observer = std::make_shared<rclcpp::Node>("usbl_test_observer");
    model = ecm.CreateEntity();
    ecm.CreateComponent(model, gz::sim::components::Model());
    ecm.CreateComponent(model, gz::sim::components::Name("box"));
    ecm.CreateComponent(model, gz::sim::components::Pose());
    auto link = ecm.CreateEntity();
    ecm.CreateComponent(link, gz::sim::components::Link());
    ecm.CreateComponent(link, gz::sim::components::Name("link"));
    ecm.CreateComponent(link, gz::sim::components::Pose());
    ecm.CreateComponent(link, gz::sim::components::ParentEntity(model));
    ecm.SetParentEntity(link, model);
  }

  std::shared_ptr<sdf::Element> Config(const std::string & extra = "")
  {
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
      "<namespace>USBL_test</namespace><transceiver_device>tx</transceiver_device>"
      "<transceiver_ID>168</transceiver_ID><transponder_device>rx</transponder_device>"
      "<transponder_ID>1</transponder_ID><transceiver_model>box</transceiver_model>"
      "<transponder_attached_object>box</transponder_attached_object>"
      "<enable_ping_scheduler>false</enable_ping_scheduler><sigma>0</sigma>" +
      extra + "</plugin></world></sdf>");
    EXPECT_TRUE(errors.empty());
    return root.Element()->GetElement("world")->GetElement("plugin");
  }

  template <typename Predicate>
  bool Wait(Predicate predicate)
  {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!predicate() && std::chrono::steady_clock::now() < deadline)
    {
      rclcpp::spin_some(observer);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return predicate();
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity model;
  std::shared_ptr<rclcpp::Node> observer;
};

TEST_F(Usbl, UnknownCommandTargetsDoNotThrowOrCrash)
{
  UsblTransceiver transceiver;
  transceiver.Configure(model, Config(), ecm, events);
  for (std::string id : {"unknown", "99999999999999999999", "99", "1suffix", ""})
  {
    EXPECT_NO_THROW(transceiver.sendCommand(1, id));
  }
}

TEST_F(Usbl, IncompleteConfigurationStaysInactive)
{
  UsblTransceiver transceiver;
  auto config = Config();
  config->RemoveChild(config->GetElement("transponder_ID"));
  transceiver.Configure(model, config, ecm, events);
  gz::sim::UpdateInfo info;
  info.paused = false;
  info.iterations = 1;
  transceiver.PostUpdate(info, ecm);
  SUCCEED();
}

TEST_F(Usbl, InvalidIdsAndSchedulerValuesAreRejected)
{
  auto ping = observer->create_subscription<std_msgs::msg::String>(
    "/USBL_test/common_interrogation_ping", 10, [](std_msgs::msg::String::SharedPtr) {});
  for (const std::string id : {"01", "-1", "1tail", "2147483648", "1,1", "1,2", "1,"})
  {
    UsblTransceiver transceiver;
    auto config = Config();
    config->GetElement("transponder_ID")->Set(id);
    EXPECT_NO_THROW(transceiver.Configure(model, config, ecm, events));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(ping->get_publisher_count(), 0u) << id;
  }
  for (const std::string frequency : {"0", "-1", "nan", "inf"})
  {
    UsblTransceiver transceiver;
    auto config = Config("<ping_frequency>" + frequency + "</ping_frequency>");
    config->GetElement("enable_ping_scheduler")->Set(true);
    transceiver.Configure(model, config, ecm, events);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(ping->get_publisher_count(), 0u) << frequency;
  }
}

TEST_F(Usbl, CoincidentPositionsHaveFiniteAngles)
{
  UsblTransceiver transceiver;
  transceiver.Configure(model, Config(), ecm, events);
  double bearing, range, elevation;
  transceiver.calculateRelativePose({0, 0, 0}, bearing, range, elevation);
  EXPECT_DOUBLE_EQ(range, 0.0);
  EXPECT_DOUBLE_EQ(bearing, 0.0);
  EXPECT_DOUBLE_EQ(elevation, 0.0);
}

TEST_F(Usbl, ResponsePreservesFullTransceiverIdAndChecksDestination)
{
  UsblTransponder transponder;
  transponder.Configure(model, Config(), ecm, events);
  posim_interfaces::msg::UsblResponse::SharedPtr response;
  auto subscription = observer->create_subscription<posim_interfaces::msg::UsblResponse>(
    "/USBL_test/tx_168/command_response", 10,
    [&](posim_interfaces::msg::UsblResponse::SharedPtr msg) { response = msg; });
  ASSERT_TRUE(Wait([&]() { return subscription->get_publisher_count() > 0; }));
  posim_interfaces::msg::UsblCommand command;
  command.command_id = 1;
  command.transponder_id = 1;
  transponder.commandRosCallback(command);
  ASSERT_TRUE(Wait([&]() { return response != nullptr; }));
  EXPECT_EQ(response->transceiver_id, 168);
  response.reset();
  command.transponder_id = 2;
  transponder.commandRosCallback(command);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  rclcpp::spin_some(observer);
  EXPECT_EQ(response, nullptr);
}

TEST_F(Usbl, InvalidChannelDoesNotChangeSelectionAndPositionsWaitForUpdate)
{
  UsblTransceiver transceiver;
  auto config = Config();
  config->GetElement("transponder_ID")->Set("1,2");
  config->GetElement("transponder_attached_object")->Set("box,box");
  transceiver.Configure(model, config, ecm, events);
  std::vector<posim_interfaces::msg::Location> positions;
  auto subscription = observer->create_subscription<posim_interfaces::msg::Location>(
    "/USBL_test/tx_168/transponder_location", 10,
    [&](posim_interfaces::msg::Location::SharedPtr msg) { positions.push_back(*msg); });
  ASSERT_TRUE(Wait([&]() { return subscription->get_publisher_count() > 0; }));
  auto channel = std::make_shared<std_msgs::msg::String>();
  channel->data = "1";
  transceiver.channelSwitchCallback(channel);
  channel->data = "99";
  transceiver.channelSwitchCallback(channel);
  gz::msgs::Vector3d position;
  position.set_x(3);
  position.set_y(4);
  position.set_z(0);
  for (const auto * id : {"1", "2", "99"})
  {
    transceiver.receiveGazeboCallback(id, position);
  }
  gz::sim::UpdateInfo info;
  info.paused = true;
  transceiver.PostUpdate(info, ecm);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  rclcpp::spin_some(observer);
  EXPECT_TRUE(positions.empty());
  info.paused = false;
  transceiver.PostUpdate(info, ecm);
  ASSERT_TRUE(Wait([&]() { return !positions.empty(); }));
  EXPECT_EQ(positions.size(), 1u);
  EXPECT_EQ(positions.front().transponder_id, 1);
  EXPECT_NEAR(positions.front().x, 53.130102354, 1e-6);
  EXPECT_DOUBLE_EQ(positions.front().y, 5.0);
  EXPECT_DOUBLE_EQ(positions.front().z, 0.0);
}

TEST_F(Usbl, SchedulerUsesSimulationTimeAndHandlesPauseAndRewind)
{
  UsblTransceiver transceiver;
  auto config = Config("<ping_frequency>0.5</ping_frequency>");
  config->GetElement("enable_ping_scheduler")->Set(true);
  transceiver.Configure(model, config, ecm, events);
  size_t count = 0;
  auto subscription = observer->create_subscription<std_msgs::msg::String>(
    "/USBL_test/common_interrogation_ping", 10, [&](std_msgs::msg::String::SharedPtr) { ++count; });
  ASSERT_TRUE(Wait([&]() { return subscription->get_publisher_count() > 0; }));
  gz::sim::UpdateInfo info;
  info.paused = false;
  info.iterations = 123;  // Deliberately unrelated to simulation milliseconds.
  info.simTime = std::chrono::seconds(1);
  transceiver.PostUpdate(info, ecm);
  ASSERT_TRUE(Wait([&]() { return count == 1; }));
  info.simTime = std::chrono::seconds(2);
  transceiver.PostUpdate(info, ecm);
  info.paused = true;
  info.simTime = std::chrono::seconds(3);
  transceiver.PostUpdate(info, ecm);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  rclcpp::spin_some(observer);
  EXPECT_EQ(count, 1u);
  info.paused = false;
  transceiver.PostUpdate(info, ecm);
  ASSERT_TRUE(Wait([&]() { return count == 2; }));
  info.simTime = std::chrono::seconds(0);
  transceiver.PostUpdate(info, ecm);
  ASSERT_TRUE(Wait([&]() { return count == 3; }));
}

TEST_F(Usbl, HighSchedulerFrequencyDoesNotDivideByZero)
{
  UsblTransceiver transceiver;
  auto config = Config("<ping_frequency>2000</ping_frequency>");
  config->GetElement("enable_ping_scheduler")->Set(true);
  transceiver.Configure(model, config, ecm, events);
  gz::sim::UpdateInfo info;
  info.paused = false;
  info.simTime = std::chrono::milliseconds(1);
  transceiver.PostUpdate(info, ecm);
  info.simTime = std::chrono::milliseconds(2);
  transceiver.PostUpdate(info, ecm);
  SUCCEED();
}

TEST_F(Usbl, InvalidTransponderConfigurationAndLinklessModelsStayInactive)
{
  auto subscription = observer->create_subscription<posim_interfaces::msg::UsblResponse>(
    "/USBL_invalid/tx_168/command_response", 10,
    [](posim_interfaces::msg::UsblResponse::SharedPtr) {});
  for (const auto * extra : {"<mu>nan</mu>", "<sound_speed>0</sound_speed>"})
  {
    UsblTransponder transponder;
    auto config = Config(extra);
    config->GetElement("namespace")->Set("USBL_invalid");
    transponder.Configure(model, config, ecm, events);
    transponder.sendLocation();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(subscription->get_publisher_count(), 0u);
  }
  const auto linkless = ecm.CreateEntity();
  ecm.CreateComponent(linkless, gz::sim::components::Model());
  ecm.CreateComponent(linkless, gz::sim::components::Name("linkless"));
  UsblTransponder transponder;
  UsblTransceiver transceiver;
  transponder.Configure(linkless, Config(), ecm, events);
  transceiver.Configure(linkless, Config(), ecm, events);
  gz::sim::UpdateInfo info;
  info.paused = false;
  transponder.PostUpdate(info, ecm);
  transceiver.PostUpdate(info, ecm);
}
