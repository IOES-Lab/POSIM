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
#include <gz/sim/components/RgbdCamera.hh>
#include <limits>
#include <sdf/Camera.hh>
#include <sdf/Sensor.hh>
#include <thread>
#include <vector>
#include "posim_gz_sensor_plugins/UnderwaterCamera.hh"

class CameraSynchronization : public ::testing::Test
{
protected:
  void SetUp() override
  {
    unsetenv("POSIM_UNDERWATER_PLUGIN");
    sdf::Camera config;
    config.SetImageWidth(2);
    config.SetImageHeight(2);
    config.SetNearClip(0.1);
    config.SetFarClip(100);
    config.SetLensIntrinsicsFx(2);
    config.SetLensIntrinsicsFy(2);
    config.SetLensIntrinsicsCx(1);
    config.SetLensIntrinsicsCy(1);
    static unsigned serial = 0;
    const auto name = "camera_synchronization_" + std::to_string(++serial);
    sdf::Sensor sensor;
    sensor.SetName(name);
    sensor.SetType(sdf::SensorType::RGBD_CAMERA);
    sensor.SetTopic("/" + name);
    sensor.SetCameraSensor(config);
    const auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::RgbdCamera(sensor));
    auto plugin = std::make_shared<sdf::Element>();
    plugin->SetName("plugin");
    camera.Configure(entity, plugin, ecm, events);
    observer = std::make_shared<rclcpp::Node>(name + "_observer");
    subscription = observer->create_subscription<sensor_msgs::msg::Image>(
      "/" + name + "/simulated_image", 10,
      [this](sensor_msgs::msg::Image::SharedPtr message) { received.push_back(*message); });
    executor = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    executor->add_node(observer);
    ASSERT_TRUE(Wait([&]() { return subscription->get_publisher_count() == 1; }));
  }

  template <typename Predicate>
  bool Wait(Predicate predicate, std::chrono::milliseconds duration = std::chrono::seconds(2))
  {
    const auto deadline = std::chrono::steady_clock::now() + duration;
    while (!predicate() && std::chrono::steady_clock::now() < deadline)
    {
      executor->spin_some();
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return predicate();
  }

  gz::msgs::Image Image(int seconds, bool depth, float value = 3, int nanoseconds = 0)
  {
    gz::msgs::Image image;
    image.set_width(2);
    image.set_height(2);
    image.set_step(depth ? 8 : 6);
    image.set_pixel_format_type(depth ? gz::msgs::R_FLOAT32 : gz::msgs::BGR_INT8);
    image.mutable_header()->mutable_stamp()->set_sec(seconds);
    image.mutable_header()->mutable_stamp()->set_nsec(nanoseconds);
    if (depth)
    {
      const float pixels[] = {value, value, value, value};
      image.set_data(pixels, sizeof(pixels));
    }
    else
    {
      image.set_data(std::string(12, '\x64'));
      auto frame = image.mutable_header()->add_data();
      frame->set_key("frame_id");
      frame->add_value("vehicle::camera::optical");
    }
    return image;
  }

  void Start()
  {
    camera.DepthImageCallback(Image(1, true));
    camera.CameraCallback(Image(1, false));
    camera.CameraCallback(Image(1, false));
    ASSERT_TRUE(Wait([&]() { return !received.empty(); }));
    received.clear();
  }

  void ExpectSilence()
  {
    EXPECT_FALSE(Wait([&]() { return !received.empty(); }, std::chrono::milliseconds(150)));
  }

  void ExpectFrame(int seconds, double depth, int nanoseconds = 0)
  {
    ASSERT_TRUE(Wait([&]() { return !received.empty(); }));
    ASSERT_EQ(received.size(), 1u);
    const auto & frame = received.front();
    EXPECT_EQ(frame.header.stamp.sec, seconds);
    EXPECT_EQ(frame.header.stamp.nanosec, static_cast<unsigned>(nanoseconds));
    EXPECT_EQ(frame.header.frame_id, "vehicle/camera/optical");
    EXPECT_EQ(frame.encoding, "bgr8");
    ASSERT_EQ(frame.data.size(), 12u);
    // At (1, 1), range equals depth; this distinguishes stale from matched depth.
    EXPECT_NEAR(frame.data[9], 100 * std::exp(-depth / 30.0), 1);
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  posim_gz_sensor_plugins::UnderwaterCamera camera;
  std::shared_ptr<rclcpp::Node> observer;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor;
  std::vector<sensor_msgs::msg::Image> received;
};

TEST_F(CameraSynchronization, FirstPairWithColorFirst)
{
  camera.CameraCallback(Image(0, false));
  camera.DepthImageCallback(Image(0, true, 30));
  ExpectFrame(0, 30);
}

TEST_F(CameraSynchronization, FirstPairWithDepthFirst)
{
  camera.DepthImageCallback(Image(0, true, 30));
  camera.CameraCallback(Image(0, false));
  ExpectFrame(0, 30);
}

TEST_F(CameraSynchronization, ColorWaitsForItsDepth)
{
  Start();
  camera.CameraCallback(Image(2, false));
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30));
  ExpectFrame(2, 30);
}

TEST_F(CameraSynchronization, DepthWaitsForItsColor)
{
  Start();
  camera.DepthImageCallback(Image(2, true, 30));
  ExpectSilence();
  camera.CameraCallback(Image(2, false));
  ExpectFrame(2, 30);
}

TEST_F(CameraSynchronization, NanosecondsMustMatch)
{
  Start();
  camera.DepthImageCallback(Image(2, true, 3, 100));
  camera.CameraCallback(Image(2, false, 0, 200));
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30, 200));
  ExpectFrame(2, 30, 200);
}

TEST_F(CameraSynchronization, QueuedColorsRetainTheirOwnedPixels)
{
  Start();
  auto color = Image(2, false);
  camera.CameraCallback(color);
  color.set_data(std::string(12, '\0'));
  camera.CameraCallback(Image(3, false));
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30));
  ExpectFrame(2, 30);
  received.clear();
  camera.DepthImageCallback(Image(3, true, 60));
  ExpectFrame(3, 60);
}

TEST_F(CameraSynchronization, CompletedAndLatePairsAreNotRepublished)
{
  Start();
  camera.DepthImageCallback(Image(1, true));
  camera.CameraCallback(Image(1, false));
  camera.DepthImageCallback(Image(0, true));
  camera.CameraCallback(Image(0, false));
  ExpectSilence();
}

TEST_F(CameraSynchronization, QueuedDepthsRetainTheirOwnedPixels)
{
  Start();
  auto depth = Image(2, true, 30);
  camera.DepthImageCallback(depth);
  depth.set_data(std::string(16, '\0'));
  camera.DepthImageCallback(Image(3, true, 60));
  camera.CameraCallback(Image(2, false));
  ExpectFrame(2, 30);
  received.clear();
  camera.CameraCallback(Image(3, false));
  ExpectFrame(3, 60);
}

TEST_F(CameraSynchronization, InvalidTimestampsAreRejected)
{
  Start();
  const std::pair<int64_t, int32_t> invalid[] = {
    {static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1, 0},
    {static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1, 0},
    {2, -1},
    {2, 1000000000}};
  for (const auto & stamp : invalid)
  {
    auto color = Image(2, false);
    auto depth = Image(2, true);
    for (auto * image : {&color, &depth})
    {
      image->mutable_header()->mutable_stamp()->set_sec(stamp.first);
      image->mutable_header()->mutable_stamp()->set_nsec(stamp.second);
    }
    camera.DepthImageCallback(depth);
    camera.CameraCallback(color);
  }
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30));
  camera.CameraCallback(Image(2, false));
  ExpectFrame(2, 30);
}

TEST_F(CameraSynchronization, MalformedDepthDoesNotConsumeColor)
{
  Start();
  camera.CameraCallback(Image(2, false));
  auto bad = Image(2, true);
  bad.set_data("short");
  camera.DepthImageCallback(bad);
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30));
  ExpectFrame(2, 30);
}

TEST_F(CameraSynchronization, RewindClearsPendingFrames)
{
  gz::sim::UpdateInfo info;
  info.simTime = std::chrono::seconds(3);
  camera.PostUpdate(info, ecm);
  Start();
  camera.DepthImageCallback(Image(2, true, 60));
  info.simTime = std::chrono::seconds(0);
  camera.PostUpdate(info, ecm);
  camera.CameraCallback(Image(0, false));
  ExpectSilence();
  camera.DepthImageCallback(Image(0, true, 30));
  ExpectFrame(0, 30);
  received.clear();
  camera.CameraCallback(Image(2, false));
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 3));
  ExpectFrame(2, 3);
}

TEST_F(CameraSynchronization, OldUnmatchedFramesAreEvicted)
{
  Start();
  for (int time = 2; time < 20; ++time)
  {
    camera.CameraCallback(Image(time, false));
  }
  ExpectSilence();
  camera.DepthImageCallback(Image(2, true, 30));
  ExpectSilence();
  camera.DepthImageCallback(Image(19, true, 60));
  ExpectFrame(19, 60);
}

TEST_F(CameraSynchronization, OldUnmatchedDepthsAreEvicted)
{
  Start();
  for (int time = 2; time < 20; ++time)
  {
    camera.DepthImageCallback(Image(time, true, 60));
  }
  camera.CameraCallback(Image(2, false));
  ExpectSilence();
  camera.CameraCallback(Image(19, false));
  ExpectFrame(19, 60);
}
