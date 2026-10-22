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

// Regression tests for malformed camera input and acquisition metadata.
#include <gtest/gtest.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/RgbdCamera.hh>
#include <gz/sim/components/World.hh>
#include <limits>
#include <sdf/Camera.hh>
#include <sdf/Sensor.hh>
#include <thread>
#include "posim_gz_sensor_plugins/UnderwaterCamera.hh"

using posim_gz_sensor_plugins::UnderwaterCamera;
namespace
{
gz::msgs::Image RGB(gz::msgs::PixelFormatType format = gz::msgs::RGB_INT8)
{
  gz::msgs::Image img;
  img.set_width(2);
  img.set_height(2);
  img.set_step(6);
  img.set_pixel_format_type(format);
  const unsigned char bytes[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
  img.set_data(bytes, sizeof(bytes));
  return img;
}
class CameraInput : public ::testing::Test
{
protected:
  UnderwaterCamera camera;
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  void SetUp() override
  {
    auto world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    ecm.CreateComponent(world, gz::sim::components::Name("adversarial"));
    sdf::Camera config;
    config.SetImageWidth(2);
    config.SetImageHeight(2);
    config.SetNearClip(0.1);
    config.SetFarClip(100);
    config.SetLensIntrinsicsFx(2);
    config.SetLensIntrinsicsFy(2);
    config.SetLensIntrinsicsCx(1);
    config.SetLensIntrinsicsCy(1);
    sdf::Sensor sensor;
    sensor.SetName("adversarial_camera");
    sensor.SetType(sdf::SensorType::RGBD_CAMERA);
    sensor.SetTopic("/adversarial_camera");
    sensor.SetCameraSensor(config);
    auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::RgbdCamera(sensor));
    auto plugin = std::make_shared<sdf::Element>();
    plugin->SetName("plugin");
    camera.Configure(entity, plugin, ecm, events);
  }
  void TearDown() override
  {
    if (rclcpp::ok())
    {
      rclcpp::shutdown();
    }
  }
  gz::msgs::Image Depth(float value = 3)
  {
    gz::msgs::Image d;
    d.set_width(2);
    d.set_height(2);
    d.set_step(8);
    d.set_pixel_format_type(gz::msgs::R_FLOAT32);
    const float values[] = {value, value, value, value};
    d.set_data(values, sizeof(values));
    return d;
  }
};
}  // namespace
TEST(CameraConversion, RgbDoesNotModifySource)
{
  UnderwaterCamera c;
  const auto img = RGB();
  const auto before = img.SerializeAsString();
  auto converted = c.ConvertGazeboToOpenCV(img);
  EXPECT_EQ(before, img.SerializeAsString());
  EXPECT_EQ(converted.at<cv::Vec3b>(0, 0), cv::Vec3b(30, 20, 10));
}
TEST(CameraConversion, RgbaDoesNotModifySource)
{
  UnderwaterCamera c;
  auto img = RGB(gz::msgs::RGBA_INT8);
  img.set_step(8);
  img.set_data(std::string("\x10\x20\x30\xff", 4) + std::string(12, '\x40'));
  const auto before = img.SerializeAsString();
  const auto converted = c.ConvertGazeboToOpenCV(img);
  EXPECT_EQ(before, img.SerializeAsString());
  EXPECT_EQ(converted.at<cv::Vec4b>(0, 0), cv::Vec4b(0x30, 0x20, 0x10, 255));
}
TEST(CameraConversion, InvalidFloatRowAlignmentIsRejected)
{
  UnderwaterCamera c;
  auto img = RGB(gz::msgs::R_FLOAT32);
  img.set_step(9);
  img.set_data(std::string(18, '\0'));
  EXPECT_NO_THROW(EXPECT_TRUE(c.ConvertGazeboToOpenCV(img).empty()));
}
TEST(CameraConversion, UnknownFormatIsRejected)
{
  UnderwaterCamera c;
  auto img = RGB(gz::msgs::UNKNOWN_PIXEL_FORMAT);
  EXPECT_NO_THROW(EXPECT_TRUE(c.ConvertGazeboToOpenCV(img).empty()));
}
TEST(CameraConversion, PaddedRgbRowsRemainImmutable)
{
  UnderwaterCamera c;
  auto img = RGB();
  img.set_step(8);
  img.set_data(
    std::string("\x10\x20\x30\x40\x50\x60\xee\xff", 8) +
    std::string("\x70\x80\x90\xa0\xb0\xc0\xdd\xee", 8));
  const auto before = img.SerializeAsString();
  auto mat = c.ConvertGazeboToOpenCV(img);
  EXPECT_EQ(before, img.SerializeAsString());
  EXPECT_EQ(mat.at<cv::Vec3b>(1, 0), cv::Vec3b(0x90, 0x80, 0x70));
}
TEST_F(CameraInput, MalformedTransportInputDoesNotThrow)
{
  camera.DepthImageCallback(Depth());
  camera.CameraCallback(RGB());
  EXPECT_NO_THROW(camera.CameraCallback(RGB(gz::msgs::UNKNOWN_PIXEL_FORMAT)));
  auto bad = Depth();
  bad.set_step(9);
  bad.set_data(std::string(18, '\0'));
  camera.DepthImageCallback(bad);
  EXPECT_NO_THROW(camera.CameraCallback(RGB()));
}
TEST_F(CameraInput, NonfiniteDepthUsesDefinedClipping)
{
  unsetenv("POSIM_UNDERWATER_PLUGIN");
  cv::Mat rgb(2, 2, CV_8UC3, cv::Scalar(90, 120, 150));
  cv::Mat depth(2, 2, CV_32FC1, cv::Scalar(3));
  cv::Mat out(2, 2, CV_8UC3);
  depth.at<float>(1, 1) = std::numeric_limits<float>::quiet_NaN();
  camera.SimulateUnderwater(rgb, depth, out);
  for (int c = 0; c < 3; ++c)
  {
    EXPECT_NEAR(out.at<cv::Vec3b>(1, 1)[c], (90 + c * 30) * std::exp(-100.0 / 30.0), 1.0);
  }
  depth.at<float>(1, 1) = std::numeric_limits<float>::infinity();
  EXPECT_NO_THROW(camera.SimulateUnderwater(rgb, depth, out));
  depth.at<float>(1, 1) = -std::numeric_limits<float>::infinity();
  camera.SimulateUnderwater(rgb, depth, out);
  EXPECT_NEAR(out.at<cv::Vec3b>(1, 1)[0], 90 * std::exp(-0.1 / 30.0), 1.0);
}
TEST_F(CameraInput, PassthroughDoesNotApplyOpticsTwice)
{
  setenv("POSIM_UNDERWATER_PLUGIN", "1", 1);
  cv::Mat rgb(2, 2, CV_8UC3, cv::Scalar(90, 120, 150));
  cv::Mat depth(2, 2, CV_32FC1, cv::Scalar(30));
  cv::Mat out(2, 2, CV_8UC3);
  camera.SimulateUnderwater(rgb, depth, out);
  EXPECT_EQ(cv::norm(rgb, out, cv::NORM_INF), 0);
  unsetenv("POSIM_UNDERWATER_PLUGIN");
}
