// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <limits>
#include <opencv2/core.hpp>
#include "AcousticReadout.hh"
#include "RaySurfaceNormals.hh"

namespace gz::sensors
{

// Integrate a perspective pixel band over the spherical elevation fan. With
// ray (1,u,v), dOmega = du dv / (1+u*u+v*v)^(3/2). The fan boundaries are
// v = +/-tan(beta)*sqrt(1+u*u). Splitting at their intersections makes each
// integral analytic, including the first/last cells of a subsampled column.
inline double PerspectiveBandArea(double a, double b, double low, double high, double beta)
{
  const double t = std::tan(beta), s = std::sin(beta);
  std::vector<double> cuts{a, b};
  for (double v : {low, high})
  {
    if (std::isfinite(v) && std::abs(v) > t)
    {
      const double cross = std::sqrt((v / t) * (v / t) - 1.);
      for (double u : {-cross, cross})
      {
        if (u > a && u < b)
        {
          cuts.push_back(u);
        }
      }
    }
  }
  std::sort(cuts.begin(), cuts.end());
  auto primitive = [](double u, double v)
  { return std::atan(u * v / std::sqrt(1. + u * u + v * v)); };
  double area = 0.;
  for (size_t i = 1; i < cuts.size(); ++i)
  {
    const double x = cuts[i - 1], y = cuts[i], mid = (x + y) / 2.;
    const double edge = t * std::sqrt(1. + mid * mid);
    if (low >= edge || high <= -edge)
    {
      continue;
    }
    const double upper =
      high >= edge ? s * (std::atan(y) - std::atan(x)) : primitive(y, high) - primitive(x, high);
    const double lower =
      low <= -edge ? -s * (std::atan(y) - std::atan(x)) : primitive(y, low) - primitive(x, low);
    area += upper - lower;
  }
  return std::max(0., area);
}

struct PerspectiveSonarGeometry
{
  int columns{0}, rows{0};
  double horizontalTangent{0}, verticalTangent{0}, halfElevation{0};
  std::vector<float> azimuth;
  std::vector<std::vector<float>> elevation;  // column, increasing row
  AcousticReadout readout;
  SurfaceNormalCalibration normalCalibration;

  bool Inside(int row, int column) const
  {
    const double u = horizontalTangent * (2. * (column + .5) / columns - 1.);
    const double v = verticalTangent * (2. * (row + .5) / rows - 1.);
    return std::abs(v) <= std::tan(halfElevation) * std::sqrt(1. + u * u);
  }
};

inline PerspectiveSonarGeometry MakePerspectiveSonarGeometry(
  int columns, int rows, double halfAzimuth, double halfElevation, int stride,
  std::vector<float> receivers, double powerBeamwidth)
{
  if (
    columns < 2 || rows < 2 || stride < 1 || !std::isfinite(halfAzimuth) ||
    !std::isfinite(halfElevation) || halfAzimuth <= 0 || halfAzimuth >= M_PI / 2 ||
    halfElevation <= 0 || halfElevation >= M_PI / 2)
  {
    throw std::invalid_argument("invalid perspective sonar fan");
  }
  PerspectiveSonarGeometry grid;
  grid.columns = columns;
  grid.rows = rows;
  grid.halfElevation = halfElevation;
  grid.horizontalTangent = std::tan(halfAzimuth);
  // A perspective frustum narrows in elevation away from its central column.
  grid.verticalTangent = std::tan(halfElevation) / std::cos(halfAzimuth);
  grid.elevation.resize(columns);
  const double du = 2. * grid.horizontalTangent / columns;
  const double dv = 2. * grid.verticalTangent / rows;
  std::vector<float> fullAreas(static_cast<size_t>(columns) * rows, 0.f);
  std::vector<bool> selected(rows, false);
  for (int c = 0; c < columns; ++c)
  {
    const double u = -grid.horizontalTangent + (c + .5) * du;
    grid.azimuth.push_back(std::atan(u));
    std::vector<int> inside;
    for (int r = 0; r < rows; ++r)
    {
      const double v = -grid.verticalTangent + (r + .5) * dv;
      const double angle = std::atan(v / std::sqrt(1. + u * u));
      grid.elevation[c].push_back(angle);
      if (grid.Inside(r, c))
      {
        inside.push_back(r);
      }
    }
    if (inside.empty())
    {
      throw std::invalid_argument("increase vertical samples to cover the sonar fan");
    }
    std::vector<int> representatives;
    for (size_t i = 0; i < inside.size(); i += static_cast<size_t>(stride))
    {
      representatives.push_back(inside[i]);
    }
    if (representatives.back() != inside.back())
    {
      representatives.push_back(inside.back());
    }
    for (size_t i = 0; i < representatives.size(); ++i)
    {
      const int r = representatives[i];
      // Voronoi boundaries in image coordinates; boundary representatives
      // absorb the remaining clipped fan area, never an outside-center hit.
      const double low = i == 0
                           ? -std::numeric_limits<double>::infinity()
                           : -grid.verticalTangent + (.5 * (representatives[i - 1] + r) + .5) * dv;
      const double high = i + 1 == representatives.size()
                            ? std::numeric_limits<double>::infinity()
                            : -grid.verticalTangent + (.5 * (r + representatives[i + 1]) + .5) * dv;
      fullAreas[static_cast<size_t>(r) * columns + c] = PerspectiveBandArea(
        -grid.horizontalTangent + c * du, -grid.horizontalTangent + (c + 1) * du, low, high,
        halfElevation);
      selected[r] = true;
    }
  }
  // Reuse the same receiver response as the spherical geometry path. Replace
  // only its separable cell areas with the exact perspective partition.
  grid.readout = MakeAcousticReadout(
    grid.azimuth, {0.f}, -halfAzimuth, halfAzimuth, -halfElevation, halfElevation, 1,
    std::move(receivers), powerBeamwidth);
  grid.readout.geometryRows = rows;
  grid.readout.elevationIndices.clear();
  grid.readout.angularAreas.clear();
  for (int r = 0; r < rows; ++r)
  {
    if (selected[r])
    {
      grid.readout.elevationIndices.push_back(r);
      grid.readout.angularAreas.insert(
        grid.readout.angularAreas.end(), fullAreas.begin() + static_cast<size_t>(r) * columns,
        fullAreas.begin() + static_cast<size_t>(r + 1) * columns);
    }
  }
  grid.normalCalibration = MakeSurfaceNormalCalibration(grid.azimuth, grid.elevation);
  return grid;
}

// Gazebo's XYZ image is top-to-bottom, left-to-right. Acoustic grids increase
// in elevation and azimuth, so reverse both indices without reprojecting XYZ.
inline void ReadPerspectiveSonarFrame(
  const float * image, const PerspectiveSonarGeometry & grid, double minimum, double maximum,
  cv::Mat & points, cv::Mat & ranges)
{
  points.create(grid.rows, grid.columns, CV_32FC3);
  ranges.create(grid.rows, grid.columns, CV_32FC1);
  const float missing = std::numeric_limits<float>::quiet_NaN();
  for (int r = 0; r < grid.rows; ++r)
  {
    for (int c = 0; c < grid.columns; ++c)
    {
      const size_t i =
        (static_cast<size_t>(grid.rows - 1 - r) * grid.columns + grid.columns - 1 - c) * 4;
      const cv::Vec3f p(image[i], image[i + 1], image[i + 2]);
      const double d = cv::norm(p);
      const bool valid = grid.Inside(r, c) && std::isfinite(d) && d >= minimum && d <= maximum;
      points.at<cv::Vec3f>(r, c) = valid ? p : cv::Vec3f(missing, missing, missing);
      ranges.at<float>(r, c) = valid ? static_cast<float>(d) : missing;
    }
  }
}

}  // namespace gz::sensors
