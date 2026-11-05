/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <cmath>
#include <iostream>
#include <limits>
#include <optional>

#include <gz/common/Console.hh>
#include <gz/math/SphericalCoordinates.hh>
#include <gz/sim/World.hh>
#include <rclcpp/rclcpp.hpp>
#include "gz/plugin/Register.hh"
#include "gz/sim/components/World.hh"

#include "posim_ros_gz_plugins/SphericalCoords.hh"

GZ_ADD_PLUGIN(
  posim_ros_gz_plugins::SphericalCoords, gz::sim::System,
  posim_ros_gz_plugins::SphericalCoords::ISystemConfigure,
  posim_ros_gz_plugins::SphericalCoords::ISystemPostUpdate)

namespace posim_ros_gz_plugins
{

namespace
{
constexpr double invalidCoordinate = std::numeric_limits<double>::quiet_NaN();

bool ValidGeographicPosition(double latitude, double longitude, double altitude)
{
  return std::isfinite(latitude) && std::isfinite(longitude) && std::isfinite(altitude) &&
         latitude >= -90.0 && latitude <= 90.0 && longitude >= -180.0 && longitude <= 180.0;
}
}  // namespace

struct SphericalCoords::PrivateData
{
  gz::sim::EntityComponentManager * ecm = nullptr;
  gz::sim::World world;
  rclcpp::Service<posim_interfaces::srv::GetOriginSphericalCoord>::SharedPtr getOriginSrv;
  rclcpp::Service<posim_interfaces::srv::SetOriginSphericalCoord>::SharedPtr setOriginSrv;
  rclcpp::Service<posim_interfaces::srv::TransformToSphericalCoord>::SharedPtr transformToSrv;
  rclcpp::Service<posim_interfaces::srv::TransformFromSphericalCoord>::SharedPtr transformFromSrv;

  std::optional<gz::math::SphericalCoordinates> Coordinates() const
  {
    if (!ecm || !world.Valid(*ecm))
    {
      return std::nullopt;
    }
    return world.SphericalCoordinates(*ecm);
  }
};

SphericalCoords::SphericalCoords() : dataPtr(std::make_unique<PrivateData>()) {}
SphericalCoords::~SphericalCoords() = default;

void SphericalCoords::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "posim_ros_gz_plugins::SphericalCoords::Configure on entity: " << _entity << std::endl;

  this->dataPtr->world = gz::sim::World(_entity);
  if (!this->dataPtr->world.Valid(_ecm))
  {
    gzerr << "SphericalCoords must be attached to a world. ROS services are disabled." << std::endl;
    return;
  }
  const auto coords = this->dataPtr->world.SphericalCoordinates(_ecm);
  if (!coords)
  {
    gzerr << "SphericalCoords requires <spherical_coordinates> in the world. "
          << "ROS services are disabled." << std::endl;
    return;
  }
  this->dataPtr->ecm = &_ecm;

  if (!rclcpp::ok())
  {
    // Gazebo owns process signal handling for embedded ROS nodes.
    rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  }

  this->ros_node_ = std::make_shared<rclcpp::Node>("sc_node");

  gzmsg << "Spherical coordinates reference=" << std::endl
        << "\t- Latitude [degrees]=" << coords->LatitudeReference().Degree() << std::endl
        << "\t- Longitude [degrees]=" << coords->LongitudeReference().Degree() << std::endl
        << "\t- Altitude [m]=" << coords->ElevationReference() << std::endl
        << "\t- Heading [degrees] =" << coords->HeadingOffset().Degree() << std::endl;

  this->dataPtr->getOriginSrv =
    this->ros_node_->create_service<posim_interfaces::srv::GetOriginSphericalCoord>(
      "/gz/get_origin_spherical_coordinates", std::bind(
                                                &SphericalCoords::GetOriginSphericalCoord, this,
                                                std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->setOriginSrv =
    this->ros_node_->create_service<posim_interfaces::srv::SetOriginSphericalCoord>(
      "/gz/set_origin_spherical_coordinates", std::bind(
                                                &SphericalCoords::SetOriginSphericalCoord, this,
                                                std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->transformToSrv =
    this->ros_node_->create_service<posim_interfaces::srv::TransformToSphericalCoord>(
      "/gz/transform_to_spherical_coordinates", std::bind(
                                                  &SphericalCoords::TransformToSphericalCoord, this,
                                                  std::placeholders::_1, std::placeholders::_2));

  this->dataPtr->transformFromSrv =
    this->ros_node_->create_service<posim_interfaces::srv::TransformFromSphericalCoord>(
      "/gz/transform_from_spherical_coordinates",
      std::bind(
        &SphericalCoords::TransformFromSphericalCoord, this, std::placeholders::_1,
        std::placeholders::_2));
}

bool SphericalCoords::TransformFromSphericalCoord(
  const std::shared_ptr<posim_interfaces::srv::TransformFromSphericalCoord::Request> request,
  std::shared_ptr<posim_interfaces::srv::TransformFromSphericalCoord::Response> response)
{
  response->output.x = response->output.y = response->output.z = invalidCoordinate;
  const auto coords = this->dataPtr->Coordinates();
  if (
    !coords ||
    !ValidGeographicPosition(request->latitude_deg, request->longitude_deg, request->altitude))
  {
    gzerr << "Cannot transform spherical coordinates: missing origin or invalid position."
          << std::endl;
    return false;
  }
  gz::math::Angle lat, lon;
  lat.SetDegree(request->latitude_deg);
  lon.SetDegree(request->longitude_deg);
  auto scVec = gz::math::CoordinateVector3::Spherical(lat, lon, request->altitude);

  auto cartOpt = coords->LocalFromSphericalPosition(scVec);
  if (!cartOpt || !cartOpt->AsMetricVector())
  {
    return false;
  }
  const auto cartVec = *cartOpt->AsMetricVector();
  if (!cartVec.IsFinite())
  {
    return false;
  }

  response->output.x = cartVec.X();
  response->output.y = cartVec.Y();
  response->output.z = cartVec.Z();

  return true;
}

bool SphericalCoords::TransformToSphericalCoord(
  const std::shared_ptr<posim_interfaces::srv::TransformToSphericalCoord::Request> request,
  std::shared_ptr<posim_interfaces::srv::TransformToSphericalCoord::Response> response)
{
  response->latitude_deg = response->longitude_deg = response->altitude = invalidCoordinate;
  const auto coords = this->dataPtr->Coordinates();
  if (
    !coords || !std::isfinite(request->input.x) || !std::isfinite(request->input.y) ||
    !std::isfinite(request->input.z))
  {
    gzerr << "Cannot transform local coordinates: missing origin or invalid position." << std::endl;
    return false;
  }
  auto cartVec =
    gz::math::CoordinateVector3::Metric(request->input.x, request->input.y, request->input.z);

  auto scOpt = coords->SphericalFromLocalPosition(cartVec);
  if (
    !scOpt || !scOpt->Lat() || !scOpt->Lon() || !scOpt->Z() ||
    !ValidGeographicPosition(scOpt->Lat()->Degree(), scOpt->Lon()->Degree(), *scOpt->Z()))
  {
    return false;
  }
  response->latitude_deg = scOpt->Lat()->Degree();
  response->longitude_deg = scOpt->Lon()->Degree();
  response->altitude = *scOpt->Z();

  return true;
}

bool SphericalCoords::GetOriginSphericalCoord(
  const std::shared_ptr<posim_interfaces::srv::GetOriginSphericalCoord::Request> request,
  std::shared_ptr<posim_interfaces::srv::GetOriginSphericalCoord::Response> response)
{
  response->latitude_deg = response->longitude_deg = response->altitude = invalidCoordinate;
  const auto coords = this->dataPtr->Coordinates();
  if (!coords)
  {
    gzerr << "Cannot read spherical coordinates: world origin is unavailable." << std::endl;
    return false;
  }
  response->latitude_deg = coords->LatitudeReference().Degree();
  response->longitude_deg = coords->LongitudeReference().Degree();
  response->altitude = coords->ElevationReference();
  return true;
}

bool SphericalCoords::SetOriginSphericalCoord(
  const std::shared_ptr<posim_interfaces::srv::SetOriginSphericalCoord::Request> request,
  std::shared_ptr<posim_interfaces::srv::SetOriginSphericalCoord::Response> response)
{
  response->success = false;
  auto optionalSphericalCoords = this->dataPtr->Coordinates();

  if (
    !optionalSphericalCoords ||
    !ValidGeographicPosition(request->latitude_deg, request->longitude_deg, request->altitude))
  {
    gzerr << "Cannot set spherical coordinates: missing origin or invalid position." << std::endl;
    return false;
  }

  auto sphericalCoords = optionalSphericalCoords.value();

  gz::math::Angle angle;

  angle.SetDegree(request->latitude_deg);
  sphericalCoords.SetLatitudeReference(angle);

  angle.SetDegree(request->longitude_deg);
  sphericalCoords.SetLongitudeReference(angle);

  sphericalCoords.SetElevationReference(request->altitude);

  // Apply the modified spherical coordinates back to the world
  this->dataPtr->world.SetSphericalCoordinates(*this->dataPtr->ecm, sphericalCoords);

  response->success = true;

  return true;
}

void SphericalCoords::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!this->ros_node_)
  {
    return;
  }
  const auto context = this->ros_node_->get_node_base_interface()->get_context();
  if (!rclcpp::ok(context))
  {
    return;
  }
  if (!_info.paused)
  {
    try
    {
      rclcpp::spin_some(this->ros_node_);
    }
    catch (const rclcpp::exceptions::RCLError &)
    {
      // Another embedded ROS component may shut down the shared context
      // after the check above. Do not hide errors during normal operation.
      if (rclcpp::ok(context))
      {
        throw;
      }
      return;
    }

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "posim_ros_gz_plugins::SphericalCoords::PostUpdate" << std::endl;
    }
  }
}

}  // namespace posim_ros_gz_plugins
