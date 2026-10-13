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

TEST(SeaPressure, PreservesSignalHandlerAndToleratesContextShutdown)
{
  auto handler = +[](int) {};
  auto previous = std::signal(SIGINT, handler);
  {
    posim_gz_sensor_plugins::SubseaPressureSensorPlugin sensor;
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::Name("pressure_shutdown_test"));
    ecm.CreateComponent(entity, gz::sim::components::Pose(gz::math::Pose3d(0, 0, -5, 0, 0, 0)));
    auto plugin = std::make_shared<sdf::Element>();
    plugin->SetName("plugin");
    auto ns = std::make_shared<sdf::Element>();
    ns->SetName("namespace");
    ns->AddValue("string", "pressure_shutdown_test", true);
    plugin->InsertElement(ns);
    sensor.Configure(entity, plugin, ecm, events);
    EXPECT_EQ(std::signal(SIGINT, handler), handler);
    gz::sim::UpdateInfo info;
    info.simTime = std::chrono::milliseconds(125);
    info.iterations = 1;
    info.paused = false;
    sensor.PreUpdate(info, ecm);
    rclcpp::shutdown();
    EXPECT_NO_THROW(sensor.PostUpdate(info, ecm));
  }
  std::signal(SIGINT, previous);
}
