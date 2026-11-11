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

// Include the necessary headers for Ocean Current Plugin
#include "posim_gz_world_plugins/OceanCurrentWorldPlugin.hh"

// Include messages for Stratified Current Velocity
#include <posim_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.pb.h>

// Include Gazebo and Ignition libraries
#include <gz/msgs/vector3d.pb.h>
#include <gz/common/Util.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Vector4.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>

// Boost libraries for string manipulation, binding, and shared pointers
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>

// Other necessary libraries
#include <math.h>
#include <chrono>
#include <map>
#include <memory>
#include <string>

// AMENT library for package directory functions
#include <ament_index_cpp/get_package_share_directory.hpp>

// Custom plugin headers
#include "CurrentProfile.hh"
#include "TidalProfile.hh"
#include "posim_gz_world_plugins/gauss_markov_process.hh"  // For Gauss-Markov process
#include "posim_gz_world_plugins/tidal_oscillation.hh"     // For tidal oscillation modeling

GZ_ADD_PLUGIN(
  posim_gz_world_plugins::OceanCurrentWorldPlugin, gz::sim::System,
  posim_gz_world_plugins::OceanCurrentWorldPlugin::ISystemConfigure,
  posim_gz_world_plugins::OceanCurrentWorldPlugin::ISystemUpdate,
  posim_gz_world_plugins::OceanCurrentWorldPlugin::ISystemPostUpdate)

namespace posim_gz_world_plugins
{
struct OceanCurrentWorldPlugin::PrivateData
{
  // Gazebo Simulation and World Components
  gz::sim::World world{gz::sim::kNullEntity};  // Gazebo world instance
  sdf::ElementPtr sdf;                         // Pointer to the SDF element
  bool hasSurface;                             // Flag to check if surface is present
  bool configured = false;

  // Communication Node for Gazebo
  // std::shared_ptr<gz::transport::Node> gz_node;  // Transport node for communication
  gz::transport::Node gz_node;

  // Publishers
  gz::transport::Node::Publisher gz_node_cvel_world_pub;  // Publisher for current velocity

  // Publishers
  gz::transport::Node::Publisher
    gz_node_scvel_world_pub;  // Publisher for Stratified current velocity

  // Subscribers
  gz::transport::Node subscriber;  // Subscriber for vehicle depth

  // Database File Paths
  std::string databaseFilePath;  // Path to ocean current database (CSV file)
  std::string tidalFilePath;     // Path to tidal oscillation database (TXT file)

  // Namespace for Topics and Services
  std::string ns;  // Namespace for topics and services

  // Tidal Oscillation Variables
  bool tideFlag;
  posim_gz_world_plugins::TidalOscillation tide;

  // Timing
  std::chrono::steady_clock::duration lastUpdate{0};

  // Stratified Current Database Path
  std::string db_path;  // Path to the stratified current database

  // ROS 2 Node
  std::shared_ptr<rclcpp::Node> rosNode;
};

OceanCurrentWorldPlugin * OceanCurrentWorldPlugin::singletonInstance = nullptr;

OceanCurrentWorldPlugin::OceanCurrentWorldPlugin()
: dataPtr(std::make_unique<PrivateData>()), sharedDataPtr(std::shared_ptr<SharedData>())
{
  this->sharedDataPtr = std::make_shared<SharedData>();
}

/////////////////////////////////////////////////

OceanCurrentWorldPlugin::~OceanCurrentWorldPlugin()
{
  // Clear the static pointer if we are the instance
  if (OceanCurrentWorldPlugin::singletonInstance == this)
  {
    OceanCurrentWorldPlugin::singletonInstance = nullptr;
  }
}

OceanCurrentWorldPlugin * OceanCurrentWorldPlugin::Instance()
{
  return OceanCurrentWorldPlugin::singletonInstance;
}

// ----------------------------------------------

void OceanCurrentWorldPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventMgr)
{
  this->dataPtr->world = gz::sim::World(_entity);
  if (!this->dataPtr->world.Valid(_ecm))  // check
  {
    gzerr << "World entity not found" << std::endl;
    return;
  }
  if (
    !_sdf || !_sdf->HasElement("namespace") || !_sdf->HasElement("constant_current") ||
    !_sdf->HasElement("transient_current"))
  {
    gzerr << "OceanCurrentWorldPlugin requires <namespace>, <constant_current> and "
          << "<transient_current>. Current publication is disabled." << std::endl;
    return;
  }
  this->dataPtr->sdf = _sdf->Clone();

  // Read the namespace for topics and services
  this->dataPtr->ns = _sdf->Get<std::string>("namespace");
  if (this->dataPtr->ns.empty())
  {
    gzerr << "OceanCurrentWorldPlugin requires a nonempty namespace." << std::endl;
    return;
  }

  gzmsg << "Loading Ocean current world plugin" << std::endl;

  LoadGlobalCurrentConfig();
  for (auto * model :
       {&this->sharedDataPtr->currentVelModel, &this->sharedDataPtr->currentHorzAngleModel,
        &this->sharedDataPtr->currentVertAngleModel})
  {
    if (!model->SetModel(model->mean, model->min, model->max, model->mu, model->noiseAmp))
    {
      gzerr << "Invalid current model parameters. Current publication is disabled." << std::endl;
      return;
    }
  }
  if (!LoadStratifiedCurrentDatabase())
  {
    return;
  }

  if (_sdf->HasElement("tidal_oscillation"))
  {
    if (!LoadTidalOscillationDatabase())
    {
      return;
    }
    gzmsg << "Tidal oscillation is enabled" << std::endl;
  }
  else
  {
    gzmsg << "Tidal oscillation not enabled" << std::endl;
  }

  if (!OceanCurrentWorldPlugin::singletonInstance)
  {
    OceanCurrentWorldPlugin::singletonInstance = this;
  }

  this->dataPtr->configured = true;

  gzmsg << "Underwater current plugin loaded!" << std::endl
        << "\tWARNING: Current velocity calculated in the ENU frame" << std::endl;
}

bool OceanCurrentWorldPlugin::LoadTidalOscillationDatabase()
{
  const auto params = this->dataPtr->sdf->GetElement("tidal_oscillation");
  TidalOscillation tide;
  const auto read = [](const sdf::ElementPtr & element, const char * name, auto & value)
  {
    if (
      !element || !element->HasElement(name) || !element->GetElement(name)->GetValue()->Get(value))
    {
      throw std::invalid_argument(std::string("missing or invalid tidal parameter: ") + name);
    }
  };
  try
  {
    if (!params->HasElement("mean_direction") || !params->HasElement("world_start_time_GMT"))
    {
      throw std::invalid_argument("tidal directions and world start time are required");
    }
    const auto direction = params->GetElement("mean_direction");
    read(direction, "ebb", tide.ebbDirection);
    read(direction, "flood", tide.floodDirection);
    const auto start = params->GetElement("world_start_time_GMT");
    read(start, "year", tide.worldStartTime[0]);
    read(start, "month", tide.worldStartTime[1]);
    read(start, "day", tide.worldStartTime[2]);
    read(start, "hour", tide.worldStartTime[3]);
    read(start, "minute", tide.worldStartTime[4]);
    const bool harmonic =
      !params->HasElement("databasefilePath") && params->HasElement("harmonic_constituents");
    if (harmonic)
    {
      const auto constituents = params->GetElement("harmonic_constituents");
      const auto load = [&](const char * name, double & amplitude, double & phase, double & speed)
      {
        if (!constituents->HasElement(name))
        {
          throw std::invalid_argument(std::string("missing tidal constituent: ") + name);
        }
        const auto element = constituents->GetElement(name);
        read(element, "amp", amplitude);
        read(element, "phase", phase);
        read(element, "speed", speed);
      };
      load("M2", tide.M2_amp, tide.M2_phase, tide.M2_speed);
      load("S2", tide.S2_amp, tide.S2_phase, tide.S2_speed);
      load("N2", tide.N2_amp, tide.N2_phase, tide.N2_speed);
    }
    else
    {
      this->dataPtr->tidalFilePath =
        params->HasElement("databasefilePath")
          ? params->Get<std::string>("databasefilePath")
          : ament_index_cpp::get_package_share_directory("posim_worlds") +
              "/worlds/ACT1951_1_Annual_2021.csv";
      std::ifstream csv(this->dataPtr->tidalFilePath);
      if (!csv)
      {
        gz::common::SystemPaths paths;
        this->dataPtr->tidalFilePath = paths.FindFile(this->dataPtr->tidalFilePath, true);
        csv.clear();
        csv.open(this->dataPtr->tidalFilePath);
      }
      if (!csv)
      {
        throw std::invalid_argument("tidal CSV file does not exist");
      }
      auto profile = detail::ReadTidalProfile(csv);
      tide.dateGMT = std::move(profile.dateGMT);
      tide.speedcmsec = std::move(profile.speedcmsec);
    }
    if (!tide.Initiate(harmonic))
    {
      throw std::invalid_argument("invalid tidal dates, directions or constituent values");
    }
    auto & data = *this->sharedDataPtr;
    data.tidalHarmonicFlag = harmonic;
    data.dateGMT = tide.dateGMT;
    data.speedcmsec = tide.speedcmsec;
    data.ebbDirection = tide.ebbDirection;
    data.floodDirection = tide.floodDirection;
    data.world_start_time_year = tide.worldStartTime[0];
    data.world_start_time_month = tide.worldStartTime[1];
    data.world_start_time_day = tide.worldStartTime[2];
    data.world_start_time_hour = tide.worldStartTime[3];
    data.world_start_time_minute = tide.worldStartTime[4];
    data.M2_amp = tide.M2_amp;
    data.M2_phase = tide.M2_phase;
    data.M2_speed = tide.M2_speed;
    data.S2_amp = tide.S2_amp;
    data.S2_phase = tide.S2_phase;
    data.S2_speed = tide.S2_speed;
    data.N2_amp = tide.N2_amp;
    data.N2_phase = tide.N2_phase;
    data.N2_speed = tide.N2_speed;
    this->dataPtr->tide = std::move(tide);
    this->dataPtr->tideFlag = true;
    return true;
  }
  catch (const std::exception & error)
  {
    gzerr << "Invalid tidal configuration: " << error.what() << ". Current publication is disabled."
          << std::endl;
    return false;
  }
}

/////////////////////////////////////////////////
bool OceanCurrentWorldPlugin::LoadStratifiedCurrentDatabase()
{
  sdf::ElementPtr transientCurrentParams;

  if (this->dataPtr->sdf->HasElement("transient_current"))
  {
    transientCurrentParams = this->dataPtr->sdf->GetElement("transient_current");
  }
  else
  {
    gzerr << "Transient current configuration not available" << std::endl;
    return false;
  }

  if (transientCurrentParams->HasElement("topic_stratified"))
  {
    this->sharedDataPtr->stratifiedCurrentVelocityTopic =
      transientCurrentParams->Get<std::string>("topic_stratified");
  }
  else
  {
    this->sharedDataPtr->stratifiedCurrentVelocityTopic = "stratified_current_velocity";
  }

  if (this->sharedDataPtr->stratifiedCurrentVelocityTopic.empty())
  {
    gzerr << "Empty stratified ocean current velocity topic" << std::endl;
  }

  // Read the depth dependent ocean current file path from the SDF file
  if (transientCurrentParams->HasElement("databasefilePath"))
  {
    this->dataPtr->databaseFilePath = transientCurrentParams->Get<std::string>("databasefilePath");
  }
  else if (transientCurrentParams->HasElement("databasefileName"))
  {
    this->dataPtr->databaseFilePath = ament_index_cpp::get_package_share_directory("posim_worlds") +
                                      "/worlds/" +
                                      transientCurrentParams->Get<std::string>("databasefileName");
  }
  else
  {
    this->dataPtr->databaseFilePath = ament_index_cpp::get_package_share_directory("posim_worlds") +
                                      "/worlds/transientOceanCurrentDatabase.csv";
  }

  if (this->dataPtr->databaseFilePath.empty())
  {
    gzerr << "Empty stratified ocean current database file path" << std::endl;
    return false;
  }

  gzmsg << this->dataPtr->databaseFilePath << std::endl;

  std::ifstream csvFile(this->dataPtr->databaseFilePath);
  if (!csvFile)
  {
    gz::common::SystemPaths paths;
    this->dataPtr->databaseFilePath = paths.FindFile(this->dataPtr->databaseFilePath, true);
    csvFile.clear();
    csvFile.open(this->dataPtr->databaseFilePath);
  }

  if (!csvFile)
  {
    gzerr << "Stratified Ocean database file does not exist" << std::endl;
    return false;
  }

  std::vector<gz::math::Vector3d> profile;
  try
  {
    profile = detail::ReadCurrentProfile(csvFile);
  }
  catch (const std::invalid_argument & error)
  {
    gzerr << "Invalid current profile " << this->dataPtr->databaseFilePath << ": " << error.what()
          << std::endl;
    return false;
  }
  this->sharedDataPtr->stratifiedDatabase = std::move(profile);
  this->sharedDataPtr->stratifiedCurrentModels.clear();
  for (const auto & row : this->sharedDataPtr->stratifiedDatabase)
  {
    // Create Gauss-Markov processes for the stratified currents
    // Means are the database-specified magnitudes & angles, and
    // the other values come from the constant current models

    posim_gz_world_plugins::GaussMarkovProcess magnitudeModel;
    magnitudeModel.mean = hypot(row.Y(), row.X());
    magnitudeModel.var = magnitudeModel.mean;
    magnitudeModel.max = this->sharedDataPtr->currentVelModel.max;
    magnitudeModel.min = 0.0;
    magnitudeModel.mu = this->sharedDataPtr->currentVelModel.mu;
    magnitudeModel.noiseAmp = this->sharedDataPtr->currentVelModel.noiseAmp;
    magnitudeModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    posim_gz_world_plugins::GaussMarkovProcess hAngleModel;
    hAngleModel.mean = atan2(row.Y(), row.X());
    hAngleModel.var = hAngleModel.mean;
    hAngleModel.max = M_PI;
    hAngleModel.min = -M_PI;
    hAngleModel.mu = this->sharedDataPtr->currentHorzAngleModel.mu;
    hAngleModel.noiseAmp = this->sharedDataPtr->currentHorzAngleModel.noiseAmp;
    hAngleModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    posim_gz_world_plugins::GaussMarkovProcess vAngleModel;
    vAngleModel.mean = 0.0;
    vAngleModel.var = vAngleModel.mean;
    vAngleModel.max = M_PI / 2.0;
    vAngleModel.min = -M_PI / 2.0;
    vAngleModel.mu = this->sharedDataPtr->currentVertAngleModel.mu;
    vAngleModel.noiseAmp = this->sharedDataPtr->currentVertAngleModel.noiseAmp;
    vAngleModel.lastUpdate = std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

    std::vector<posim_gz_world_plugins::GaussMarkovProcess> depthModels;
    depthModels.push_back(magnitudeModel);
    depthModels.push_back(hAngleModel);
    depthModels.push_back(vAngleModel);
    this->sharedDataPtr->stratifiedCurrentModels.push_back(depthModels);
  }
  csvFile.close();

  this->dataPtr->gz_node_scvel_world_pub =
    this->dataPtr->gz_node.Advertise<posim_gz_world_plugins_msgs::msgs::StratifiedCurrentVelocity>(
      this->dataPtr->ns + "/" + this->sharedDataPtr->stratifiedCurrentVelocityTopic);

  gzmsg << "Stratified current velocity topic name: "
        << this->dataPtr->ns + "/" + this->sharedDataPtr->stratifiedCurrentVelocityTopic
        << std::endl;
  return true;
}

/////////////////////////////////////////////////
void OceanCurrentWorldPlugin::LoadGlobalCurrentConfig()
{
  // Retrieve the velocity configuration, if existent

  sdf::ElementPtr currentVelocityParams;
  if (this->dataPtr->sdf->HasElement("constant_current"))
  {
    currentVelocityParams = this->dataPtr->sdf->GetElement("constant_current");
  }
  else
  {
    gzerr << "Current configuration not available" << std::endl;
  }

  if (currentVelocityParams->HasElement("use_constant_current"))
  {
    this->sharedDataPtr->use_constant_current =
      currentVelocityParams->Get<bool>("use_constant_current");
  }
  else
  {
    this->sharedDataPtr->use_constant_current = false;
  }

  // Read the topic names from the SDF file
  if (currentVelocityParams->HasElement("topic"))
  {
    this->sharedDataPtr->currentVelocityTopic = currentVelocityParams->Get<std::string>("topic");
  }
  else
  {
    this->sharedDataPtr->currentVelocityTopic = "ocean_current";
  }

  if (this->sharedDataPtr->currentVelocityTopic.empty())
  {
    gzerr << "Empty ocean current velocity topic" << std::endl;
  }

  if (currentVelocityParams->HasElement("velocity"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("velocity");
    if (elem->HasElement("mean"))
    {
      this->sharedDataPtr->currentVelModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->sharedDataPtr->currentVelModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->sharedDataPtr->currentVelModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->sharedDataPtr->currentVelModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->sharedDataPtr->currentVelModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    if (!(this->sharedDataPtr->currentVelModel.min < this->sharedDataPtr->currentVelModel.max))
    {
      gzerr << "Invalid current velocity limits" << std::endl;
    }

    if (!(this->sharedDataPtr->currentVelModel.mean >= this->sharedDataPtr->currentVelModel.min))
    {
      gzerr << "Mean velocity must be greater than minimum" << std::endl;
    }
    if (!(this->sharedDataPtr->currentVelModel.mean <= this->sharedDataPtr->currentVelModel.max))
    {
      gzerr << "Mean velocity must be smaller than maximum" << std::endl;
    }

    if (!(this->sharedDataPtr->currentVelModel.mu >= 0 &&
          this->sharedDataPtr->currentVelModel.mu <= 1))
    {
      gzerr << "Invalid process constant" << std::endl;
    }

    if (!(std::isfinite(this->sharedDataPtr->currentVelModel.noiseAmp) &&
          this->sharedDataPtr->currentVelModel.noiseAmp >= 0))
    {
      gzerr << "Noise amplitude must be finite and nonnegative" << std::endl;
    }
  }

  this->sharedDataPtr->currentVelModel.var = this->sharedDataPtr->currentVelModel.mean;
  gzmsg << "Current velocity [m/s] Gauss-Markov process model:" << std::endl;
  this->sharedDataPtr->currentVelModel.Print();

  if (currentVelocityParams->HasElement("horizontal_angle"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("horizontal_angle");

    if (elem->HasElement("mean"))
    {
      this->sharedDataPtr->currentHorzAngleModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->sharedDataPtr->currentHorzAngleModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->sharedDataPtr->currentHorzAngleModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->sharedDataPtr->currentHorzAngleModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->sharedDataPtr->currentHorzAngleModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    if (!(this->sharedDataPtr->currentHorzAngleModel.min <
          this->sharedDataPtr->currentHorzAngleModel.max))
    {
      gzerr << "Invalid current horizontal angle limits" << std::endl;
    }

    if (!(this->sharedDataPtr->currentHorzAngleModel.mean >=
          this->sharedDataPtr->currentHorzAngleModel.min))
    {
      gzerr << "Mean horizontal angle must be greater than minimum" << std::endl;
    }

    if (!(this->sharedDataPtr->currentHorzAngleModel.mean <=
          this->sharedDataPtr->currentHorzAngleModel.max))
    {
      gzerr << "Mean horizontal angle must be smaller than maximum" << std::endl;
    }

    if (!(this->sharedDataPtr->currentHorzAngleModel.mu >= 0 &&
          this->sharedDataPtr->currentHorzAngleModel.mu <= 1))
    {
      gzerr << "Invalid process constant" << std::endl;
    }

    if (!(std::isfinite(this->sharedDataPtr->currentHorzAngleModel.noiseAmp) &&
          this->sharedDataPtr->currentHorzAngleModel.noiseAmp >= 0))
    {
      gzerr << "Horizontal angle noise amplitude must be finite and nonnegative" << std::endl;
    }
  }

  this->sharedDataPtr->currentHorzAngleModel.var = this->sharedDataPtr->currentHorzAngleModel.mean;
  gzmsg << "Current velocity horizontal angle [rad] Gauss-Markov process model:" << std::endl;
  this->sharedDataPtr->currentHorzAngleModel.Print();

  if (currentVelocityParams->HasElement("vertical_angle"))
  {
    sdf::ElementPtr elem = currentVelocityParams->GetElement("vertical_angle");

    if (elem->HasElement("mean"))
    {
      this->sharedDataPtr->currentVertAngleModel.mean = elem->Get<double>("mean");
    }
    if (elem->HasElement("min"))
    {
      this->sharedDataPtr->currentVertAngleModel.min = elem->Get<double>("min");
    }
    if (elem->HasElement("max"))
    {
      this->sharedDataPtr->currentVertAngleModel.max = elem->Get<double>("max");
    }
    if (elem->HasElement("mu"))
    {
      this->sharedDataPtr->currentVertAngleModel.mu = elem->Get<double>("mu");
    }
    if (elem->HasElement("noiseAmp"))
    {
      this->sharedDataPtr->currentVertAngleModel.noiseAmp = elem->Get<double>("noiseAmp");
    }

    if (!(this->sharedDataPtr->currentVertAngleModel.min <
          this->sharedDataPtr->currentVertAngleModel.max))
    {
      gzerr << "Invalid current vertical angle limits" << std::endl;
    }

    if (!(this->sharedDataPtr->currentVertAngleModel.mean >=
          this->sharedDataPtr->currentVertAngleModel.min))
    {
      gzerr << "Mean vertical angle must be greater than minimum" << std::endl;
    }

    if (!(this->sharedDataPtr->currentVertAngleModel.mean <=
          this->sharedDataPtr->currentVertAngleModel.max))
    {
      gzerr << "Mean vertical angle must be smaller than maximum" << std::endl;
    }

    if (!(this->sharedDataPtr->currentVertAngleModel.mu >= 0 &&
          this->sharedDataPtr->currentVertAngleModel.mu <= 1))
    {
      gzerr << "Invalid process constant" << std::endl;
    }

    if (!(std::isfinite(this->sharedDataPtr->currentVertAngleModel.noiseAmp) &&
          this->sharedDataPtr->currentVertAngleModel.noiseAmp >= 0))
    {
      gzerr << "Vertical angle noise amplitude must be finite and nonnegative" << std::endl;
    }
  }

  this->sharedDataPtr->currentVertAngleModel.var = this->sharedDataPtr->currentVertAngleModel.mean;
  gzmsg << "Current velocity vertical angle [rad] Gauss-Markov process model:" << std::endl;
  this->sharedDataPtr->currentVertAngleModel.Print();

  this->sharedDataPtr->currentVelModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();
  this->sharedDataPtr->currentHorzAngleModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();
  this->sharedDataPtr->currentVertAngleModel.lastUpdate =
    std::chrono::duration<double>(this->dataPtr->lastUpdate).count();

  if (this->sharedDataPtr->use_constant_current)
  {  // Advertise the current velocity & stratified current velocity topics
    this->dataPtr->gz_node_cvel_world_pub = this->dataPtr->gz_node.Advertise<gz::msgs::Vector3d>(
      this->sharedDataPtr->currentVelocityTopic);
    gzmsg << "Current velocity topic name: " << this->sharedDataPtr->currentVelocityTopic
          << std::endl;
  }
}

// ----------------------------------------------

/////////////////////////////////////////////////
void OceanCurrentWorldPlugin::Update(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  if (!this->dataPtr->configured || _info.paused)
  {
    return;
  }
  // Update the time
  auto time = std::chrono::duration<double>(_info.simTime).count();

  // Calculate the flow velocity and the direction using the Gauss-Markov
  // model

  // Update current velocity
  double currentVelMag = this->sharedDataPtr->currentVelModel.Update(time);

  // Update current horizontal direction around z axis of flow frame
  double horzAngle = this->sharedDataPtr->currentHorzAngleModel.Update(time);

  // Update current horizontal direction around z axis of flow frame
  double vertAngle = this->sharedDataPtr->currentVertAngleModel.Update(time);

  // Generate the current velocity vector in the world (ENU) frame.
  this->sharedDataPtr->currentVelocity = gz::math::Vector3d(
    currentVelMag * cos(horzAngle) * cos(vertAngle),
    currentVelMag * sin(horzAngle) * cos(vertAngle), currentVelMag * sin(vertAngle));

  // Generate the depth-specific velocities
  this->sharedDataPtr->currentStratifiedVelocity.clear();
  for (int i = 0; i < this->sharedDataPtr->stratifiedDatabase.size(); i++)
  {
    double depth = this->sharedDataPtr->stratifiedDatabase[i].Z();
    currentVelMag = this->sharedDataPtr->stratifiedCurrentModels[i][0].Update(time);
    horzAngle = this->sharedDataPtr->stratifiedCurrentModels[i][1].Update(time);
    vertAngle = this->sharedDataPtr->stratifiedCurrentModels[i][2].Update(time);
    gz::math::Vector4d depthVel(
      currentVelMag * cos(horzAngle) * cos(vertAngle),
      currentVelMag * sin(horzAngle) * cos(vertAngle), currentVelMag * sin(vertAngle), depth);
    this->sharedDataPtr->currentStratifiedVelocity.push_back(depthVel);
  }
}

// ----------------------------------------------
/////////////////////////////////////////////////
void OceanCurrentWorldPlugin::PublishCurrentVelocity()
{
  gz::msgs::Vector3d currVel;
  gz::msgs::Set(
    &currVel, gz::math::Vector3d(
                this->sharedDataPtr->currentVelocity.X(), this->sharedDataPtr->currentVelocity.Y(),
                this->sharedDataPtr->currentVelocity.Z()));
  this->dataPtr->gz_node_cvel_world_pub.Publish(currVel);
}

/////////////////////////////////////////////////
void OceanCurrentWorldPlugin::PublishStratifiedCurrentVelocity()
{
  posim_gz_world_plugins_msgs::msgs::StratifiedCurrentVelocity stratcurrentVel;  // check
  for (std::vector<gz::math::Vector4d>::iterator it =
         this->sharedDataPtr->currentStratifiedVelocity.begin();
       it != this->sharedDataPtr->currentStratifiedVelocity.end(); ++it)
  {
    gz::msgs::Set(stratcurrentVel.add_velocity(), gz::math::Vector3d(it->X(), it->Y(), it->Z()));
    stratcurrentVel.add_depth(it->W());
  }
  if (stratcurrentVel.velocity_size() == 0)
  {
    return;
  }
  this->dataPtr->gz_node_scvel_world_pub.Publish(stratcurrentVel);
}

/////////////////////////////////////////////////
void OceanCurrentWorldPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!this->dataPtr->configured || _info.paused)
  {
    return;
  }
  // gzmsg << "Ocean current world plugin post update" << std::endl;
  // Update time stamp
  this->dataPtr->lastUpdate = _info.simTime;
  if (this->sharedDataPtr->use_constant_current)
  {
    PublishCurrentVelocity();
  }
  PublishStratifiedCurrentVelocity();
}
// ----------------------------------------------

}  // namespace posim_gz_world_plugins
