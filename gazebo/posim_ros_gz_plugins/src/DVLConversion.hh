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

#ifndef POSIM_ROS_GZ_PLUGINS__DVL_CONVERSION_HH_
#define POSIM_ROS_GZ_PLUGINS__DVL_CONVERSION_HH_

#include <gz/msgs/dvl_velocity_tracking.pb.h>
#include "posim_interfaces/msg/dvl.hpp"

namespace posim_ros_gz_plugins::detail
{
posim_interfaces::msg::DVL ConvertDVL(const gz::msgs::DVLVelocityTracking & message);
}

#endif  // POSIM_ROS_GZ_PLUGINS__DVL_CONVERSION_HH_
