// Copyright (c) 2016 The dave Simulator Authors.
// All rights reserved.
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

#ifndef POSIM_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_
#define POSIM_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_

#include "posim_gz_world_plugins/gauss_markov_process.hh"
#include "posim_gz_world_plugins/tidal_oscillation.hh"
#include "posim_interfaces/msg/stratified_current_database.hpp"
#include "posim_interfaces/msg/stratified_current_velocity.hpp"

// OceanCurrentModelPlugin

#include <gz/physics/World.hh>
#include <gz/sim/System.hh>

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <rclcpp/rclcpp.hpp>

#include <gz/transport/Node.hh>
#include <sdf/sdf.hh>

namespace posim_gz_model_plugins
{
class OceanCurrentModelPlugin : public gz::sim::System,
                                public gz::sim::ISystemConfigure,
                                public gz::sim::ISystemUpdate,
                                public gz::sim::ISystemPostUpdate
{
public:
  OceanCurrentModelPlugin();
  ~OceanCurrentModelPlugin() override;

  // ----------------------------------------------

  void Configure(
    const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
    gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr);

  bool LoadCurrentVelocityParams(sdf::ElementPtr _sdf, gz::sim::EntityComponentManager & _ecm);

  void UpdateDatabase(
    const std::shared_ptr<const posim_interfaces::msg::StratifiedCurrentDatabase> & _msg);

  gz::sim::Entity GetModelEntity(
    const std::string & modelName, gz::sim::EntityComponentManager & ecm);

  gz::math::Pose3d GetModelPose(
    const gz::sim::Entity & modelEntity, gz::sim::EntityComponentManager & ecm);

  // ----------------------------------------------

  void Update(const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm);

  void CalculateOceanCurrent(double vehicleDepth);

  // ----------------------------------------------

  // Function called after the simulation state updates
  void PostUpdate(const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm);

  void PublishCurrentVelocity(const gz::sim::UpdateInfo & _info);

private:
  std::shared_ptr<rclcpp::Node> ros_node_;
  struct PrivateData;
  std::unique_ptr<PrivateData> dataPtr;
};
}  // namespace posim_gz_model_plugins

#endif  // POSIM_GZ_MODEL_PLUGINS__OCEANCURRENTMODELPLUGIN_HH_
