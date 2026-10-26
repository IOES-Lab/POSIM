// Copyright 2026 Kwon Yeseol
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
#include <atomic>
#include <chrono>
#include <csignal>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/EventManager.hh>
#include <gz/transport/Node.hh>
#include <thread>
#include "posim_ros_gz_plugins/DVLBridge.hh"

using posim_ros_gz_plugins::DVLBridge;
class DvlInput : public ::testing::Test
{
protected:
  std::unique_ptr<DVLBridge> bridge;
  rclcpp::Node::SharedPtr node;
  rclcpp::Subscription<posim_interfaces::msg::DVL>::SharedPtr subscription;
  posim_interfaces::msg::DVL::SharedPtr received;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor;
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    }
    executor = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    bridge = std::make_unique<DVLBridge>();
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto sdf = std::make_shared<sdf::Element>();
    sdf->SetName("plugin");
    bridge->Configure(0, sdf, ecm, events);
    node = std::make_shared<rclcpp::Node>("dvl_observer");
    executor->add_node(node);
    subscription = node->create_subscription<posim_interfaces::msg::DVL>(
      "/dvl/velocity", 10, [this](posim_interfaces::msg::DVL::SharedPtr m) { received = m; });
  }
  void TearDown() override
  {
    bridge.reset();
    subscription.reset();
    executor->remove_node(node);
    executor.reset();
    node.reset();
    rclcpp::shutdown();
  }
  void Send(const gz::msgs::DVLVelocityTracking & msg)
  {
    for (int i = 0; i < 150 && !received; ++i)
    {
      bridge->receiveGazeboCallback(msg);
      executor->spin_some();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
};
TEST_F(DvlInput, MapsLinearCovarianceIntoSixBySix)
{
  gz::msgs::DVLVelocityTracking msg;
  msg.mutable_header()->mutable_stamp()->set_sec(3);
  msg.mutable_header()->mutable_stamp()->set_nsec(42);
  auto frame = msg.mutable_header()->add_data();
  frame->set_key("frame_id");
  frame->add_value("rov::dvl");
  auto beam = msg.add_beams();
  beam->set_id(2);
  beam->set_locked(true);
  for (double v : {1., .1, .2, .1, 2., .3, .2, .3, 3.})
  {
    msg.mutable_velocity()->add_covariance(v);
    beam->mutable_velocity()->add_covariance(2 * v);
  }
  Send(msg);
  ASSERT_TRUE(received);
  EXPECT_EQ(received->header.frame_id, "rov/dvl");
  EXPECT_EQ(received->header.stamp.nanosec, 42u);
  ASSERT_EQ(received->beams.size(), 1u);
  for (size_t row = 0; row < 6; ++row)
  {
    for (size_t col = 0; col < 6; ++col)
    {
      const double value = row < 3 && col < 3 ? msg.velocity().covariance(row * 3 + col) : 0;
      EXPECT_DOUBLE_EQ(received->velocity.covariance[row * 6 + col], value);
      EXPECT_DOUBLE_EQ(received->beams[0].velocity.covariance[row * 6 + col], 2 * value);
    }
  }
}
TEST_F(DvlInput, EmptyCovarianceRemainsZero)
{
  gz::msgs::DVLVelocityTracking msg;
  Send(msg);
  ASSERT_TRUE(received);
  for (auto v : received->velocity.covariance)
  {
    EXPECT_EQ(v, 0);
  }
}
TEST_F(DvlInput, ExcessCovarianceCannotOverflow)
{
  gz::msgs::DVLVelocityTracking msg;
  auto beam = msg.add_beams();
  for (int i = 0; i < 100; ++i)
  {
    msg.mutable_velocity()->add_covariance(i);
    beam->mutable_velocity()->add_covariance(i);
  }
  Send(msg);
  ASSERT_TRUE(received);
  EXPECT_EQ(received->velocity.covariance[14], 8);
  EXPECT_EQ(received->velocity.covariance[35], 0);
}
TEST(DvlLifecycle, PreservesHostSignalHandler)
{
  if (rclcpp::ok())
  {
    rclcpp::shutdown();
  }
  auto handler = +[](int) {};
  auto previous = std::signal(SIGINT, handler);
  {
    DVLBridge bridge;
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto sdf = std::make_shared<sdf::Element>();
    sdf->SetName("plugin");
    bridge.Configure(0, sdf, ecm, events);
    EXPECT_EQ(std::signal(SIGINT, handler), handler);
  }
  rclcpp::shutdown();
  std::signal(SIGINT, previous);
}
TEST(DvlLifecycle, CallbackWithoutConfigureIsSafe)
{
  DVLBridge bridge;
  gz::msgs::DVLVelocityTracking message;
  EXPECT_NO_THROW(bridge.receiveGazeboCallback(message));
}

TEST(DvlLifecycle, TransportTrafficDuringDestruction)
{
  rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  gz::transport::Node publisher;
  auto pub = publisher.Advertise<gz::msgs::DVLVelocityTracking>("/dvl/velocity");
  std::atomic<bool> running{true};
  std::thread producer(
    [&]()
    {
      gz::msgs::DVLVelocityTracking msg;
      while (running.load())
      {
        pub.Publish(msg);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
      }
    });
  bool observedConnection = false;
  for (int i = 0; i < 30; ++i)
  {
    DVLBridge bridge;
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto sdf = std::make_shared<sdf::Element>();
    sdf->SetName("plugin");
    bridge.Configure(0, sdf, ecm, events);
    for (int attempt = 0; attempt < 100 && !pub.HasConnections(); ++attempt)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    observedConnection = observedConnection || pub.HasConnections();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  running = false;
  producer.join();
  EXPECT_TRUE(observedConnection);
  rclcpp::shutdown();
}

TEST_F(DvlInput, CallbackAfterContextShutdownIsSafe)
{
  rclcpp::shutdown();
  gz::msgs::DVLVelocityTracking message;
  EXPECT_NO_THROW(bridge->receiveGazeboCallback(message));
}
