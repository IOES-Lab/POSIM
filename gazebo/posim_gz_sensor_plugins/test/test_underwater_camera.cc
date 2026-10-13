#include <gtest/gtest.h>
#include <csignal>
#include <cstring>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/EventManager.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/RgbdCamera.hh>
#include <gz/sim/components/World.hh>
#include <sdf/Camera.hh>
#include <sdf/Sensor.hh>

#include "posim_gz_sensor_plugins/UnderwaterCamera.hh"

using posim_gz_sensor_plugins::UnderwaterCamera;

TEST(UnderwaterCamera, UnconfiguredUpdateAndDestruction)
{
  UnderwaterCamera camera;
  gz::sim::EntityComponentManager ecm;
  gz::sim::UpdateInfo info;
  camera.PostUpdate(info, ecm);
}

TEST(UnderwaterCamera, PaddedDepthRows)
{
  UnderwaterCamera camera;
  gz::msgs::Image image;
  image.set_width(2);
  image.set_height(2);
  image.set_step(12);
  image.set_pixel_format_type(gz::msgs::PixelFormatType::R_FLOAT32);
  const float data[] = {1, 2, 99, 3, 4, 99};
  image.set_data(data, sizeof(data));
  const auto mat = camera.ConvertGazeboToOpenCV(image);
  ASSERT_FALSE(mat.empty());
  EXPECT_FLOAT_EQ(mat.at<float>(1, 0), 3);
  EXPECT_FLOAT_EQ(mat.at<float>(1, 1), 4);
}

TEST(UnderwaterCamera, TruncatedOrMissingImageIsRejected)
{
  UnderwaterCamera camera;
  gz::msgs::Image image;
  image.set_pixel_format_type(gz::msgs::PixelFormatType::RGB_INT8);
  EXPECT_TRUE(camera.ConvertGazeboToOpenCV(image).empty());
  image.set_width(2);
  image.set_height(2);
  image.set_step(6);
  image.set_data(std::string(11, '\0'));
  EXPECT_TRUE(camera.ConvertGazeboToOpenCV(image).empty());
  image.set_step(5);
  image.set_data(std::string(12, '\0'));
  EXPECT_TRUE(camera.ConvertGazeboToOpenCV(image).empty());
}

TEST(UnderwaterCamera, RgbToBgrIsUnchanged)
{
  UnderwaterCamera camera;
  gz::msgs::Image image;
  image.set_width(1);
  image.set_height(1);
  image.set_step(3);
  image.set_pixel_format_type(gz::msgs::PixelFormatType::RGB_INT8);
  const unsigned char data[] = {10, 20, 30};
  image.set_data(data, sizeof(data));
  const auto mat = camera.ConvertGazeboToOpenCV(image);
  ASSERT_FALSE(mat.empty());
  EXPECT_EQ(mat.at<cv::Vec3b>(0, 0), cv::Vec3b(30, 20, 10));
}

TEST(UnderwaterCamera, ConfigurePreservesSignalHandlerAndMissingDepthIsSafe)
{
  // A Gazebo plugin must not take ownership of the enclosing process's SIGINT.
  auto handler = +[](int) {};
  auto previous = std::signal(SIGINT, handler);
  {
    UnderwaterCamera camera;
    gz::sim::EntityComponentManager ecm;
    gz::sim::EventManager events;
    auto world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    ecm.CreateComponent(world, gz::sim::components::Name("camera_test"));
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
    sensor.SetName("test_camera");
    sensor.SetType(sdf::SensorType::RGBD_CAMERA);
    sensor.SetTopic("/camera_unit_test");
    sensor.SetCameraSensor(config);
    auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::RgbdCamera(sensor));
    auto plugin = std::make_shared<sdf::Element>();
    plugin->SetName("plugin");
    camera.Configure(entity, plugin, ecm, events);
    EXPECT_EQ(std::signal(SIGINT, handler), handler);

    gz::msgs::Image rgb;
    rgb.set_width(2);
    rgb.set_height(2);
    rgb.set_step(6);
    rgb.set_pixel_format_type(gz::msgs::PixelFormatType::RGB_INT8);
    rgb.set_data(std::string(12, '\x20'));
    camera.CameraCallback(rgb);
    camera.CameraCallback(rgb);  // No depth has arrived; must not index it.

    cv::Mat color(2, 2, CV_8UC3, cv::Scalar(90, 120, 150));
    cv::Mat depth(2, 2, CV_32FC1, cv::Scalar(3));
    cv::Mat output(2, 2, CV_8UC3);
    camera.SimulateUnderwater(color, depth, output);
    // At the principal point, range equals depth. Default attenuation is 1/30.
    for (int channel = 0; channel < 3; ++channel)
    {
      const auto expected = (90 + 30 * channel) * std::exp(-3.0 / 30.0);
      EXPECT_NEAR(output.at<cv::Vec3b>(1, 1)[channel], expected, 1.0);
    }

    // Explicit shutdown by another component must also be safe for PostUpdate.
    rclcpp::shutdown();
    gz::sim::UpdateInfo info;
    camera.PostUpdate(info, ecm);
  }
  std::signal(SIGINT, previous);
}
