#include <gtest/gtest.h>
#include <chrono>
#include <csignal>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/EventManager.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <thread>
#include "posim_gz_sensor_plugins/sea_pressure_sensor.hh"

TEST(SeaPressure, DerivedDepthUsesTheSameFullMeasurementStamp)
{
  rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  {
    auto node = std::make_shared<rclcpp::Node>("pressure_stamp_observer");
    sensor_msgs::msg::FluidPressure::SharedPtr pressure;
    geometry_msgs::msg::PointStamped::SharedPtr depth;
    auto ps = node->create_subscription<sensor_msgs::msg::FluidPressure>(
      "/model/pressure_stamp_test/sea_pressure", 10,
      [&](sensor_msgs::msg::FluidPressure::SharedPtr msg) { pressure = msg; });
    auto ds = node->create_subscription<geometry_msgs::msg::PointStamped>(
      "/model/pressure_stamp_test/sea_pressure_depth", 10,
      [&](geometry_msgs::msg::PointStamped::SharedPtr msg) { depth = msg; });
    posim_gz_sensor_plugins::SubseaPressureSensorPlugin sensor;
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::Name("pressure_stamp_test"));
    ecm.CreateComponent(entity, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -5, 0, 0, 0)));
    auto plugin = std::make_shared<sdf::Element>();
    plugin->SetName("plugin");
    auto ns = std::make_shared<sdf::Element>();
    ns->SetName("namespace");
    ns->AddValue("string", "pressure_stamp_test", true);
    plugin->InsertElement(ns);
    sensor.Configure(entity, plugin, ecm, events);
    const auto discovery = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while ((ps->get_publisher_count() == 0 || ds->get_publisher_count() == 0) &&
           std::chrono::steady_clock::now() < discovery)
    {
      rclcpp::spin_some(node);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_GT(ps->get_publisher_count(), 0u);
    EXPECT_GT(ds->get_publisher_count(), 0u);
    for (int milliseconds : {125, 325})
    {
      pressure.reset();
      depth.reset();
      gz::sim::UpdateInfo info;
      info.simTime = std::chrono::milliseconds(milliseconds);
      info.iterations = 1;
      info.paused = false;
      sensor.PreUpdate(info, ecm);
      sensor.PostUpdate(info, ecm);
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
      while ((!pressure || !depth) && std::chrono::steady_clock::now() < deadline)
      {
        rclcpp::spin_some(node);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      EXPECT_TRUE(pressure);
      EXPECT_TRUE(depth);
      if (pressure && depth)
      {
        EXPECT_EQ(pressure->header.stamp, depth->header.stamp);
        EXPECT_EQ(depth->header.stamp.nanosec, static_cast<unsigned>(milliseconds * 1000000));
        EXPECT_NEAR(pressure->fluid_pressure, 101325.0 + 5.0 * 9806.38, 1e-6);
        EXPECT_NEAR(depth->point.z, 5.0, 1e-9);
        EXPECT_DOUBLE_EQ(pressure->variance, 9000000.0);
      }
    }
  }
  rclcpp::shutdown();
}
