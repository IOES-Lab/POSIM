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
#include <gz/sim/components/World.hh>
#include <limits>
#include <sdf/Root.hh>
#include "posim_ros_gz_plugins/OceanCurrentPlugin.hh"

using posim_gz_world_plugins::OceanCurrentWorldPlugin;
using posim_ros_gz_plugins::OceanCurrentPlugin;

class CurrentServices : public ::testing::Test
{
protected:
  void SetUp() override
  {
    const auto entity = ecm.CreateEntity();
    ecm.CreateComponent(entity, gz::sim::components::World());
    sdf::Root root;
    ASSERT_TRUE(
      root
        .LoadSdfString("<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
                       "<namespace>current_service_test</namespace><constant_current/>"
                       "<transient_current/></plugin></world></sdf>")
        .empty());
    world.Configure(entity, root.Element()->GetElement("world")->GetElement("plugin"), ecm, events);
    ASSERT_EQ(OceanCurrentWorldPlugin::Instance(), &world);
    auto & data = *world.sharedDataPtr;
    ASSERT_TRUE(data.currentVelModel.SetModel(0.3, 0.0, 1.0, 0.0, 0.0));
    ASSERT_TRUE(data.currentHorzAngleModel.SetModel(0.0, -M_PI, M_PI, 0.0, 0.0));
    ASSERT_TRUE(data.currentVertAngleModel.SetModel(0.0, -M_PI / 2, M_PI / 2, 0.0, 0.0));
    data.stratifiedDatabase = {{0.0, 0.05, 0.0}, {0.0, 0.01, 10.0}};
    data.stratifiedCurrentModels.resize(2);
    for (size_t layer = 0; layer < 2; ++layer)
    {
      auto & models = data.stratifiedCurrentModels[layer];
      models = {data.currentVelModel, data.currentHorzAngleModel, data.currentVertAngleModel};
      ASSERT_TRUE(models[0].SetMean(data.stratifiedDatabase[layer].Y()));
      ASSERT_TRUE(models[1].SetMean(M_PI / 2));
    }
  }

  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  OceanCurrentWorldPlugin world;
  OceanCurrentPlugin services;
};

TEST_F(CurrentServices, InvalidVelocityRequestPreservesEveryComponent)
{
  using Service = posim_interfaces::srv::SetCurrentVelocity;
  auto request = std::make_shared<Service::Request>();
  auto response = std::make_shared<Service::Response>();
  request->velocity = 0.6;
  request->horizontal_angle = 0.5;
  request->vertical_angle = std::numeric_limits<double>::quiet_NaN();
  services.UpdateCurrentVelocity(request, response);
  EXPECT_FALSE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVelModel.mean, 0.3);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVelModel.var, 0.3);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentHorzAngleModel.mean, 0.0);
  request->vertical_angle = 0.1;
  services.UpdateCurrentVelocity(request, response);
  EXPECT_TRUE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVelModel.mean, 0.6);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentHorzAngleModel.mean, 0.5);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVertAngleModel.mean, 0.1);
}

TEST_F(CurrentServices, InvalidLayerRequestPreservesModelAndDatabase)
{
  using Service = posim_interfaces::srv::SetStratifiedCurrentVelocity;
  auto request = std::make_shared<Service::Request>();
  auto response = std::make_shared<Service::Response>();
  const auto original = world.sharedDataPtr->stratifiedDatabase[0];
  request->layer = 0;
  request->velocity = 0.6;
  request->horizontal_angle = 0.5;
  request->vertical_angle = 5.0;
  services.UpdateStratCurrentVelocity(request, response);
  EXPECT_FALSE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->stratifiedCurrentModels[0][0].mean, 0.05);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->stratifiedCurrentModels[0][1].mean, M_PI / 2);
  EXPECT_EQ(world.sharedDataPtr->stratifiedDatabase[0], original);
  request->vertical_angle = 0.0;
  services.UpdateStratCurrentVelocity(request, response);
  EXPECT_TRUE(response->success);
  EXPECT_NEAR(world.sharedDataPtr->stratifiedDatabase[0].X(), 0.6 * std::cos(0.5), 1e-12);
  request->layer = 100;
  services.UpdateStratCurrentVelocity(request, response);
  EXPECT_FALSE(response->success);
}

TEST_F(CurrentServices, VelocityBoundsApplyToAllLayersOrLeaveEverythingUnchanged)
{
  using Service = posim_interfaces::srv::SetCurrentModel;
  auto request = std::make_shared<Service::Request>();
  auto response = std::make_shared<Service::Response>();
  request->mean = 0.02;
  request->min = 0.0;
  request->max = 0.03;
  services.UpdateCurrentVelocityModel(request, response);
  EXPECT_FALSE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVelModel.mean, 0.3);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->stratifiedCurrentModels[1][0].max, 1.0);
  request->mean = std::numeric_limits<double>::quiet_NaN();
  request->max = 0.8;
  services.UpdateCurrentVelocityModel(request, response);
  EXPECT_FALSE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->stratifiedCurrentModels[0][0].max, 1.0);
  request->mean = 0.3;
  request->min = 0.005;
  services.UpdateCurrentVelocityModel(request, response);
  EXPECT_TRUE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentVelModel.min, 0.005);
  for (const auto & layer : world.sharedDataPtr->stratifiedCurrentModels)
  {
    EXPECT_DOUBLE_EQ(layer[0].min, 0.005);
    EXPECT_DOUBLE_EQ(layer[0].max, 0.8);
  }
}

TEST_F(CurrentServices, AngleModelUpdatesValidateAllAffectedLayers)
{
  using Service = posim_interfaces::srv::SetCurrentModel;
  auto request = std::make_shared<Service::Request>();
  auto response = std::make_shared<Service::Response>();
  request->mean = 0.0;
  request->min = -0.1;
  request->max = 0.1;
  request->mu = 0.2;
  services.UpdateCurrentHorzAngleModel(request, response);
  EXPECT_FALSE(response->success);
  EXPECT_DOUBLE_EQ(world.sharedDataPtr->currentHorzAngleModel.mu, 0.0);
  request->min = -M_PI;
  request->max = M_PI;
  services.UpdateCurrentHorzAngleModel(request, response);
  EXPECT_TRUE(response->success);
  for (const auto & layer : world.sharedDataPtr->stratifiedCurrentModels)
  {
    EXPECT_DOUBLE_EQ(layer[1].mu, 0.2);
  }
  request->min = -1.0;
  request->max = 1.0;
  services.UpdateCurrentVertAngleModel(request, response);
  EXPECT_TRUE(response->success);
  for (const auto & layer : world.sharedDataPtr->stratifiedCurrentModels)
  {
    EXPECT_DOUBLE_EQ(layer[2].mu, 0.2);
    EXPECT_DOUBLE_EQ(layer[2].mean, 0.0);
  }
}
