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
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <gz/sim/components/World.hh>
#include <limits>
#include <sdf/Root.hh>
#include <sstream>
#include "CurrentProfile.hh"
#include "TidalProfile.hh"
#include "posim_gz_world_plugins/OceanCurrentWorldPlugin.hh"

using posim_gz_world_plugins::GaussMarkovProcess;
using posim_gz_world_plugins::OceanCurrentWorldPlugin;
using posim_gz_world_plugins::detail::ReadCurrentProfile;
using posim_gz_world_plugins::detail::ReadTidalProfile;

TEST(TidalProfile, ReadsAlternatingPeaksAndSkipsSlack)
{
  std::istringstream input(
    "Date,Event,Speed\n2020-01-01 00:00, flood,100\n2020-01-01 00:30,slack,-\n"
    "2020-01-01 01:00, ebb,-200\n2020-01-01 01:30,ebb,-100\n");
  const auto profile = ReadTidalProfile(input);
  ASSERT_EQ(profile.dateGMT.size(), 2u);
  EXPECT_DOUBLE_EQ(profile.speedcmsec[0], 100);
  EXPECT_DOUBLE_EQ(profile.speedcmsec[1], -200);
}

TEST(TidalProfile, RejectsMalformedRowsDatesAndSpeeds)
{
  for (const std::string & line :
       {"", "short", "2020-01-01 00:00,flood,100,extra", "2020-01-01 00:00,flood,nan",
        "2020-01-01 00:00,flood,100m", "2020-02-30 00:00,flood,100", "2020-01-01 00:00,bad,100",
        "2020-01-01 00:00,flood,100"})
  {
    std::istringstream input("Date,Event,Speed\n" + line + "\n");
    EXPECT_THROW(ReadTidalProfile(input), std::invalid_argument);
  }
  std::istringstream duplicate(
    "Date,Event,Speed\n2020-01-01 00:00,flood,100\n2020-01-01 00:00,ebb,-100\n");
  EXPECT_THROW(ReadTidalProfile(duplicate), std::invalid_argument);
}

TEST(CurrentProfile, ReadsDescriptionAndFiniteSortedLayers)
{
  std::istringstream input(
    "description\r\nmetadata\r\nnorth,east,depth\r\n"
    " 0.1 , -0.2, 0 \r\n\r\n0.2,0.1,10\r\n");
  const auto rows = ReadCurrentProfile(input);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_DOUBLE_EQ(rows[0].X(), 0.1);
  EXPECT_DOUBLE_EQ(rows[0].Y(), -0.2);
  EXPECT_DOUBLE_EQ(rows[0].Z(), 0.0);
  EXPECT_DOUBLE_EQ(rows[1].Z(), 10.0);
}

TEST(CurrentProfile, RejectsIncompleteMalformedOrUnorderedProfiles)
{
  for (const std::string & data :
       {"", "0,0,0\n", "0,0\n0,0,10\n", "0,0,0,extra\n0,0,10\n", "0,0,0,\n0,0,10\n",
        "bad,0,0\n0,0,10\n", "0.1m,0,0\n0,0,10\n", "nan,0,0\n0,0,10\n", "1e999,0,0\n0,0,10\n",
        "0,0,-1\n0,0,10\n", "0,0,10\n0,0,10\n", "0,0,10\n0,0,0\n"})
  {
    SCOPED_TRACE(data);
    std::istringstream input("description\nmetadata\ncolumns\n" + data);
    EXPECT_THROW(ReadCurrentProfile(input), std::invalid_argument);
  }
  std::istringstream missingHeader("one\ntwo\n");
  EXPECT_THROW(ReadCurrentProfile(missingHeader), std::invalid_argument);
}

TEST(CurrentModel, NonFiniteParametersDoNotChangeTheModel)
{
  GaussMarkovProcess model;
  ASSERT_TRUE(model.SetModel(0.3, 0.0, 1.0, 0.1, 0.0));
  const auto nan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(model.SetMean(nan));
  EXPECT_FALSE(model.SetMean(std::numeric_limits<double>::infinity()));
  EXPECT_FALSE(model.SetModel(nan, 0, 1, 0.1, 0));
  EXPECT_FALSE(model.SetModel(0.3, nan, 1, 0.1, 0));
  EXPECT_FALSE(model.SetModel(0.3, 0, nan, 0.1, 0));
  EXPECT_FALSE(model.SetModel(0.3, 0, 1, nan, 0));
  EXPECT_FALSE(model.SetModel(0.3, 0, 1, 0.1, nan));
  EXPECT_DOUBLE_EQ(model.mean, 0.3);
  EXPECT_DOUBLE_EQ(model.var, 0.3);
  EXPECT_DOUBLE_EQ(model.mu, 0.1);
}

TEST(CurrentModel, RepeatedTimeDoesNotSampleNoiseAndRewindResetsState)
{
  GaussMarkovProcess model;
  ASSERT_TRUE(model.SetModel(0.3, -1.0, 1.0, 0.1, 0.5));
  const auto value = model.Update(1.0);
  EXPECT_DOUBLE_EQ(model.Update(1.0), value);
  EXPECT_DOUBLE_EQ(model.Update(std::numeric_limits<double>::quiet_NaN()), value);
  EXPECT_DOUBLE_EQ(model.Update(0.0), model.mean);
  EXPECT_DOUBLE_EQ(model.lastUpdate, 0.0);
}

TEST(CurrentConfiguration, MissingRequiredBlocksLeavesWorldPluginInactive)
{
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  const auto world = ecm.CreateEntity();
  ecm.CreateComponent(world, gz::sim::components::World());
  for (const std::string & blocks :
       {"", "<namespace>test</namespace><constant_current/>",
        "<namespace>test</namespace><transient_current/>",
        "<constant_current/><transient_current/>"})
  {
    SCOPED_TRACE(blocks);
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>" + blocks +
      "</plugin></world></sdf>");
    ASSERT_TRUE(errors.empty());
    auto config = root.Element()->GetElement("world")->GetElement("plugin");
    OceanCurrentWorldPlugin plugin;
    plugin.Configure(world, config, ecm, events);
    EXPECT_EQ(OceanCurrentWorldPlugin::Instance(), nullptr);
    plugin.Update(gz::sim::UpdateInfo(), ecm);
    plugin.PostUpdate(gz::sim::UpdateInfo(), ecm);
    EXPECT_TRUE(plugin.sharedDataPtr->stratifiedDatabase.empty());
  }
}

TEST(CurrentConfiguration, InvalidTidalFilesAndStartTimesLeavePluginInactive)
{
  std::string path = (std::filesystem::temp_directory_path() / "posim-current-XXXXXX").string();
  const auto descriptor = mkstemp(path.data());
  ASSERT_GE(descriptor, 0);
  close(descriptor);
  {
    std::ofstream file(path);
    file << "description\nmetadata\ncolumns\n0.2,0,0\n0.4,0,10\n";
  }
  gz::sim::EntityComponentManager ecm;
  gz::sim::EventManager events;
  const auto world = ecm.CreateEntity();
  ecm.CreateComponent(world, gz::sim::components::World());
  const std::string directions = "<mean_direction><ebb>0</ebb><flood>180</flood></mean_direction>";
  const std::string start =
    "<world_start_time_GMT><year>2020</year><month>1</month><day>1</day>"
    "<hour>0</hour><minute>0</minute></world_start_time_GMT>";
  for (const auto & tide :
       {directions + start + "<databasefilePath>" + path + ".missing</databasefilePath>",
        directions + "<harmonic_constituents/>", start + "<harmonic_constituents/>"})
  {
    sdf::Root root;
    const auto errors = root.LoadSdfString(
      "<sdf version='1.12'><world name='test'><plugin name='test' filename='test'>"
      "<namespace>test</namespace><constant_current/><transient_current><databasefilePath>" +
      path + "</databasefilePath></transient_current><tidal_oscillation>" + tide +
      "</tidal_oscillation></plugin></world></sdf>");
    ASSERT_TRUE(errors.empty());
    OceanCurrentWorldPlugin plugin;
    plugin.Configure(world, root.Element()->GetElement("world")->GetElement("plugin"), ecm, events);
    EXPECT_EQ(plugin.sharedDataPtr->stratifiedDatabase.size(), 2u);
    EXPECT_EQ(OceanCurrentWorldPlugin::Instance(), nullptr);
    plugin.Update(gz::sim::UpdateInfo(), ecm);
    plugin.PostUpdate(gz::sim::UpdateInfo(), ecm);
  }
  std::filesystem::remove(path);
}
