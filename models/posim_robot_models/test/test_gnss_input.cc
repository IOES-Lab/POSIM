// SPDX-License-Identifier: Apache-2.0
#include <gtest/gtest.h>
#include "GnssInput.hh"
namespace
{
posim::GnssInput::Packet Dry()
{
  return {1, 0, 10.25, 3, 35.1, 129.2, 42, 2, 3, 4, .49, 1.44, .01, .2, 0};
}
TEST(GnssInput, ConvertsUnitsAxesAndDatum)
{
  posim::GnssInput input;
  ASSERT_TRUE(input.Receive(Dry()));
  auto message = input.Message(10.5, 1, 30);
  EXPECT_EQ(message.fix_type, 3);
  EXPECT_EQ(message.lat, 351000000);
  EXPECT_EQ(message.lon, 1292000000);
  EXPECT_FLOAT_EQ(message.alt, 12);
  EXPECT_FLOAT_EQ(message.vn, 3);
  EXPECT_FLOAT_EQ(message.ve, 2);
  EXPECT_FLOAT_EQ(message.vd, -4);
  EXPECT_FLOAT_EQ(message.horiz_accuracy, .7);
  EXPECT_FLOAT_EQ(message.vert_accuracy, 1.2);
  EXPECT_FLOAT_EQ(message.speed_accuracy, .1);
  EXPECT_EQ(message.header.stamp.sec, 10);
  EXPECT_EQ(message.header.stamp.nanosec, 250000000u);
  EXPECT_EQ(message.ignore_flags, 6);
  EXPECT_EQ(message.time_week, 0);
  EXPECT_EQ(message.satellites_visible, 255);
}
TEST(GnssInput, RejectsFutureStaleWetAndInvalidFixes)
{
  posim::GnssInput input;
  EXPECT_EQ(input.Message(10.5, 1, 0).fix_type, 1);
  input.Receive(Dry());
  EXPECT_EQ(input.Message(10.2, 1, 0).fix_type, 1);
  EXPECT_EQ(input.Message(11.26, 1, 0).fix_type, 1);
  for (int state : {0, 1, 2, 4, 5})
  {
    auto packet = Dry();
    packet[1] = state + 1;
    packet[3] = state;
    for (int i = 4; i <= 9; ++i)
    {
      packet[i] = std::numeric_limits<double>::quiet_NaN();
    }
    ASSERT_TRUE(input.Receive(packet));
    auto message = input.Message(10.5, 1, 0);
    EXPECT_EQ(message.fix_type, 1);
    EXPECT_EQ(message.ignore_flags, 255);
    EXPECT_EQ(message.lat, 0);
    EXPECT_EQ(message.satellites_visible, 0);
  }
}
TEST(GnssInput, ResetAndOutOfOrderPackets)
{
  posim::GnssInput input;
  input.Receive(Dry());
  auto old = Dry();
  old[2] = 9;
  EXPECT_FALSE(input.Receive(old));
  EXPECT_EQ(input.Message(10.5, 1, 0).fix_type, 3);
  auto reset = Dry();
  reset[1] = 1;
  reset[2] = .2;
  reset[3] = 2;
  ASSERT_TRUE(input.Receive(reset));
  EXPECT_EQ(input.Message(.5, 1, 0).fix_type, 1);
  EXPECT_FALSE(input.Receive(Dry()));
  reset[2] = 3.4;
  reset[3] = 3;
  ASSERT_TRUE(input.Receive(reset));
  EXPECT_EQ(input.Message(3.6, 1, 0).fix_type, 3);
  input.Invalidate();
  EXPECT_EQ(input.Message(3.7, 1, 0).fix_type, 1);
}
TEST(GnssInput, MalformedValuesNeverBecomeValidGps)
{
  for (const auto & change :
       {std::pair<size_t, double>{0, 2},
        {1, .5},
        {2, -1},
        {3, 3.5},
        {4, 91},
        {5, 181},
        {6, 1e30},
        {7, 1e300},
        {10, -1},
        {12, std::numeric_limits<double>::infinity()},
        {13, 0}})
  {
    posim::GnssInput input;
    auto packet = Dry();
    packet[change.first] = change.second;
    input.Receive(packet);
    EXPECT_EQ(input.Message(10.5, 1, 0).fix_type, 1) << change.first;
  }
}
}  // namespace
