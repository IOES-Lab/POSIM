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
#include <cmath>
#include <limits>
#include "posim_gz_world_plugins/tidal_oscillation.hh"

using posim_gz_world_plugins::TidalOscillation;

namespace
{
TidalOscillation Profile()
{
  TidalOscillation tide;
  tide.worldStartTime = {2020, 1, 1, 0, 0};
  tide.dateGMT = {{2020, 1, 1, 0, 0}, {2020, 1, 1, 1, 0}};
  tide.speedcmsec = {100, 300};
  tide.ebbDirection = 0;
  tide.floodDirection = 0;
  tide.Initiate(false);
  return tide;
}
}  // namespace

TEST(TidalOscillation, InterpolatesSecondsAndClampsAtBothEnds)
{
  auto tide = Profile();
  EXPECT_NEAR(tide.Update(1800, 0.5).first, -1, 1e-8);
  EXPECT_NEAR(tide.Update(0, 1).first, -1, 1e-8);
  EXPECT_NEAR(tide.Update(-60, 1).first, -1, 1e-8);
  EXPECT_NEAR(tide.Update(3600, 1).first, -3, 1e-8);
  EXPECT_NEAR(tide.Update(7200, 1).first, -3, 1e-8);
}

TEST(TidalOscillation, ReloadsChangedDatesEvenWhenRowCountMatches)
{
  auto tide = Profile();
  tide.dateGMT[1] = {2020, 1, 1, 2, 0};
  tide.Initiate(false);
  EXPECT_NEAR(tide.Update(3600, 1).first, -2, 1e-8);
}

TEST(TidalOscillation, RejectsInvalidCalendarDates)
{
  TidalOscillation tide;
  EXPECT_DOUBLE_EQ(tide.TranslateDate({1970, 1, 1, 0, 0}), 719529);
  EXPECT_DOUBLE_EQ(
    tide.TranslateDate({2020, 3, 1, 0, 0}) - tide.TranslateDate({2020, 2, 28, 0, 0}), 2);
  for (const std::array<int, 5> & date :
       {std::array<int, 5>{2021, 2, 29, 0, 0},
        {2020, 0, 1, 0, 0},
        {2020, 13, 1, 0, 0},
        {2020, 1, 0, 0, 0},
        {2020, 1, 1, 24, 0},
        {2020, 1, 1, 0, 60}})
  {
    EXPECT_TRUE(std::isnan(tide.TranslateDate(date)));
  }
}

TEST(TidalOscillation, MalformedProfilesReturnNoCurrent)
{
  for (int variant = 0; variant < 5; ++variant)
  {
    auto tide = Profile();
    if (variant == 0)
    {
      tide.dateGMT.clear();
    }
    if (variant == 1)
    {
      tide.speedcmsec.clear();
    }
    if (variant == 2)
    {
      tide.dateGMT[1] = tide.dateGMT[0];
    }
    if (variant == 3)
    {
      tide.speedcmsec[1] = std::numeric_limits<double>::quiet_NaN();
    }
    if (variant == 4)
    {
      tide.worldStartTime[1] = 13;
    }
    tide.Initiate(false);
    EXPECT_EQ(tide.Update(1800, 1), std::make_pair(0.0, 0.0));
  }
}

TEST(TidalOscillation, InvalidRuntimeValuesAndResetReturnNoCurrent)
{
  auto tide = Profile();
  EXPECT_EQ(tide.Update(std::numeric_limits<double>::quiet_NaN(), 1), std::make_pair(0.0, 0.0));
  EXPECT_EQ(tide.Update(1, std::numeric_limits<double>::infinity()), std::make_pair(0.0, 0.0));
  tide.Reset();
  EXPECT_EQ(tide.Update(1, 1), std::make_pair(0.0, 0.0));
}
