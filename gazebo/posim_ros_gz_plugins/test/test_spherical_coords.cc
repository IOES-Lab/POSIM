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
#include <cmath>
#include <gz/sim/components/SphericalCoordinates.hh>
#include <gz/sim/components/World.hh>
#include <limits>
#include "posim_ros_gz_plugins/SphericalCoords.hh"

using posim_interfaces::srv::GetOriginSphericalCoord;
using posim_interfaces::srv::SetOriginSphericalCoord;
using posim_interfaces::srv::TransformFromSphericalCoord;
using posim_interfaces::srv::TransformToSphericalCoord;
using posim_ros_gz_plugins::SphericalCoords;

TEST(SphericalCoordinates, UnconfiguredCallbacksFailWithoutDereferencingWorld)
{
  SphericalCoords plugin;
  auto origin = std::make_shared<GetOriginSphericalCoord::Response>();
  EXPECT_FALSE(
    plugin.GetOriginSphericalCoord(std::make_shared<GetOriginSphericalCoord::Request>(), origin));
  EXPECT_TRUE(std::isnan(origin->latitude_deg));
  auto from = std::make_shared<TransformFromSphericalCoord::Response>();
  EXPECT_FALSE(plugin.TransformFromSphericalCoord(
    std::make_shared<TransformFromSphericalCoord::Request>(), from));
  EXPECT_TRUE(std::isnan(from->output.x));
  auto to = std::make_shared<TransformToSphericalCoord::Response>();
  EXPECT_FALSE(
    plugin.TransformToSphericalCoord(std::make_shared<TransformToSphericalCoord::Request>(), to));
  EXPECT_TRUE(std::isnan(to->altitude));
  auto set = std::make_shared<SetOriginSphericalCoord::Response>();
  EXPECT_FALSE(
    plugin.SetOriginSphericalCoord(std::make_shared<SetOriginSphericalCoord::Request>(), set));
  EXPECT_FALSE(set->success);
  gz::sim::EntityComponentManager ecm;
  plugin.PostUpdate(gz::sim::UpdateInfo(), ecm);
}

TEST(SphericalCoordinates, MissingWorldOrOriginLeavesPluginDisabled)
{
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  const auto entity = ecm.CreateEntity();
  for (const bool isWorld : {false, true})
  {
    if (isWorld)
    {
      ecm.CreateComponent(entity, gz::sim::components::World());
    }
    SphericalCoords plugin;
    plugin.Configure(entity, nullptr, ecm, events);
    auto response = std::make_shared<GetOriginSphericalCoord::Response>();
    EXPECT_FALSE(plugin.GetOriginSphericalCoord(
      std::make_shared<GetOriginSphericalCoord::Request>(), response));
    EXPECT_TRUE(std::isnan(response->latitude_deg));
    plugin.PostUpdate(gz::sim::UpdateInfo(), ecm);
  }
}

class ConfiguredCoordinates : public ::testing::Test
{
protected:
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  gz::sim::Entity world;
  SphericalCoords plugin;

  void SetUp() override
  {
    world = ecm.CreateEntity();
    ecm.CreateComponent(world, gz::sim::components::World());
    gz::math::SphericalCoordinates origin;
    gz::math::Angle latitude, longitude, heading;
    latitude.SetDegree(35.074823);
    longitude.SetDegree(129.084798);
    heading.SetDegree(20.0);
    origin.SetLatitudeReference(latitude);
    origin.SetLongitudeReference(longitude);
    origin.SetElevationReference(12.0);
    origin.SetHeadingOffset(heading);
    ecm.CreateComponent(world, gz::sim::components::SphericalCoordinates(origin));
    plugin.Configure(world, nullptr, ecm, events);
  }
};

TEST_F(ConfiguredCoordinates, LocalGeographicRoundTripRespectsWorldOriginAndHeading)
{
  auto toRequest = std::make_shared<TransformToSphericalCoord::Request>();
  toRequest->input.x = 100.0;
  toRequest->input.y = -200.0;
  toRequest->input.z = 3.0;
  auto geographic = std::make_shared<TransformToSphericalCoord::Response>();
  ASSERT_TRUE(plugin.TransformToSphericalCoord(toRequest, geographic));
  auto fromRequest = std::make_shared<TransformFromSphericalCoord::Request>();
  fromRequest->latitude_deg = geographic->latitude_deg;
  fromRequest->longitude_deg = geographic->longitude_deg;
  fromRequest->altitude = geographic->altitude;
  auto local = std::make_shared<TransformFromSphericalCoord::Response>();
  ASSERT_TRUE(plugin.TransformFromSphericalCoord(fromRequest, local));
  EXPECT_NEAR(local->output.x, toRequest->input.x, 0.001);
  EXPECT_NEAR(local->output.y, toRequest->input.y, 0.001);
  EXPECT_NEAR(local->output.z, toRequest->input.z, 0.001);
}

TEST_F(ConfiguredCoordinates, UpdatesOriginAndRejectsInvalidRequestsWithoutChangingIt)
{
  auto setRequest = std::make_shared<SetOriginSphericalCoord::Request>();
  setRequest->latitude_deg = -35.0;
  setRequest->longitude_deg = 170.0;
  setRequest->altitude = 100.0;
  auto setResponse = std::make_shared<SetOriginSphericalCoord::Response>();
  ASSERT_TRUE(plugin.SetOriginSphericalCoord(setRequest, setResponse));
  ASSERT_TRUE(setResponse->success);
  for (const double invalid :
       {91.0, -91.0, std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity()})
  {
    setRequest->latitude_deg = invalid;
    EXPECT_FALSE(plugin.SetOriginSphericalCoord(setRequest, setResponse));
    EXPECT_FALSE(setResponse->success);
  }
  setRequest->latitude_deg = 35.0;
  setRequest->longitude_deg = 181.0;
  EXPECT_FALSE(plugin.SetOriginSphericalCoord(setRequest, setResponse));
  setRequest->longitude_deg = 129.0;
  setRequest->altitude = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(plugin.SetOriginSphericalCoord(setRequest, setResponse));
  auto origin = std::make_shared<GetOriginSphericalCoord::Response>();
  ASSERT_TRUE(
    plugin.GetOriginSphericalCoord(std::make_shared<GetOriginSphericalCoord::Request>(), origin));
  EXPECT_DOUBLE_EQ(origin->latitude_deg, -35.0);
  EXPECT_DOUBLE_EQ(origin->longitude_deg, 170.0);
  EXPECT_DOUBLE_EQ(origin->altitude, 100.0);
  auto toRequest = std::make_shared<TransformToSphericalCoord::Request>();
  toRequest->input.x = std::numeric_limits<double>::quiet_NaN();
  auto to = std::make_shared<TransformToSphericalCoord::Response>();
  EXPECT_FALSE(plugin.TransformToSphericalCoord(toRequest, to));
  EXPECT_TRUE(std::isnan(to->latitude_deg));
  auto fromRequest = std::make_shared<TransformFromSphericalCoord::Request>();
  fromRequest->longitude_deg = -181.0;
  auto from = std::make_shared<TransformFromSphericalCoord::Response>();
  EXPECT_FALSE(plugin.TransformFromSphericalCoord(fromRequest, from));
  EXPECT_TRUE(std::isnan(from->output.z));
}

TEST_F(ConfiguredCoordinates, RemovedOriginFailsSafely)
{
  ecm.RemoveComponent<gz::sim::components::SphericalCoordinates>(world);
  auto origin = std::make_shared<GetOriginSphericalCoord::Response>();
  EXPECT_FALSE(
    plugin.GetOriginSphericalCoord(std::make_shared<GetOriginSphericalCoord::Request>(), origin));
  EXPECT_TRUE(std::isnan(origin->altitude));
}
