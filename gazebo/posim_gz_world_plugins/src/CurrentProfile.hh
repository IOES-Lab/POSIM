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

#ifndef POSIM_GZ_WORLD_PLUGINS__CURRENT_PROFILE_HH_
#define POSIM_GZ_WORLD_PLUGINS__CURRENT_PROFILE_HH_

#include <cmath>
#include <gz/math/Vector3.hh>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace posim_gz_world_plugins::detail
{
inline std::vector<gz::math::Vector3d> ReadCurrentProfile(std::istream & input)
{
  std::string line;
  for (int i = 0; i < 3; ++i)
  {
    if (!std::getline(input, line))
    {
      throw std::invalid_argument("expected three description/header rows");
    }
  }
  std::vector<gz::math::Vector3d> rows;
  size_t lineNumber = 3;
  while (std::getline(input, line))
  {
    ++lineNumber;
    if (line.find_first_not_of(" \t\r") == std::string::npos)
    {
      continue;
    }
    std::istringstream record(line);
    gz::math::Vector3d row;
    for (int column = 0; column < 3; ++column)
    {
      std::string field;
      if (!std::getline(record, field, ','))
      {
        throw std::invalid_argument("expected three columns on row " + std::to_string(lineNumber));
      }
      std::istringstream value(field);
      double number;
      if (!(value >> number) || !std::isfinite(number) || !(value >> std::ws).eof())
      {
        throw std::invalid_argument("invalid number on row " + std::to_string(lineNumber));
      }
      row[column] = number;
    }
    if (record.peek() != std::char_traits<char>::eof() || line.back() == ',')
    {
      throw std::invalid_argument("extra column on row " + std::to_string(lineNumber));
    }
    if (row.Z() < 0 || (!rows.empty() && row.Z() <= rows.back().Z()))
    {
      throw std::invalid_argument("depths must be nonnegative and strictly increasing");
    }
    rows.push_back(row);
  }
  if (rows.size() < 2)
  {
    throw std::invalid_argument("at least two depth layers are required");
  }
  return rows;
}
}  // namespace posim_gz_world_plugins::detail

#endif  // POSIM_GZ_WORLD_PLUGINS__CURRENT_PROFILE_HH_
