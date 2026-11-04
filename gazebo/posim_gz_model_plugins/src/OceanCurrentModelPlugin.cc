// Copyright (c) 2016 The UUV Simulator Authors.
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

#include "posim_gz_model_plugins/OceanCurrentModelPlugin.hh"
#include "posim_gz_world_plugins/gauss_markov_process.hh"
#include "posim_gz_world_plugins/tidal_oscillation.hh"

// Gazebo includes
#include <gz/msgs/vector3d.pb.h>
#include <gz/plugin/Register.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>

// ROS 2 includes
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <rclcpp/rclcpp.hpp>

// Custom message interfaces
#include "posim_interfaces/msg/stratified_current_database.hpp"

// Standard library includes
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

GZ_ADD_PLUGIN(
  posim_gz_model_plugins::OceanCurrentModelPlugin, gz::sim::System,
  posim_gz_model_plugins::OceanCurrentModelPlugin::ISystemConfigure,
  posim_gz_model_plugins::OceanCurrentModelPlugin::ISystemUpdate,
  posim_gz_model_plugins::OceanCurrentModelPlugin::ISystemPostUpdate)

namespace posim_gz_model_plugins
{
struct OceanCurrentModelPlugin::PrivateData
{
  bool configured{false};
  bool sampled{false};
  // Gazebo Simulation Related Variables
  gz::sim::World world{gz::sim::kNullEntity};
  gz::sim::Entity entity{gz::sim::kNullEntity};
  gz::sim::Model model{gz::sim::kNullEntity};
  gz::sim::Entity modelLink{gz::sim::kNullEntity};
  gz::sim::Entity modelEntity;
  std::string modelName;

  // Transport and Communication
  std::shared_ptr<gz::transport::Node> gz_node;
  gz::transport::Node::Publisher gz_current_vel_pub;
  std::shared_ptr<rclcpp::Node> ros_node_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr flowVelocityPub;
  rclcpp::Subscription<posim_interfaces::msg::StratifiedCurrentDatabase>::SharedPtr databaseSub;

  // Topics
  std::string currentVelocityTopic;
  std::string transientCurrentVelocityTopic;

  // Time and Periods
  std::chrono::steady_clock::duration lastUpdate{0};
  std::chrono::steady_clock::duration rosPublishPeriod{0};
  std::chrono::steady_clock::duration time;

  // Lock for Synchronization
  std::mutex lock_;

  // Current Velocity
  gz::math::Vector3d currentVelocity;
  std::vector<gz::math::Vector3d> database;

  // Gauss-Markov Process Models
  posim_gz_world_plugins::GaussMarkovProcess currentVelNorthModel;
  posim_gz_world_plugins::GaussMarkovProcess currentVelEastModel;
  posim_gz_world_plugins::GaussMarkovProcess currentVelDownModel;

  // Noise Amplitudes and Frequencies
  double noiseAmp_North{0};
  double noiseAmp_East{0};
  double noiseAmp_Down{0};
  double noiseFreq_North;
  double noiseFreq_East;
  double noiseFreq_Down;

  // Tidal Oscillation Model
  posim_gz_world_plugins::TidalOscillation tide;

  bool tideFlag{false};
};

OceanCurrentModelPlugin::OceanCurrentModelPlugin() : dataPtr(std::make_unique<PrivateData>()) {}

OceanCurrentModelPlugin::~OceanCurrentModelPlugin() = default;

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  this->dataPtr->configured = false;
  if (
    !_sdf || !_sdf->HasElement("namespace") || !_sdf->HasElement("transient_current") ||
    _sdf->Get<std::string>("namespace").empty() || !gz::sim::Model(_entity).Valid(_ecm))
  {
    gzerr << "OceanCurrentModelPlugin requires a model, namespace and transient_current block."
          << std::endl;
    return;
  }
  // Check if ROS is initialized; if not, initialize it
  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  }

  // Initialize the ROS 2 node
  this->ros_node_ = std::make_shared<rclcpp::Node>("OceanCurrentModelPlugin");

  // Initialize the Gazebo transport node
  this->dataPtr->gz_node = std::make_shared<gz::transport::Node>();

  gzdbg << "posim_gz_model_plugins::OceanCurrentModelPlugin::Configure on entity: " << _entity
        << std::endl;

  // Clone the SDF element to allow calling non-const methods
  sdf::ElementPtr sdfClone = _sdf->Clone();

  // Get the world entity from the Entity Component Manager (ECM)
  auto worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
  this->dataPtr->world = gz::sim::World(worldEntity);

  // Get the model entity
  auto model = gz::sim::Model(_entity);
  this->dataPtr->model = model;
  gzmsg << "Loading ocean current model plugin..." << std::endl;

  // Read the namespace for topics and services
  if (!_sdf->HasElement("namespace"))
  {
    gzerr << "Missing required parameter <namespace>, plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->modelName = _sdf->Get<std::string>("namespace");

  // Set the flow velocity topic from SDF or use the default topic name

  ///  For Hydrodynamics Plugin -  <namespace> - This allows the robot to have an individual
  ///  namespace for current. This is useful when you have multiple vehicles in different
  ///  locations and you wish to set the currents of each vehicle separately. If no namespace is
  ///  given then the plugin listens on the `/ocean_current` topic for a `Vector3d` message.
  ///  Otherwise it listens on `/model/{namespace name}/ocean_current`.[String, Optional]

  if (_sdf->HasElement("flow_velocity_topic"))
  {
    this->dataPtr->currentVelocityTopic =
      "model/" + this->dataPtr->modelName + "/" + _sdf->Get<std::string>("flow_velocity_topic");
    gzwarn << "Setting flow_velocity_topic to anything other than 'ocean_current' is not "
              "recommended, as it might not be compatible with the hydrodynamics plugin."
           << std::endl;
  }
  else
  {
    this->dataPtr->currentVelocityTopic = "model/" + this->dataPtr->modelName + "/ocean_current";
  }
  gzmsg << "Transient velocity topic name for " << this->dataPtr->modelName << " : "
        << this->dataPtr->currentVelocityTopic << std::endl;

  // Get the model entity based on the model name
  this->dataPtr->modelEntity = _entity;

  // Read the stratified ocean current topic name from the SDF
  if (!LoadCurrentVelocityParams(sdfClone, _ecm))
  {
    gzerr << "Invalid model-current parameters; current publication is disabled." << std::endl;
    return;
  }

  try
  {
    // Advertise the ROS flow velocity as a stamped twist message
    this->dataPtr->flowVelocityPub =
      this->ros_node_->create_publisher<geometry_msgs::msg::TwistStamped>(
        this->dataPtr->currentVelocityTopic, rclcpp::QoS(10));

    // Advertise the current velocity topic in Gazebo
    this->dataPtr->gz_current_vel_pub =
      this->dataPtr->gz_node->Advertise<gz::msgs::Vector3d>(this->dataPtr->currentVelocityTopic);

    // Subscribe to the stratified ocean current database
    this->dataPtr->databaseSub =
      this->ros_node_->create_subscription<posim_interfaces::msg::StratifiedCurrentDatabase>(
        this->dataPtr->transientCurrentVelocityTopic, 10,
        std::bind(&OceanCurrentModelPlugin::UpdateDatabase, this, std::placeholders::_1));
  }
  catch (const rclcpp::exceptions::RCLError & error)
  {
    gzerr << "Unable to initialize model-current topics: " << error.what() << std::endl;
    return;
  }
  catch (const rclcpp::exceptions::InvalidTopicNameError & error)
  {
    gzerr << "Invalid model-current topic: " << error.what() << std::endl;
    return;
  }
  if (!this->dataPtr->gz_current_vel_pub)
  {
    return;
  }
  this->dataPtr->configured = true;

  gzmsg << "Transient current model plugin loaded!" << std::endl;
}

/////////////////////////////////////////////////
bool OceanCurrentModelPlugin::LoadCurrentVelocityParams(
  sdf::ElementPtr _sdf, gz::sim::EntityComponentManager & _ecm)
{
  // Read topic name of stratified ocean current from SDF
  sdf::ElementPtr currentVelocityParams;
  if (_sdf->HasElement("transient_current"))
  {
    currentVelocityParams = _sdf->GetElement("transient_current");
    for (const auto component : {"velocity_north", "velocity_east", "velocity_down"})
    {
      if (!currentVelocityParams->HasElement(component))
      {
        continue;
      }
      const auto element = currentVelocityParams->GetElement(component);
      for (const auto parameter : {"mean", "mu", "noiseAmp", "noiseFreq"})
      {
        double value;
        if (
          element->HasElement(parameter) &&
          (!element->GetElement(parameter)->GetValue()->Get(value) || !std::isfinite(value)))
        {
          return false;
        }
      }
    }
    if (currentVelocityParams->HasElement("topic_stratified"))
    {
      this->dataPtr->transientCurrentVelocityTopic =
        currentVelocityParams->Get<std::string>("topic_stratified");
    }
    else
    {
      this->dataPtr->transientCurrentVelocityTopic =
        "hydrodynamics/stratified_current_velocity_topic_database";
    }

    // initialize velocity_north_model parameters
    if (currentVelocityParams->HasElement("velocity_north"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_north");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelNorthModel.mean = elem->Get<double>("mean");
      }
      else
      {
        this->dataPtr->currentVelNorthModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelNorthModel.mu = elem->Get<double>("mu");
      }
      else
      {
        this->dataPtr->currentVelNorthModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_North = elem->Get<double>("noiseAmp");
      }
      else
      {
        this->dataPtr->noiseAmp_North = 0.0;
      }
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_North = elem->Get<double>("noiseFreq");
      }
      else
      {
        this->dataPtr->noiseFreq_North = 0.0;
      }
      this->dataPtr->currentVelNorthModel.min =
        this->dataPtr->currentVelNorthModel.mean - this->dataPtr->noiseAmp_North;
      this->dataPtr->currentVelNorthModel.max =
        this->dataPtr->currentVelNorthModel.mean + this->dataPtr->noiseAmp_North;
      this->dataPtr->currentVelNorthModel.noiseAmp = this->dataPtr->noiseFreq_North;
    }

    this->dataPtr->currentVelNorthModel.var = this->dataPtr->currentVelNorthModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current north-direction velocity [m/s] "
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelNorthModel.Print();

    // initialize velocity_east_model parameters
    if (currentVelocityParams->HasElement("velocity_east"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_east");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelEastModel.mean = elem->Get<double>("mean");
      }
      else
      {
        this->dataPtr->currentVelEastModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelEastModel.mu = elem->Get<double>("mu");
      }
      else
      {
        this->dataPtr->currentVelEastModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_East = elem->Get<double>("noiseAmp");
      }
      else
      {
        this->dataPtr->noiseAmp_East = 0.0;
      }
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_East = elem->Get<double>("noiseFreq");
      }
      else
      {
        this->dataPtr->noiseFreq_East = 0.0;
      }
      this->dataPtr->currentVelEastModel.min =
        this->dataPtr->currentVelEastModel.mean - this->dataPtr->noiseAmp_East;
      this->dataPtr->currentVelEastModel.max =
        this->dataPtr->currentVelEastModel.mean + this->dataPtr->noiseAmp_East;
      this->dataPtr->currentVelEastModel.noiseAmp = this->dataPtr->noiseFreq_East;
    }

    this->dataPtr->currentVelEastModel.var = this->dataPtr->currentVelEastModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current east-direction velocity [m/s] "
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelEastModel.Print();

    // initialize velocity_down_model parameters
    if (currentVelocityParams->HasElement("velocity_down"))
    {
      sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity_down");
      if (elem->HasElement("mean"))
      {
        this->dataPtr->currentVelDownModel.mean = elem->Get<double>("mean");
      }
      else
      {
        this->dataPtr->currentVelDownModel.mean = 0.0;
      }
      if (elem->HasElement("mu"))
      {
        this->dataPtr->currentVelDownModel.mu = elem->Get<double>("mu");
      }
      else
      {
        this->dataPtr->currentVelDownModel.mu = 0.0;
      }
      if (elem->HasElement("noiseAmp"))
      {
        this->dataPtr->noiseAmp_Down = elem->Get<double>("noiseAmp");
      }
      else
      {
        this->dataPtr->noiseAmp_Down = 0.0;
      }
      if (elem->HasElement("noiseFreq"))
      {
        this->dataPtr->noiseFreq_Down = elem->Get<double>("noiseFreq");
      }
      else
      {
        this->dataPtr->noiseFreq_Down = 0.0;
      }
      this->dataPtr->currentVelDownModel.min =
        this->dataPtr->currentVelDownModel.mean - this->dataPtr->noiseAmp_Down;
      this->dataPtr->currentVelDownModel.max =
        this->dataPtr->currentVelDownModel.mean + this->dataPtr->noiseAmp_Down;
      this->dataPtr->currentVelDownModel.noiseAmp = this->dataPtr->noiseFreq_Down;
    }

    this->dataPtr->currentVelDownModel.var = this->dataPtr->currentVelDownModel.mean;
    gzmsg << "For vehicle " << this->dataPtr->modelName
          << " -> Current down-direction velocity [m/s]"
          << "Gauss-Markov process model:" << std::endl;
    this->dataPtr->currentVelDownModel.Print();

    this->dataPtr->currentVelNorthModel.lastUpdate = this->dataPtr->lastUpdate.count();
    this->dataPtr->currentVelEastModel.lastUpdate = this->dataPtr->lastUpdate.count();
    this->dataPtr->currentVelDownModel.lastUpdate = this->dataPtr->lastUpdate.count();

    // Tidal Oscillation
    if (_sdf->HasElement("tide_oscillation"))
    {
      this->dataPtr->tideFlag = _sdf->Get<bool>("tide_oscillation");
    }
    else
    {
      this->dataPtr->tideFlag = false;
    }
  }
  for (const auto model :
       {&this->dataPtr->currentVelNorthModel, &this->dataPtr->currentVelEastModel,
        &this->dataPtr->currentVelDownModel})
  {
    if (
      !std::isfinite(model->mean) || !std::isfinite(model->min) || !std::isfinite(model->max) ||
      model->min > model->max || !std::isfinite(model->mu) || model->mu < 0 || model->mu > 1 ||
      !std::isfinite(model->noiseAmp) || model->noiseAmp < 0)
    {
      return false;
    }
  }
  return !this->dataPtr->transientCurrentVelocityTopic.empty();
}

void OceanCurrentModelPlugin::UpdateDatabase(
  const std::shared_ptr<const posim_interfaces::msg::StratifiedCurrentDatabase> & _msg)
{
  if (
    !this->dataPtr->configured || !_msg || _msg->depths.size() < 2 ||
    _msg->depths.size() != _msg->velocities.size())
  {
    gzerr << "Ignoring incomplete model-current database." << std::endl;
    return;
  }
  std::vector<gz::math::Vector3d> database;
  for (size_t i = 0; i < _msg->depths.size(); ++i)
  {
    const auto & velocity = _msg->velocities[i];
    if (
      !std::isfinite(_msg->depths[i]) || _msg->depths[i] < 0 ||
      (i && _msg->depths[i] <= _msg->depths[i - 1]) || !std::isfinite(velocity.x) ||
      !std::isfinite(velocity.y) || !std::isfinite(velocity.z))
    {
      gzerr << "Ignoring invalid model-current database." << std::endl;
      return;
    }
    database.emplace_back(velocity.x, velocity.y, _msg->depths[i]);
  }
  posim_gz_world_plugins::TidalOscillation tide;
  if (this->dataPtr->tideFlag)
  {
    tide.worldStartTime = {
      _msg->world_start_time_year, _msg->world_start_time_month, _msg->world_start_time_day,
      _msg->world_start_time_hour, _msg->world_start_time_minute};
    tide.ebbDirection = _msg->ebb_direction;
    tide.floodDirection = _msg->flood_direction;
    if (_msg->tideconstituents)
    {
      tide.M2_amp = _msg->m2_amp;
      tide.M2_phase = _msg->m2_phase;
      tide.M2_speed = _msg->m2_speed;
      tide.S2_amp = _msg->s2_amp;
      tide.S2_phase = _msg->s2_phase;
      tide.S2_speed = _msg->s2_speed;
      tide.N2_amp = _msg->n2_amp;
      tide.N2_phase = _msg->n2_phase;
      tide.N2_speed = _msg->n2_speed;
    }
    else
    {
      const auto count = _msg->time_gmt_year.size();
      if (
        count != _msg->time_gmt_month.size() || count != _msg->time_gmt_day.size() ||
        count != _msg->time_gmt_hour.size() || count != _msg->time_gmt_minute.size() ||
        count != _msg->tidevelocities.size())
      {
        gzerr << "Ignoring incomplete tidal database." << std::endl;
        return;
      }
      for (size_t i = 0; i < count; ++i)
      {
        tide.dateGMT.push_back(
          {_msg->time_gmt_year[i], _msg->time_gmt_month[i], _msg->time_gmt_day[i],
           _msg->time_gmt_hour[i], _msg->time_gmt_minute[i]});
        tide.speedcmsec.push_back(_msg->tidevelocities[i]);
      }
    }
    if (!tide.Initiate(_msg->tideconstituents))
    {
      gzerr << "Ignoring invalid tidal database." << std::endl;
      return;
    }
  }
  std::lock_guard<std::mutex> lock(this->dataPtr->lock_);
  this->dataPtr->database = std::move(database);
  this->dataPtr->tide = std::move(tide);
}

//////////////////////////////////////////
gz::sim::Entity OceanCurrentModelPlugin::GetModelEntity(
  const std::string & modelName, gz::sim::EntityComponentManager & ecm)
{
  gz::sim::Entity modelEntity = gz::sim::kNullEntity;

  ecm.Each<gz::sim::components::Name>(
    [&](const gz::sim::Entity & entity, const gz::sim::components::Name * nameComp) -> bool
    {
      if (nameComp->Data() == modelName)
      {
        modelEntity = entity;
        return false;  // Stop iteration
      }
      return true;  // Continue iteration
    });

  return modelEntity;
}

/////////////////////////////////////////////////
gz::math::Pose3d OceanCurrentModelPlugin::GetModelPose(
  const gz::sim::Entity & modelEntity, gz::sim::EntityComponentManager & ecm)
{
  const auto * poseComp = ecm.Component<gz::sim::components::Pose>(modelEntity);
  if (poseComp)
  {
    return gz::sim::worldPose(modelEntity, ecm);
  }
  else
  {
    gzerr << "Pose component not found for entity: " << modelEntity << std::endl;
    return gz::math::Pose3d::Zero;
  }
}

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::Update(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  if (
    !this->dataPtr->configured || _info.paused || !rclcpp::ok() ||
    (this->dataPtr->sampled && _info.simTime == this->dataPtr->time))
  {
    return;
  }
  // Update vehicle position
  gz::math::Pose3d vehicle_pos = GetModelPose(this->dataPtr->modelEntity, _ecm);
  double vehicleDepth = std::max(0.0, -vehicle_pos.Z());
  this->dataPtr->time = _info.simTime;
  this->dataPtr->sampled = true;
  CalculateOceanCurrent(vehicleDepth);
}

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::CalculateOceanCurrent(double vehicleDepth)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->lock_);
  const auto & database = this->dataPtr->database;
  if (database.empty() || !std::isfinite(vehicleDepth))
  {
    return;
  }
  gz::math::Vector3d current;
  if (vehicleDepth <= database.front().Z())
  {
    current = database.front();
  }
  else if (vehicleDepth >= database.back().Z())
  {
    current = database.back();
  }
  else
  {
    const auto upper = std::upper_bound(
      database.begin(), database.end(), vehicleDepth,
      [](double depth, const gz::math::Vector3d & row) { return depth < row.Z(); });
    const auto lower = upper - 1;
    const auto ratio = (vehicleDepth - lower->Z()) / (upper->Z() - lower->Z());
    current = *lower * (1 - ratio) + *upper * ratio;
  }
  const double seconds = std::chrono::duration<double>(this->dataPtr->time).count();
  double north = current.X();
  double east = current.Y();
  if (this->dataPtr->tideFlag)
  {
    const auto tidal = this->dataPtr->tide.Update(seconds, north);
    north = tidal.first;
    east = tidal.second;
  }
  const auto sample =
    [seconds](posim_gz_world_plugins::GaussMarkovProcess & model, double mean, double amplitude)
  {
    model.mean = mean;
    model.min = mean - amplitude;
    model.max = mean + amplitude;
    model.var = mean;
    return model.Update(seconds);
  };
  this->dataPtr->currentVelocity.Set(
    sample(this->dataPtr->currentVelNorthModel, north, this->dataPtr->noiseAmp_North),
    sample(this->dataPtr->currentVelEastModel, east, this->dataPtr->noiseAmp_East),
    sample(this->dataPtr->currentVelDownModel, 0, this->dataPtr->noiseAmp_Down));
}

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!this->dataPtr->configured || _info.paused || !rclcpp::ok())
  {
    return;
  }
  try
  {
    this->dataPtr->lastUpdate = _info.simTime;
    PublishCurrentVelocity(_info);
    rclcpp::spin_some(this->ros_node_);
  }
  catch (const rclcpp::exceptions::RCLError &)
  {
    if (rclcpp::ok())
    {
      throw;
    }
  }
}

/////////////////////////////////////////////////
void OceanCurrentModelPlugin::PublishCurrentVelocity(const gz::sim::UpdateInfo & _info)
{
  if (!this->dataPtr->configured || !rclcpp::ok())
  {
    return;
  }
  geometry_msgs::msg::TwistStamped flowVelMsg;
  const auto seconds = std::chrono::floor<std::chrono::seconds>(_info.simTime);
  flowVelMsg.header.stamp.sec = seconds.count();
  flowVelMsg.header.stamp.nanosec =
    std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime - seconds).count();
  flowVelMsg.header.frame_id = "/world";
  flowVelMsg.twist.linear.x = this->dataPtr->currentVelocity.X();
  flowVelMsg.twist.linear.y = this->dataPtr->currentVelocity.Y();
  flowVelMsg.twist.linear.z = this->dataPtr->currentVelocity.Z();

  // For Testing with higher current velocities -
  // flowVelMsg.twist.linear.x = 25;
  // flowVelMsg.twist.linear.y = 25;
  // flowVelMsg.twist.linear.z = 0;
  this->dataPtr->flowVelocityPub->publish(flowVelMsg);

  // Generate and publish Gazebo topic according to the vehicle depth
  gz::msgs::Vector3d currentVel;
  currentVel.set_x(this->dataPtr->currentVelocity.X());
  currentVel.set_y(this->dataPtr->currentVelocity.Y());
  currentVel.set_z(this->dataPtr->currentVelocity.Z());
  this->dataPtr->gz_current_vel_pub.Publish(currentVel);
}

}  // namespace posim_gz_model_plugins
