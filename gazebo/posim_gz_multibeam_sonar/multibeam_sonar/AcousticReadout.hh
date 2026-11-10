// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gz::sensors
{

// Geometry cells and receiver directions are independent. Areas are solid
// angles (steradians), not material reflectivity or a display-gain correction.
struct AcousticReadout
{
  int geometryColumns{0};
  int geometryRows{0};
  std::vector<int> elevationIndices;
  std::vector<float> angularAreas;  // selected elevation row, geometry column
  std::vector<float> receiveAngles;
  std::vector<float> pressureResponse;  // receiver, geometry column
  float powerBeamwidth{0};              // full half-power width, radians
};

inline std::vector<float> AngularCenters(double minimum, double maximum, int count)
{
  if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum >= maximum || count < 1)
  {
    throw std::invalid_argument("invalid acoustic angular grid");
  }
  std::vector<float> result(count);
  for (int i = 0; i < count; ++i)
  {
    result[i] =
      count == 1 ? (minimum + maximum) / 2 : minimum + (maximum - minimum) * i / (count - 1);
  }
  return result;
}

struct AngularCells
{
  std::vector<int> indices;
  std::vector<double> measures;
};

inline AngularCells MakeAngularCells(
  const std::vector<float> & centers, double minimum, double maximum, int stride, bool elevation)
{
  if (
    centers.empty() || stride < 1 || !std::isfinite(minimum) || !std::isfinite(maximum) ||
    minimum >= maximum || (elevation && (minimum < -M_PI / 2 || maximum > M_PI / 2)))
  {
    throw std::invalid_argument("invalid acoustic cell domain");
  }
  for (size_t i = 0; i < centers.size(); ++i)
  {
    if (
      !std::isfinite(centers[i]) || centers[i] < minimum - 1e-6 || centers[i] > maximum + 1e-6 ||
      (i && centers[i] <= centers[i - 1]))
    {
      throw std::invalid_argument("acoustic centers must increase inside their domain");
    }
  }
  AngularCells cells;
  for (size_t i = 0; i < centers.size(); i += static_cast<size_t>(stride))
  {
    cells.indices.push_back(static_cast<int>(i));
  }
  if (cells.indices.back() != static_cast<int>(centers.size() - 1))
  {
    cells.indices.push_back(static_cast<int>(centers.size() - 1));
  }
  for (size_t i = 0; i < cells.indices.size(); ++i)
  {
    const double low =
      i == 0 ? minimum : (double(centers[cells.indices[i - 1]]) + centers[cells.indices[i]]) / 2;
    const double high = i + 1 == cells.indices.size()
                          ? maximum
                          : (double(centers[cells.indices[i]]) + centers[cells.indices[i + 1]]) / 2;
    cells.measures.push_back(elevation ? std::sin(high) - std::sin(low) : high - low);
  }
  return cells;
}

inline AcousticReadout MakeAcousticReadout(
  const std::vector<float> & azimuth, const std::vector<float> & elevation, double azimuthMinimum,
  double azimuthMaximum, double elevationMinimum, double elevationMaximum, int elevationStride,
  std::vector<float> receiveAngles, double powerBeamwidth)
{
  if (
    receiveAngles.empty() || !std::isfinite(powerBeamwidth) || powerBeamwidth <= 0 ||
    powerBeamwidth >= M_PI)
  {
    throw std::invalid_argument("a physical receive beamwidth is required in radians");
  }
  const auto horizontal = MakeAngularCells(azimuth, azimuthMinimum, azimuthMaximum, 1, false);
  const auto vertical =
    MakeAngularCells(elevation, elevationMinimum, elevationMaximum, elevationStride, true);
  AcousticReadout result;
  result.geometryColumns = static_cast<int>(azimuth.size());
  result.geometryRows = static_cast<int>(elevation.size());
  result.elevationIndices = vertical.indices;
  result.receiveAngles = std::move(receiveAngles);
  result.powerBeamwidth = static_cast<float>(powerBeamwidth);
  for (double height : vertical.measures)
  {
    for (double width : horizontal.measures)
    {
      result.angularAreas.push_back(static_cast<float>(width * height));
    }
  }
  // sin(x)/x has half power at x=1.39155737825151. This is a signed,
  // shifted-sinc pressure model with unit on-axis gain, not a hardware calibration.
  const double aperture = 1.39155737825151 / std::sin(powerBeamwidth / 2);
  for (float receiver : result.receiveAngles)
  {
    if (!std::isfinite(receiver))
    {
      throw std::invalid_argument("nonfinite receive angle");
    }
    for (float sample : azimuth)
    {
      const double phase = aperture * std::sin(double(sample) - receiver);
      result.pressureResponse.push_back(
        std::abs(phase) < 1e-12 ? 1.f : static_cast<float>(std::sin(phase) / phase));
    }
  }
  return result;
}

}  // namespace gz::sensors
