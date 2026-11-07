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
#include "DVLConversion.hh"

using posim_ros_gz_plugins::detail::ConvertDVL;

TEST(DVLConversion, MapsLinearCovarianceIntoSixDimensionalTwist)
{
  gz::msgs::DVLVelocityTracking input;
  auto * beam = input.add_beams();
  for (int i = 0; i < 9; ++i)
  {
    input.mutable_velocity()->add_covariance(i + 1);
    beam->mutable_velocity()->add_covariance(i + 11);
  }
  const auto output = ConvertDVL(input);
  ASSERT_EQ(output.beams.size(), 1u);
  for (int row = 0; row < 6; ++row)
  {
    for (int col = 0; col < 6; ++col)
    {
      EXPECT_DOUBLE_EQ(
        output.velocity.covariance[row * 6 + col], row < 3 && col < 3 ? row * 3 + col + 1 : 0);
      EXPECT_DOUBLE_EQ(
        output.beams[0].velocity.covariance[row * 6 + col],
        row < 3 && col < 3 ? row * 3 + col + 11 : 0);
    }
  }
}

TEST(DVLConversion, BoundsMalformedCovarianceAndKeepsUnmeasuredEntriesZero)
{
  gz::msgs::DVLVelocityTracking input;
  auto * beam = input.add_beams();
  beam->mutable_velocity()->add_covariance(0.5);
  for (int i = 0; i < 100; ++i)
  {
    input.mutable_velocity()->add_covariance(i + 1);
  }
  const auto output = ConvertDVL(input);
  EXPECT_DOUBLE_EQ(output.velocity.covariance[14], 9);
  EXPECT_DOUBLE_EQ(output.velocity.covariance[15], 0);
  EXPECT_DOUBLE_EQ(output.velocity.covariance[35], 0);
  ASSERT_EQ(output.beams.size(), 1u);
  EXPECT_DOUBLE_EQ(output.beams[0].velocity.covariance[0], 0.5);
  for (size_t i = 1; i < output.beams[0].velocity.covariance.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(output.beams[0].velocity.covariance[i], 0);
  }
}

TEST(DVLConversion, PreservesMeasurementsAndNormalizesGazeboFrameSeparators)
{
  gz::msgs::DVLVelocityTracking input;
  input.mutable_header()->mutable_stamp()->set_sec(12);
  input.mutable_header()->mutable_stamp()->set_nsec(345);
  auto * frame = input.mutable_header()->add_data();
  frame->set_key("frame_id");
  frame->add_value("robot::sensor::dvl");
  input.set_type(gz::msgs::DVLVelocityTracking::DVL_TYPE_PHASED_ARRAY);
  input.mutable_target()->set_type(gz::msgs::DVLTrackingTarget::DVL_TARGET_BOTTOM);
  input.mutable_target()->mutable_range()->set_mean(15.0);
  input.mutable_velocity()->mutable_mean()->set_x(1.5);
  input.mutable_velocity()->mutable_mean()->set_y(-2.0);
  input.mutable_velocity()->mutable_mean()->set_z(0.25);
  auto * beam = input.add_beams();
  beam->set_id(3);
  beam->set_locked(true);
  beam->mutable_range()->set_mean(16.0);
  beam->mutable_velocity()->set_reference(gz::msgs::DVLKinematicEstimate::DVL_REFERENCE_SHIP);
  beam->mutable_velocity()->mutable_mean()->set_x(1.0);
  const auto output = ConvertDVL(input);
  EXPECT_EQ(output.header.stamp.sec, 12);
  EXPECT_EQ(output.header.stamp.nanosec, 345u);
  EXPECT_EQ(output.header.frame_id, "robot/sensor/dvl");
  EXPECT_EQ(output.type, "DVL_TYPE_PHASED_ARRAY");
  EXPECT_EQ(output.target.type, "DVL_TARGET_BOTTOM");
  EXPECT_DOUBLE_EQ(output.target.range, 15.0);
  EXPECT_DOUBLE_EQ(output.velocity.twist.linear.x, 1.5);
  EXPECT_DOUBLE_EQ(output.velocity.twist.linear.y, -2.0);
  EXPECT_DOUBLE_EQ(output.velocity.twist.linear.z, 0.25);
  ASSERT_EQ(output.beams.size(), 1u);
  EXPECT_EQ(output.beams[0].id, 3);
  EXPECT_TRUE(output.beams[0].locked);
  EXPECT_EQ(output.beams[0].reference, "DVL_REFERENCE_SHIP");
  EXPECT_DOUBLE_EQ(output.beams[0].range, 16.0);
  EXPECT_DOUBLE_EQ(output.beams[0].velocity.twist.linear.x, 1.0);
}
