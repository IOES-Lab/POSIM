// Copyright 2026 Kwon Yeseol
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
#include <array>
#include <limits>
#include "posim_gz_world_plugins/gauss_markov_process.hh"
using posim_gz_world_plugins::GaussMarkovProcess;
TEST(CurrentModel, RejectsNonfiniteMeanWithoutMutation)
{
  for (auto bad :
       {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()})
  {
    GaussMarkovProcess m;
    ASSERT_TRUE(m.SetModel(.3, -1, 1, 0, 0));
    EXPECT_FALSE(m.SetMean(bad));
    EXPECT_EQ(m.mean, .3);
    EXPECT_EQ(m.Update(1), .3);
  }
}
TEST(CurrentModel, RejectsNonfiniteParametersAtomically)
{
  for (int i = 0; i < 5; ++i)
  {
    for (auto bad :
         {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
          -std::numeric_limits<double>::infinity()})
    {
      GaussMarkovProcess m;
      ASSERT_TRUE(m.SetModel(.3, -1, 1, 0, 0));
      std::array<double, 5> p{.5, -1., 1., .1, .1};
      p[i] = bad;
      EXPECT_FALSE(m.SetModel(p[0], p[1], p[2], p[3], p[4])) << "parameter=" << i;
      EXPECT_EQ(m.mean, .3);
      EXPECT_EQ(m.min, -1);
      EXPECT_EQ(m.max, 1);
      EXPECT_EQ(m.mu, 0);
      EXPECT_EQ(m.noiseAmp, 0);
    }
  }
}
TEST(CurrentModel, ValidBoundaryValuesRetainBehavior)
{
  GaussMarkovProcess m;
  ASSERT_TRUE(m.SetModel(1, -1, 1, 0, 0));
  EXPECT_EQ(m.Update(1), 1);
  ASSERT_TRUE(m.SetMean(-1));
  EXPECT_EQ(m.Update(2), -1);
  EXPECT_FALSE(m.SetModel(0, 1, 1, 0, 0));
  EXPECT_EQ(m.mean, -1);
  EXPECT_FALSE(m.SetModel(0, -1, 1, 0, -1));
  EXPECT_EQ(m.mean, -1);
}
