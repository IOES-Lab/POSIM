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

#ifndef POSIM_GZ_SENSOR_PLUGINS__USBL_CONFIG_HH_
#define POSIM_GZ_SENSOR_PLUGINS__USBL_CONFIG_HH_

#include <algorithm>
#include <charconv>
#include <cmath>
#include <gz/common/Console.hh>
#include <sdf/Element.hh>
#include <set>
#include <string>
#include <vector>

namespace posim_gz_sensor_plugins::detail
{
inline bool ParseUsblId(const std::string & text, int & id)
{
  if (
    text.empty() || (text.size() > 1 && text.front() == '0') ||
    !std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; }))
  {
    return false;
  }
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), id);
  return parsed.ec == std::errc() && parsed.ptr == text.data() + text.size();
}

inline std::vector<std::string> UsblList(const std::string & text)
{
  std::vector<std::string> result;
  size_t start = 0;
  do
  {
    const auto end = text.find(',', start);
    auto value = text.substr(start, end - start);
    const auto first = value.find_first_not_of(" \t\r\n");
    const auto last = value.find_last_not_of(" \t\r\n");
    result.push_back(first == std::string::npos ? "" : value.substr(first, last - first + 1));
    if (end == std::string::npos)
    {
      break;
    }
    start = end + 1;
  } while (true);
  return result;
}

inline bool UsblTopicName(const std::string & text, bool allowPath = false)
{
  bool first = true;
  for (char c : text)
  {
    const bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    if (allowPath && c == '/' && !first)
    {
      first = true;
      continue;
    }
    if (!letter && (first || c < '0' || c > '9'))
    {
      return false;
    }
    first = false;
  }
  return !first;
}

inline bool ValidateUsblConfig(const std::shared_ptr<const sdf::Element> & sdf, bool transceiver)
{
  if (!sdf)
  {
    return false;
  }
  for (const auto * key :
       {"namespace", "transceiver_device", "transceiver_ID", "transponder_device",
        "transponder_ID"})
  {
    if (!sdf->HasElement(key) || sdf->Get<std::string>(key).empty())
    {
      gzerr << "USBL requires a nonempty <" << key << ">." << std::endl;
      return false;
    }
  }
  int id;
  if (
    !UsblTopicName(sdf->Get<std::string>("namespace"), true) ||
    !UsblTopicName(sdf->Get<std::string>("transceiver_device")) ||
    !UsblTopicName(sdf->Get<std::string>("transponder_device")) ||
    !ParseUsblId(sdf->Get<std::string>("transceiver_ID"), id))
  {
    gzerr << "USBL requires valid topic names and decimal int32 device IDs." << std::endl;
    return false;
  }
  const double speed = sdf->Get<double>("sound_speed", 1540.0).first;
  if (!std::isfinite(speed) || speed <= 0.0)
  {
    gzerr << "USBL sound_speed must be finite and positive." << std::endl;
    return false;
  }
  const auto ids = UsblList(sdf->Get<std::string>("transponder_ID"));
  std::set<int> unique;
  for (const auto & text : ids)
  {
    if (!ParseUsblId(text, id) || !unique.insert(id).second)
    {
      gzerr << "USBL transponder IDs must be unique nonnegative decimal int32 values." << std::endl;
      return false;
    }
  }
  if (transceiver)
  {
    if (
      !sdf->HasElement("enable_ping_scheduler") || !sdf->HasElement("transponder_attached_object"))
    {
      gzerr << "USBL requires enable_ping_scheduler and transponder_attached_object." << std::endl;
      return false;
    }
    const auto objects = UsblList(sdf->Get<std::string>("transponder_attached_object"));
    if (
      objects.size() != ids.size() ||
      std::any_of(objects.begin(), objects.end(), [](const auto & name) { return name.empty(); }))
    {
      gzerr << "USBL requires one nonempty attached model name per transponder ID." << std::endl;
      return false;
    }
    const double frequency = sdf->Get<double>("ping_frequency", 1.0).first;
    if (
      sdf->Get<bool>("enable_ping_scheduler") &&
      (!std::isfinite(frequency) || frequency <= 0.0 || !std::isfinite(1.0 / frequency)))
    {
      gzerr << "USBL ping_frequency must be finite and positive." << std::endl;
      return false;
    }
  }
  else
  {
    const double mean = sdf->Get<double>("mu", 0.0).first;
    const double sigma = sdf->Get<double>("sigma", 1.0).first;
    if (
      ids.size() != 1 || !sdf->HasElement("transceiver_model") ||
      sdf->Get<std::string>("transceiver_model").empty() || !std::isfinite(mean) ||
      !std::isfinite(sigma) || sigma < 0.0)
    {
      gzerr << "USBL transponder requires one ID, a transceiver model, finite mu and sigma >= 0."
            << std::endl;
      return false;
    }
  }
  return true;
}
}  // namespace posim_gz_sensor_plugins::detail
#endif  // POSIM_GZ_SENSOR_PLUGINS__USBL_CONFIG_HH_
