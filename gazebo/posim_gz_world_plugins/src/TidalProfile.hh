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

#ifndef POSIM_GZ_WORLD_PLUGINS__TIDAL_PROFILE_HH_
#define POSIM_GZ_WORLD_PLUGINS__TIDAL_PROFILE_HH_

#include <cmath>
#include <iomanip>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "posim_gz_world_plugins/tidal_oscillation.hh"

namespace posim_gz_world_plugins::detail
{
inline TidalOscillation ReadTidalProfile(std::istream & input)
{
  TidalOscillation result;
  std::string line;
  if (!std::getline(input, line))
  {
    throw std::invalid_argument("missing tidal CSV header");
  }
  double previousDate = 0;
  while (std::getline(input, line))
  {
    if (line.find_first_not_of(" \t\r") == std::string::npos)
    {
      continue;
    }
    std::istringstream row(line);
    std::array<std::string, 3> fields;
    for (auto & field : fields)
    {
      if (!std::getline(row, field, ','))
      {
        throw std::invalid_argument("expected three tidal CSV columns");
      }
    }
    if (row.peek() != std::char_traits<char>::eof() || line.back() == ',')
    {
      throw std::invalid_argument("extra tidal CSV column");
    }
    std::tm date{};
    std::istringstream datetime(fields[0]);
    if (!(datetime >> std::get_time(&date, "%Y-%m-%d %H:%M")) || !(datetime >> std::ws).eof())
    {
      throw std::invalid_argument("expected tidal date YYYY-MM-DD HH:MM");
    }
    const std::array<int, 5> parts = {
      date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min};
    const auto serial = result.TranslateDate(parts);
    if (!std::isfinite(serial) || serial <= previousDate)
    {
      throw std::invalid_argument("tidal dates must be valid and strictly increasing");
    }
    previousDate = serial;
    std::string event;
    std::istringstream category(fields[1]);
    category >> event;
    if (!(category >> std::ws).eof() || (event != "slack" && event != "flood" && event != "ebb"))
    {
      throw std::invalid_argument("tidal event must be slack, flood or ebb");
    }
    if (event == "slack")
    {
      continue;
    }
    double speed;
    std::istringstream value(fields[2]);
    if (!(value >> speed) || !std::isfinite(speed) || !(value >> std::ws).eof())
    {
      throw std::invalid_argument("tidal speed must be finite cm/s");
    }
    // Preserve the peak-to-peak interpolation of alternating ebb/flood events.
    if (
      !result.speedcmsec.empty() && ((speed > 0) - (speed < 0)) == ((result.speedcmsec.back() > 0) -
                                                                    (result.speedcmsec.back() < 0)))
    {
      continue;
    }
    result.dateGMT.push_back(parts);
    result.speedcmsec.push_back(speed);
  }
  if (result.dateGMT.size() < 2)
  {
    throw std::invalid_argument("at least two alternating tidal events are required");
  }
  return result;
}
}  // namespace posim_gz_world_plugins::detail

#endif  // POSIM_GZ_WORLD_PLUGINS__TIDAL_PROFILE_HH_
