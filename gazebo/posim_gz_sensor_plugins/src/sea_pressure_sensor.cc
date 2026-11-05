#include "posim_gz_sensor_plugins/sea_pressure_sensor.hh"
#include <gz/msgs/fluid_pressure.pb.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components/CustomSensor.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Model.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/sim/components/World.hh>
#include <gz/transport/Node.hh>
#include <memory>
#include <rclcpp/expand_topic_or_service_name.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>

GZ_ADD_PLUGIN(
  posim_gz_sensor_plugins::SubseaPressureSensorPlugin, gz::sim::System,
  posim_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemConfigure,
  posim_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPreUpdate,
  posim_gz_sensor_plugins::SubseaPressureSensorPlugin::ISystemPostUpdate)

namespace posim_gz_sensor_plugins
{
struct SubseaPressureSensorPlugin::PrivateData
{
public:
  double saturation = 3000.0;
  bool configured = false;
  bool measurementReady = false;
  std::chrono::steady_clock::duration measurementTime{0};
  std::chrono::steady_clock::duration lastMeasurementTime{0};
  bool hasMeasurement = false;
  bool estimateDepth = true;
  double standardPressure = 101.325;
  double kPaPerM = 9.80638;
  std::shared_ptr<gz::transport::Node> gazeboNode;
  gz::transport::Node::Publisher gz_pressure_sensor_pub;
  rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr ros_pressure_sensor_pub;
  rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr ros_depth_estimate_pub;
  std::string robotNamespace;
  std::string topic;
  double noiseAmp = 0.0;
  double noiseSigma = 3.0;
  double updateRate = 0.0;
  double inferredDepth = 0.0;
  double pressure = 0.0;
  gz::sim::Entity modelEntity = gz::sim::kNullEntity;
};

SubseaPressureSensorPlugin::SubseaPressureSensorPlugin() : dataPtr(std::make_unique<PrivateData>())
{
}

SubseaPressureSensorPlugin::~SubseaPressureSensorPlugin() {}

void SubseaPressureSensorPlugin::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  this->dataPtr = std::make_unique<PrivateData>();
  this->rosNode.reset();
  if (!_sdf || !_sdf->HasElement("namespace"))
  {
    gzerr << "Missing required parameter <namespace>, "
          << "plugin will not be initialized." << std::endl;
    return;
  }
  this->dataPtr->robotNamespace = _sdf->Get<std::string>("namespace");
  this->dataPtr->topic = _sdf->Get<std::string>("topic", "sea_pressure").first;
  bool parsed = true;
  const auto parameter = [&](const char * key, auto defaultValue)
  {
    sdf::Errors errors;
    const auto value = _sdf->Get<decltype(defaultValue)>(errors, key, defaultValue);
    parsed = parsed && errors.empty() && (!_sdf->HasElement(key) || value.second);
    if (!errors.empty())
    {
      gzerr << "Invalid pressure parameter <" << key << ">." << std::endl;
    }
    return value.first;
  };
  this->dataPtr->saturation = parameter("saturation", 3000.0);
  this->dataPtr->estimateDepth = parameter("estimate_depth_on", true);
  this->dataPtr->standardPressure = parameter("standard_pressure", 101.325);
  this->dataPtr->kPaPerM = parameter("kPa_per_meter", 9.80638);
  this->dataPtr->updateRate = parameter("update_rate", 0.0);
  if (
    !_ecm.Component<gz::sim::components::Model>(_entity) ||
    !_ecm.Component<gz::sim::components::Pose>(_entity))
  {
    gzerr << "The pressure plugin must be attached to a model with a pose." << std::endl;
    return;
  }
  if (
    !parsed || !std::isfinite(this->dataPtr->standardPressure) ||
    this->dataPtr->standardPressure < 0.0 || !std::isfinite(this->dataPtr->kPaPerM) ||
    this->dataPtr->kPaPerM <= 0.0 || !std::isfinite(this->dataPtr->updateRate))
  {
    gzerr << "Pressure parameters must be finite, with standard_pressure >= 0 and "
             "kPa_per_meter > 0."
          << std::endl;
    return;
  }

  std::string outputTopic;
  try
  {
    outputTopic = rclcpp::expand_topic_or_service_name(
      "/model/" + this->dataPtr->robotNamespace + "/" + this->dataPtr->topic,
      "subsea_pressure_sensor", "/");
  }
  catch (const rclcpp::exceptions::InvalidTopicNameError & error)
  {
    gzerr << "Invalid pressure namespace/topic: " << error.what() << std::endl;
    return;
  }

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  }
  this->rosNode = std::make_shared<rclcpp::Node>("subsea_pressure_sensor");
  this->dataPtr->gazeboNode = std::make_shared<gz::transport::Node>();
  this->dataPtr->modelEntity = _entity;

  this->dataPtr->ros_pressure_sensor_pub =
    this->rosNode->create_publisher<sensor_msgs::msg::FluidPressure>(
      outputTopic, rclcpp::QoS(10).reliable());

  if (this->dataPtr->estimateDepth)
  {
    this->dataPtr->ros_depth_estimate_pub =
      this->rosNode->create_publisher<geometry_msgs::msg::PointStamped>(
        outputTopic + "_depth", rclcpp::QoS(10).reliable());
  }

  this->dataPtr->gz_pressure_sensor_pub =
    this->dataPtr->gazeboNode->Advertise<gz::msgs::FluidPressure>(outputTopic);
  this->dataPtr->configured = static_cast<bool>(this->dataPtr->gz_pressure_sensor_pub);
}
//////////////////////////////////////////

gz::sim::Entity SubseaPressureSensorPlugin::GetModelEntity(
  const std::string & modelName, gz::sim::EntityComponentManager & ecm)
{
  gz::sim::Entity modelEntity = gz::sim::kNullEntity;

  ecm.Each<gz::sim::components::Model, gz::sim::components::Name>(
    [&](
      const gz::sim::Entity & entity, const gz::sim::components::Model *,
      const gz::sim::components::Name * nameComp) -> bool
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

gz::math::Pose3d SubseaPressureSensorPlugin::GetModelPose(
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

void SubseaPressureSensorPlugin::PreUpdate(
  const gz::sim::UpdateInfo & _info, gz::sim::EntityComponentManager & _ecm)
{
  this->dataPtr->measurementReady = false;
  if (
    !this->dataPtr->configured || _info.paused || _info.simTime.count() < 0 ||
    !_ecm.Component<gz::sim::components::Model>(this->dataPtr->modelEntity) ||
    !_ecm.Component<gz::sim::components::Pose>(this->dataPtr->modelEntity))
  {
    return;
  }
  gz::math::Pose3d sea_pressure_sensor_pos = GetModelPose(this->dataPtr->modelEntity, _ecm);
  if (!sea_pressure_sensor_pos.IsFinite())
  {
    return;
  }
  // POSIM worlds use ENU with the water surface at z=0.
  const double depth = std::max(0.0, -sea_pressure_sensor_pos.Z());
  this->dataPtr->pressure = this->dataPtr->standardPressure + depth * this->dataPtr->kPaPerM;

  // Apply the configured constant pressure offset.
  this->dataPtr->pressure += this->dataPtr->noiseAmp;  // noiseAmp is 0.0

  // double inferredDepth = 0.0;
  if (this->dataPtr->estimateDepth)
  {
    this->dataPtr->inferredDepth =
      (this->dataPtr->pressure - this->dataPtr->standardPressure) / this->dataPtr->kPaPerM;
  }
  this->dataPtr->measurementTime = _info.simTime;
  this->dataPtr->measurementReady =
    std::isfinite(this->dataPtr->pressure * 1000.0) && std::isfinite(this->dataPtr->inferredDepth);
}

void SubseaPressureSensorPlugin::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (
    !this->dataPtr->configured || !this->dataPtr->measurementReady || _info.paused ||
    this->dataPtr->measurementTime != _info.simTime || !this->rosNode ||
    !this->dataPtr->ros_pressure_sensor_pub)
  {
    return;
  }
  this->dataPtr->measurementReady = false;
  const auto context = this->rosNode->get_node_base_interface()->get_context();
  if (!rclcpp::ok(context))
  {
    return;
  }
  try
  {
    if (this->dataPtr->hasMeasurement && _info.simTime >= this->dataPtr->lastMeasurementTime)
    {
      const double elapsed =
        std::chrono::duration<double>(_info.simTime - this->dataPtr->lastMeasurementTime).count();
      if (
        elapsed == 0.0 ||
        (this->dataPtr->updateRate > 0.0 && elapsed * this->dataPtr->updateRate < 1.0))
      {
        return;
      }
    }

    this->dataPtr->lastMeasurementTime = _info.simTime;
    this->dataPtr->hasMeasurement = true;

    // The pressure model uses kPa internally, while both Gazebo and ROS
    // FluidPressure messages require Pa (and Pa^2 for variance).
    constexpr double kPaToPa = 1000.0;
    const double pressurePa = this->dataPtr->pressure * kPaToPa;
    const double variancePa2 =
      this->dataPtr->noiseSigma * this->dataPtr->noiseSigma * kPaToPa * kPaToPa;

    // Publishing Sea_Pressure and depth estimate on gazebo topic
    gz::msgs::FluidPressure gzPressureMsg;
    gzPressureMsg.mutable_header()->mutable_stamp()->set_sec(
      std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count());
    gzPressureMsg.mutable_header()->mutable_stamp()->set_nsec(
      std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime).count() % 1000000000);
    gzPressureMsg.set_pressure(pressurePa);
    gzPressureMsg.set_variance(variancePa2);

    // Publishing the pressure message
    this->dataPtr->gz_pressure_sensor_pub.Publish(gzPressureMsg);

    // Publishing Sea_Pressure on Ros Topic
    sensor_msgs::msg::FluidPressure rosPressureMsg;
    rosPressureMsg.header.stamp.sec =
      std::chrono::duration_cast<std::chrono::seconds>(_info.simTime).count();  // Time in seconds
    rosPressureMsg.header.stamp.nanosec =
      std::chrono::duration_cast<std::chrono::nanoseconds>(_info.simTime).count() %
      1000000000;  // Time in nanoseconds
    rosPressureMsg.fluid_pressure = pressurePa;
    rosPressureMsg.variance = variancePa2;
    this->dataPtr->ros_pressure_sensor_pub->publish(rosPressureMsg);

    // publishing depth message
    if (this->dataPtr->estimateDepth)
    {
      geometry_msgs::msg::PointStamped rosDepthMsg;
      rosDepthMsg.point.z = this->dataPtr->inferredDepth;
      rosDepthMsg.header = rosPressureMsg.header;
      this->dataPtr->ros_depth_estimate_pub->publish(rosDepthMsg);
    }

    if (!_info.paused)
    {
      rclcpp::spin_some(this->rosNode);

      if (_info.iterations % 1000 == 0)
      {
        gzmsg << "posim_ros_gz_plugins::SubseaPressureSensorPlugin::PostUpdate" << std::endl;
      }
    }
  }
  catch (const rclcpp::exceptions::RCLError &)
  {
    // The context may be invalidated between the check and publication/spin.
    // Do not hide unrelated failures while ROS remains valid.
    if (rclcpp::ok(context))
    {
      throw;
    }
  }
}

}  // namespace posim_gz_sensor_plugins
