/*
 * Copyright (C) 2024 Rakesh Vivekanandan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <functional>
#include <iostream>

#include <gz/common/Console.hh>
#include <gz/transport/Node.hh>
#include <rclcpp/rclcpp.hpp>
#include "gz/plugin/Register.hh"

#include "posim_ros_gz_plugins/DVLBridge.hh"

GZ_ADD_PLUGIN(
  posim_ros_gz_plugins::DVLBridge, gz::sim::System,
  posim_ros_gz_plugins::DVLBridge::ISystemConfigure,
  posim_ros_gz_plugins::DVLBridge::ISystemPostUpdate)

namespace posim_ros_gz_plugins
{

struct DVLBridge::PrivateData
{
  // Add any private data members here.
  std::mutex mutex_;
  gz::transport::Node gz_node;
  std::string dvl_topic;
  rclcpp::Publisher<posim_interfaces::msg::DVL>::SharedPtr dvl_pub;
};

struct DVLBridge::CallbackState
{
  std::mutex mutex;
  DVLBridge * owner = nullptr;
};

DVLBridge::DVLBridge()
: dataPtr(std::make_unique<PrivateData>()), callbackState(std::make_shared<CallbackState>())
{
  this->callbackState->owner = this;
}

DVLBridge::~DVLBridge()
{
  {
    std::lock_guard<std::mutex> lock(this->callbackState->mutex);
    this->callbackState->owner = nullptr;
  }
  if (!this->dataPtr->dvl_topic.empty())
  {
    this->dataPtr->gz_node.Unsubscribe(this->dataPtr->dvl_topic);
  }
}

void DVLBridge::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "posim_ros_gz_plugins::DVLBridge::Configure on entity: " << _entity << std::endl;

  if (!rclcpp::ok())
  {
    rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  }

  this->ros_node_ = std::make_shared<rclcpp::Node>("dvl_bridge_node");

  // Grab dvl topic from SDF
  if (!_sdf->HasElement("topic"))
  {
    this->dataPtr->dvl_topic = "/dvl/velocity";
    gzmsg << "dvl topic set to default:  " << this->dataPtr->dvl_topic << std::endl;
  }
  else
  {
    this->dataPtr->dvl_topic = _sdf->Get<std::string>("topic");
    gzmsg << "dvl topic: " << this->dataPtr->dvl_topic << std::endl;
  }

  // Construct the publisher before transport can call into the bridge.
  this->dataPtr->dvl_pub =
    this->ros_node_->create_publisher<posim_interfaces::msg::DVL>(this->dataPtr->dvl_topic, 1);
  const auto state = this->callbackState;
  std::function<void(const gz::msgs::DVLVelocityTracking &)> callback =
    [state](const gz::msgs::DVLVelocityTracking & message)
  {
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->owner)
    {
      state->owner->receiveGazeboCallback(message);
    }
  };
  this->dataPtr->gz_node.Subscribe(this->dataPtr->dvl_topic, callback);
}

void DVLBridge::receiveGazeboCallback(const gz::msgs::DVLVelocityTracking & msg)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex_);

  if (
    !this->ros_node_ || !this->dataPtr->dvl_pub ||
    !rclcpp::ok(this->ros_node_->get_node_base_interface()->get_context()))
  {
    return;
  }

  auto dvl_msg = posim_interfaces::msg::DVL();
  dvl_msg.header.stamp.sec = msg.header().stamp().sec();
  dvl_msg.header.stamp.nanosec = msg.header().stamp().nsec();
  for (int i = 0; i < msg.header().data_size(); ++i)
  {
    const auto & entry = msg.header().data(i);
    if (entry.key() == "frame_id" && entry.value_size() > 0)
    {
      dvl_msg.header.frame_id = entry.value(0);
      std::string::size_type pos = 0;
      while ((pos = dvl_msg.header.frame_id.find("::", pos)) != std::string::npos)
      {
        dvl_msg.header.frame_id.replace(pos, 2, "/");
        ++pos;
      }
      break;
    }
  }

  std::string dvl_type;
  std::string target_type;
  std::string reference_type;

  switch (msg.type())
  {
    case gz::msgs::DVLVelocityTracking::DVL_TYPE_UNSPECIFIED:
      dvl_type = "DVL_TYPE_UNSPECIFIED";
      break;
    case gz::msgs::DVLVelocityTracking::DVL_TYPE_PISTON:
      dvl_type = "DVL_TYPE_PISTON";
      break;
    case gz::msgs::DVLVelocityTracking::DVL_TYPE_PHASED_ARRAY:
      dvl_type = "DVL_TYPE_PHASED_ARRAY";
      break;
    default:
      dvl_type = "DVL_TYPE_UNSPECIFIED";
  }

  switch (msg.target().type())
  {
    case gz::msgs::DVLTrackingTarget::DVL_TARGET_UNSPECIFIED:
      target_type = "DVL_TARGET_UNSPECIFIED";
      break;
    case gz::msgs::DVLTrackingTarget::DVL_TARGET_BOTTOM:
      target_type = "DVL_TARGET_BOTTOM";
      break;
    case gz::msgs::DVLTrackingTarget::DVL_TARGET_WATER_MASS:
      target_type = "DVL_TARGET_WATER_MASS";
      break;
    default:
      target_type = "DVL_TARGET_UNSPECIFIED";
  }

  dvl_msg.type = dvl_type;

  dvl_msg.target.type = target_type;
  dvl_msg.target.range = msg.target().range().mean();

  dvl_msg.velocity.twist.linear.x = msg.velocity().mean().x();
  dvl_msg.velocity.twist.linear.y = msg.velocity().mean().y();
  dvl_msg.velocity.twist.linear.z = msg.velocity().mean().z();

  const size_t covariance_size = std::min<size_t>(9, msg.velocity().covariance().size());

  for (size_t i = 0; i < covariance_size; i++)
  {
    // Gazebo is 3x3 linear covariance; ROS TwistWithCovariance is 6x6.
    dvl_msg.velocity.covariance[(i / 3) * 6 + i % 3] = msg.velocity().covariance()[i];
  }

  size_t beam_size = msg.beams().size();

  for (size_t i = 0; i < beam_size; i++)
  {
    auto beam = posim_interfaces::msg::DVLBeam();
    beam.id = msg.beams()[i].id();

    switch (msg.beams()[i].velocity().reference())
    {
      case gz::msgs::DVLKinematicEstimate::DVL_REFERENCE_UNSPECIFIED:
        reference_type = "DVL_REFERENCE_UNSPECIFIED";
        break;
      case gz::msgs::DVLKinematicEstimate::DVL_REFERENCE_EARTH:
        reference_type = "DVL_REFERENCE_EARTH";
        break;
      case gz::msgs::DVLKinematicEstimate::DVL_REFERENCE_SHIP:
        reference_type = "DVL_REFERENCE_SHIP";
        break;
      default:
        reference_type = "DVL_REFERENCE_UNSPECIFIED";
    }

    beam.reference = reference_type;
    beam.range = msg.beams()[i].range().mean();
    beam.locked = msg.beams()[i].locked();

    beam.velocity.twist.linear.x = msg.beams()[i].velocity().mean().x();
    beam.velocity.twist.linear.y = msg.beams()[i].velocity().mean().y();
    beam.velocity.twist.linear.z = msg.beams()[i].velocity().mean().z();

    const size_t beam_covariance_size =
      std::min<size_t>(9, msg.beams()[i].velocity().covariance().size());

    for (size_t j = 0; j < beam_covariance_size; j++)
    {
      beam.velocity.covariance[(j / 3) * 6 + j % 3] = msg.beams()[i].velocity().covariance()[j];
    }

    dvl_msg.beams.push_back(beam);
  }

  this->dataPtr->dvl_pub->publish(dvl_msg);
}

void DVLBridge::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (_info.paused || !this->ros_node_ || !rclcpp::ok())
  {
    return;
  }

  try
  {
    rclcpp::spin_some(this->ros_node_);
  }
  catch (const rclcpp::exceptions::RCLError & error)
  {
    // SIGINT can invalidate the global context between the ok() check and
    // temporary executor construction inside rclcpp::spin_some().
    if (rclcpp::ok())
    {
      throw;
    }
    gzdbg << "Skipping DVL ROS spin during shutdown: " << error.what() << std::endl;
    return;
  }

  if (_info.iterations % 1000 == 0)
  {
    gzmsg << "posim_ros_gz_plugins::DVLBridge::PostUpdate" << std::endl;
  }
}

}  // namespace posim_ros_gz_plugins
