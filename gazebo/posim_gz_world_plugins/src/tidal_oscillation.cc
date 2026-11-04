#include <posim_gz_world_plugins/tidal_oscillation.hh>

#include <algorithm>
#include <cmath>
#include <limits>

namespace posim_gz_world_plugins
{
TidalOscillation::TidalOscillation() = default;

void TidalOscillation::Reset()
{
  this->valid = false;
  this->datenum.clear();
  this->currentType = false;
}

bool TidalOscillation::Initiate(bool _harmonicConstituent)
{
  this->Reset();
  this->harmonicConstituent = _harmonicConstituent;
  this->worldStartTime_num = TranslateDate(this->worldStartTime);
  if (
    !std::isfinite(this->worldStartTime_num) || !std::isfinite(this->ebbDirection) ||
    !std::isfinite(this->floodDirection))
  {
    return false;
  }
  if (_harmonicConstituent)
  {
    for (const auto value :
         {M2_amp, M2_phase, M2_speed, S2_amp, S2_phase, S2_speed, N2_amp, N2_phase, N2_speed})
    {
      if (!std::isfinite(value))
      {
        return false;
      }
    }
  }
  else
  {
    if (this->dateGMT.size() < 2 || this->dateGMT.size() != this->speedcmsec.size())
    {
      return false;
    }
    for (size_t i = 0; i < this->dateGMT.size(); ++i)
    {
      const auto date = TranslateDate(this->dateGMT[i]);
      if (
        !std::isfinite(date) || !std::isfinite(this->speedcmsec[i]) ||
        (!this->datenum.empty() && date <= this->datenum.back()))
      {
        this->datenum.clear();
        return false;
      }
      this->datenum.push_back(date);
    }
  }
  this->valid = true;
  return true;
}

double TidalOscillation::TranslateDate(std::array<int, 5> _datetime)
{
  const auto [year, month, day, hour, minute] = _datetime;
  constexpr int cumulativeDays[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  constexpr int monthDays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  if (
    year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 ||
    day > monthDays[month] + (month == 2 && leap) || hour < 0 || hour > 23 || minute < 0 ||
    minute > 59)
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  // Preserve the serial-day epoch used by the harmonic constituent inputs.
  const int previousYear = year - 1;
  const int days = 365 * previousYear + previousYear / 4 - previousYear / 100 + previousYear / 400 +
                   cumulativeDays[month] + day + 366 + (month > 2 && leap);
  return days + (hour * 60 + minute) / 1440.0;
}

std::pair<double, double> TidalOscillation::Update(double _time, double _currentDepthRatio)
{
  if (!this->valid || !std::isfinite(_time) || !std::isfinite(_currentDepthRatio))
  {
    return {0, 0};
  }
  const double currentTime = this->worldStartTime_num + _time / 86400.0;
  double currentVelocity = 0;
  if (this->harmonicConstituent)
  {
    const auto constituent = [currentTime](double amplitude, double phase, double speed)
    {
      return amplitude *
             std::cos(
               speed * M_PI / 180.0 * currentTime * 24.0 + phase * M_PI / 180.0 - M_PI / 2.0);
    };
    currentVelocity = constituent(M2_amp, M2_phase, M2_speed) +
                      constituent(S2_amp, S2_phase, S2_speed) +
                      constituent(N2_amp, N2_phase, N2_speed);
  }
  else
  {
    if (this->datenum.size() < 2 || this->datenum.size() != this->speedcmsec.size())
    {
      return {0, 0};
    }
    if (currentTime <= this->datenum.front())
    {
      currentVelocity = this->speedcmsec.front();
    }
    else if (currentTime >= this->datenum.back())
    {
      currentVelocity = this->speedcmsec.back();
    }
    else
    {
      const auto upper = std::upper_bound(this->datenum.begin(), this->datenum.end(), currentTime);
      const auto index = static_cast<size_t>(upper - this->datenum.begin());
      const auto ratio = (currentTime - this->datenum[index - 1]) /
                         (this->datenum[index] - this->datenum[index - 1]);
      currentVelocity = this->speedcmsec[index - 1] * (1 - ratio) + this->speedcmsec[index] * ratio;
    }
    currentVelocity /= 100.0;
  }
  const auto direction = currentVelocity > 0 ? this->floodDirection : this->ebbDirection;
  const auto magnitude = currentVelocity * _currentDepthRatio;
  if (!std::isfinite(magnitude))
  {
    return {0, 0};
  }
  this->currentType = currentVelocity > 0;
  return {
    -std::cos(direction * M_PI / 180.0) * magnitude,
    -std::sin(direction * M_PI / 180.0) * magnitude};
}
}  // namespace posim_gz_world_plugins
