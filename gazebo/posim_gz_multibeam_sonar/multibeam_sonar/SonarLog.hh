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

#ifndef POSIM_SONAR_LOG_HH_
#define POSIM_SONAR_LOG_HH_

#include <chrono>
#include <cmath>
#include <complex>
#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>

namespace posim::sonar
{
template <typename T>
inline void WriteComplexCsvValue(std::ostream & output, const std::complex<T> & value)
{
  output << value.real();
  // A positive zero still needs a separator: 1+0i must not become 10i.
  if (!std::signbit(value.imag()))
  {
    output << '+';
  }
  output << value.imag() << 'i';
}

// Reserve a directory atomically so sensors and repeated runs never share logs.
inline std::filesystem::path CreateLogDirectory(const std::filesystem::path & parent)
{
  const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
  for (unsigned int attempt = 0; attempt < 1000; ++attempt)
  {
    const auto directory =
      parent / ("SonarRawData_" + std::to_string(stamp) + "_" + std::to_string(attempt));
    std::error_code error;
    if (std::filesystem::create_directory(directory, error))
    {
      return directory;
    }
    if (error && error != std::errc::file_exists)
    {
      throw std::filesystem::filesystem_error(
        "Cannot create sonar log directory", directory, error);
    }
  }
  throw std::runtime_error("Cannot reserve a unique sonar log directory");
}
}  // namespace posim::sonar

#endif
