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
#include <gz/sim/components/RgbdCamera.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/sim/components/World.hh>
#include <limits>
#include <sdf/Camera.hh>
#include <sdf/Root.hh>
#include <sdf/Sensor.hh>
#include <thread>
#include "posim_gz_sensor_plugins/UnderwaterCamera.hh"

class CameraConfiguration : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
    }
    unsetenv("POSIM_UNDERWATER_PLUGIN");
    static unsigned serial = 0;
    const std::string name = "camera_config_" + std::to_string(++serial);
    topic = "/" + name;
    observer = std::make_shared<rclcpp::Node>(name + "_observer");
    world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    ecm.CreateComponent(world, gz::sim::components::Name("camera_configuration"));
    const auto model = ecm.CreateEntity();
    ecm.CreateComponent(model, gz::sim::components::Model());
    ecm.CreateComponent(model, gz::sim::components::Name("vehicle"));
    Parent(model, world);
    const auto link = ecm.CreateEntity();
    ecm.CreateComponent(link, gz::sim::components::Link());
    ecm.CreateComponent(link, gz::sim::components::Name("body"));
    Parent(link, model);
    entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::Sensor());
    ecm.CreateComponent(entity, gz::sim::components::Name(name));
    Parent(entity, link);
    sensor.SetName(name);
    sensor.SetType(sdf::SensorType::RGBD_CAMERA);
    sensor.SetTopic(topic);
    config.SetImageWidth(2);
    config.SetImageHeight(2);
    config.SetNearClip(0.1);
    config.SetFarClip(100);
    config.SetLensIntrinsicsFx(2);
    config.SetLensIntrinsicsFy(2);
    config.SetLensIntrinsicsCx(1);
    config.SetLensIntrinsicsCy(1);
  }

  void Parent(gz::sim::Entity child, gz::sim::Entity parent)
  {
    ecm.CreateComponent(child, gz::sim::components::ParentEntity(parent));
    ecm.SetParentEntity(child, parent);
  }

  void Configure(const std::string & parameters = "", bool nullSdf = false)
  {
    sensor.SetCameraSensor(config);
    ecm.CreateComponent(entity, gz::sim::components::RgbdCamera(sensor));
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>" + parameters +
      "</plugin></world></sdf>");
    ASSERT_TRUE(errors.empty());
    camera.Configure(
      entity, nullSdf ? nullptr : root.Element()->GetElement("world")->GetElement("plugin"), ecm,
      events);
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

  bool HasPublisher()
  {
    return Wait([&]() { return observer->count_publishers(topic + "/simulated_image") > 0; });
  }

  void ExpectRejected(const std::string & parameters = "", bool nullSdf = false)
  {
    ASSERT_NO_THROW(Configure(parameters, nullSdf));
    EXPECT_FALSE(Wait(
      [&]() { return observer->count_publishers(topic + "/simulated_image") > 0; },
      std::chrono::milliseconds(300)));
    EXPECT_NO_THROW(camera.CameraCallback(gz::msgs::Image()));
    gz::sim::UpdateInfo info;
    EXPECT_NO_THROW(camera.PostUpdate(info, ecm));
  }

  void ExpectIntensity(int row, int column, double range)
  {
    ASSERT_TRUE(HasPublisher());
    cv::Mat rgb(2, 2, CV_8UC3, cv::Scalar(100, 100, 100));
    cv::Mat depth(2, 2, CV_32FC1, cv::Scalar(30));
    cv::Mat output(2, 2, CV_8UC3);
    camera.SimulateUnderwater(rgb, depth, output);
    for (int channel = 0; channel < 3; ++channel)
    {
      EXPECT_NEAR(output.at<cv::Vec3b>(row, column)[channel], 100 * std::exp(-range / 30), 1);
    }
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity world;
  gz::sim::Entity entity;
  sdf::Sensor sensor;
  sdf::Camera config;
  std::string topic;
  std::shared_ptr<rclcpp::Node> observer;
  posim_gz_sensor_plugins::UnderwaterCamera camera;
};

TEST_F(CameraConfiguration, FieldOfViewDeterminesDefaultIntrinsics)
{
  config = sdf::Camera();
  config.SetImageWidth(2);
  config.SetImageHeight(2);
  config.SetHorizontalFov(gz::math::Angle(GZ_PI_2));
  ASSERT_FALSE(config.HasLensIntrinsics());
  Configure();
  ExpectIntensity(1, 1, 30);
  ExpectIntensity(0, 0, 30 * std::sqrt(3));
}

TEST_F(CameraConfiguration, ExplicitZeroPrincipalPointIsPreserved)
{
  config.SetLensIntrinsicsCx(0);
  config.SetLensIntrinsicsCy(0);
  Configure();
  ExpectIntensity(0, 0, 30);
}

TEST_F(CameraConfiguration, IntrinsicSkewChangesTheRay)
{
  config.SetLensIntrinsicsCx(0);
  config.SetLensIntrinsicsCy(0);
  config.SetLensIntrinsicsSkew(2);
  Configure();
  ExpectIntensity(1, 1, 30 * std::sqrt(1.25));
}

TEST_F(CameraConfiguration, ProjectionOverridesIntrinsics)
{
  config.SetLensProjectionFx(1);
  config.SetLensProjectionFy(1);
  config.SetLensProjectionCx(0);
  config.SetLensProjectionCy(0);
  Configure();
  ExpectIntensity(1, 1, 30 * std::sqrt(3));
}

TEST_F(CameraConfiguration, ExplicitTopicSupportsSdfSensorNames)
{
  sensor.SetName("camera-left.1");
  ASSERT_NO_THROW(Configure());
  EXPECT_TRUE(HasPublisher());
}

TEST_F(CameraConfiguration, ExplicitTopicDoesNotRequireWorldName)
{
  ecm.RemoveComponent<gz::sim::components::Name>(world);
  ASSERT_NO_THROW(Configure());
  EXPECT_TRUE(HasPublisher());
}

TEST_F(CameraConfiguration, MissingTopicUsesGazeboScope)
{
  sensor.SetTopic("");
  topic = "/world/camera_configuration/model/vehicle/link/body/sensor/" + sensor.Name();
  ASSERT_NO_THROW(Configure());
  EXPECT_TRUE(HasPublisher());
}

TEST_F(CameraConfiguration, InvalidRosTopicIsRejected)
{
  sensor.SetTopic("/invalid-topic");
  ExpectRejected();
}

TEST_F(CameraConfiguration, MissingPluginSdfIsRejected) { ExpectRejected("", true); }

TEST_F(CameraConfiguration, OversizedImageIsRejectedBeforeAllocation)
{
  config.SetImageWidth(65536);
  config.SetImageHeight(65536);
  ExpectRejected();
}

TEST_F(CameraConfiguration, NegativeNearClipIsRejected)
{
  config.SetNearClip(-0.1);
  ExpectRejected();
}

TEST_F(CameraConfiguration, ReversedClippingIntervalIsRejected)
{
  config.SetNearClip(10);
  config.SetFarClip(1);
  ExpectRejected();
}

TEST_F(CameraConfiguration, NonfiniteClipIsRejected)
{
  config.SetFarClip(std::numeric_limits<double>::infinity());
  ExpectRejected();
}

TEST_F(CameraConfiguration, NonfiniteFocalLengthIsRejected)
{
  config.SetLensIntrinsicsFx(std::numeric_limits<double>::quiet_NaN());
  ExpectRejected();
}

TEST_F(CameraConfiguration, NonpositiveFocalLengthIsRejected)
{
  config.SetLensIntrinsicsFy(-1);
  ExpectRejected();
}

TEST_F(CameraConfiguration, NonfinitePrincipalPointIsRejected)
{
  config.SetLensIntrinsicsCy(std::numeric_limits<double>::infinity());
  ExpectRejected();
}

TEST_F(CameraConfiguration, InvalidFieldOfViewIsRejected)
{
  config = sdf::Camera();
  config.SetImageWidth(2);
  config.SetImageHeight(2);
  config.SetHorizontalFov(gz::math::Angle(std::numeric_limits<double>::quiet_NaN()));
  ExpectRejected();
}

class InvalidCameraOptics : public CameraConfiguration,
                            public ::testing::WithParamInterface<std::string>
{
};

TEST_P(InvalidCameraOptics, RejectsBeforePublishing) { ExpectRejected(GetParam()); }

INSTANTIATE_TEST_SUITE_P(
  Parameters, InvalidCameraOptics,
  ::testing::Values(
    "<attenuationR>-1</attenuationR>", "<attenuationG>nan</attenuationG>",
    "<attenuationB>inf</attenuationB>", "<attenuationR>invalid</attenuationR>",
    "<backgroundR>-1</backgroundR>", "<backgroundG>256</backgroundG>",
    "<backgroundB>invalid</backgroundB>"));
