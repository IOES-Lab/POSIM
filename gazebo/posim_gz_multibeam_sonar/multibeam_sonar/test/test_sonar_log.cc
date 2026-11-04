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
#include <fstream>
#include <future>
#include <sstream>
#include "SonarLog.hh"

namespace fs = std::filesystem;

TEST(SonarCsv, PreservesRealValuesWithZeroImaginaryParts)
{
  std::ostringstream output;
  posim::sonar::WriteComplexCsvValue(output, std::complex<double>(1.25, 0.0));
  output << ',';
  posim::sonar::WriteComplexCsvValue(output, std::complex<double>(-1.25, -0.0));
  output << ',';
  posim::sonar::WriteComplexCsvValue(output, std::complex<double>(1.0, -2.0));
  output << ',';
  posim::sonar::WriteComplexCsvValue(output, std::complex<double>(-1.0, 2.0));
  EXPECT_EQ(output.str(), "1.25+0i,-1.25-0i,1-2i,-1+2i");
}

class SonarLog : public ::testing::Test
{
protected:
  fs::path parent = posim::sonar::CreateLogDirectory(fs::temp_directory_path());
  void TearDown() override { fs::remove_all(parent); }
};

TEST_F(SonarLog, RepeatedAndConcurrentRunsPreservePreviousData)
{
  const auto oldLog = parent / "SonarRawData_000001.csv";
  std::ofstream(oldLog) << "previous experiment";
  auto pending =
    std::async(std::launch::async, [&] { return posim::sonar::CreateLogDirectory(parent); });
  const auto first = posim::sonar::CreateLogDirectory(parent);
  const auto second = pending.get();
  ASSERT_NE(first, second);
  EXPECT_TRUE(fs::is_directory(first));
  EXPECT_TRUE(fs::is_directory(second));
  std::ofstream(first / "SonarRawData_000001.csv") << "first run";
  std::ofstream(second / "SonarRawData_000001.csv") << "second run";
  std::ifstream input(oldLog);
  std::string previous;
  std::getline(input, previous);
  EXPECT_EQ(previous, "previous experiment");
}

TEST_F(SonarLog, InvalidDestinationDoesNotModifyFiles)
{
  const auto file = parent / "existing-file";
  std::ofstream(file) << "keep";
  EXPECT_THROW(posim::sonar::CreateLogDirectory(file), fs::filesystem_error);
  EXPECT_EQ(fs::file_size(file), 4);
}
