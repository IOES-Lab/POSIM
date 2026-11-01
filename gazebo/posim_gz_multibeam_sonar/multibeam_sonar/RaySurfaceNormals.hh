// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <opencv2/core.hpp>
#include <stdexcept>
#include <vector>

namespace gz
{
namespace sensors
{

// Fixed angular neighborhoods belong to calibration, not to each frame.
struct SurfaceNormalCalibration
{
  int width{0}, height{0};
  std::vector<double> ca, sa;
  std::vector<std::vector<double>> ce, se;
  std::vector<std::array<int, 5>> columns;
  std::vector<std::vector<std::array<int, 5>>> rows;
};

inline SurfaceNormalCalibration MakeSurfaceNormalCalibration(
  const std::vector<float> & azimuth, const std::vector<std::vector<float>> & elevations)
{
  if (elevations.empty() || (elevations.size() != 1 && elevations.size() != azimuth.size()))
  {
    throw std::invalid_argument("invalid_sonar_surface_grid");
  }
  auto check = [](const std::vector<float> & angles)
  {
    for (size_t i = 0; i < angles.size(); ++i)
    {
      if (!std::isfinite(angles[i]) || (i && angles[i] <= angles[i - 1]))
      {
        throw std::invalid_argument("invalid_sonar_surface_angles");
      }
    }
  };
  check(azimuth);
  SurfaceNormalCalibration grid;
  grid.width = azimuth.size();
  grid.height = elevations.front().size();
  grid.ce.resize(elevations.size());
  grid.se.resize(elevations.size());
  for (float a : azimuth)
  {
    grid.ca.push_back(std::cos(a));
    grid.sa.push_back(std::sin(a));
  }
  constexpr double radius = 1.5 * 3.14159265358979323846 / 180.;
  auto neighborhoods = [&](const std::vector<float> & angles)
  {
    std::vector<std::array<int, 5>> result(angles.size());
    for (size_t i = 0; i < angles.size(); ++i)
    {
      result[i].fill(-1);
      size_t count = 0;
      for (int k : {-2, -1, 0, 1, 2})
      {
        const double target = angles[i] + k * radius / 2.;
        auto it = std::lower_bound(angles.begin(), angles.end(), target);
        int index = int(it - angles.begin());
        if (
          index == int(angles.size()) ||
          (index && target - angles[index - 1] < angles[index] - target))
        {
          --index;
        }
        if (std::find(result[i].begin(), result[i].end(), index) == result[i].end())
        {
          result[i][count++] = index;
        }
      }
    }
    return result;
  };
  grid.columns = neighborhoods(azimuth);
  for (size_t c = 0; c < elevations.size(); ++c)
  {
    const auto & elevation = elevations[c];
    if (int(elevation.size()) != grid.height)
    {
      throw std::invalid_argument("invalid_sonar_surface_grid");
    }
    check(elevation);
    for (float e : elevation)
    {
      grid.ce[c].push_back(std::cos(e));
      grid.se[c].push_back(std::sin(e));
    }
    grid.rows.push_back(neighborhoods(elevation));
  }
  return grid;
}

// The same local surface estimator handles spherical and measured XYZ grids.
// Missing returns and isolated hits remain unsupported.
inline cv::Mat CartesianSurfaceNormals(
  const cv::Mat & measuredPoints, const cv::Mat & ranges,
  const SurfaceNormalCalibration & calibration, double maximum)
{
  if (
    ranges.type() != CV_32FC1 || ranges.cols != calibration.width ||
    ranges.rows != calibration.height || measuredPoints.size() != ranges.size() ||
    (measuredPoints.type() != CV_32FC3 && measuredPoints.type() != CV_64FC3) ||
    !std::isfinite(maximum) || maximum <= 0)
  {
    throw std::invalid_argument("invalid_sonar_surface_grid");
  }
  cv::Mat result = cv::Mat::zeros(ranges.size(), CV_32FC3);
  if (ranges.rows < 2 || ranges.cols < 2)
  {
    return result;
  }
  cv::Mat points, valid = cv::Mat::zeros(ranges.size(), CV_8UC1);
  measuredPoints.convertTo(points, CV_64FC3);
  for (int r = 0; r < ranges.rows; ++r)
  {
    for (int c = 0; c < ranges.cols; ++c)
    {
      const double d = ranges.at<float>(r, c);
      const auto p = points.at<cv::Vec3d>(r, c);
      if (
        std::isfinite(d) && d > 0 && d <= maximum && std::isfinite(p[0]) && std::isfinite(p[1]) &&
        std::isfinite(p[2]))
      {
        valid.at<unsigned char>(r, c) = 1;
      }
    }
  }
  constexpr double radius = 1.5 * 3.14159265358979323846 / 180.;
  const auto & ca = calibration.ca;
  const auto & sa = calibration.sa;
  const auto & ce = calibration.ce;
  const auto & se = calibration.se;
  const auto & rows = calibration.rows;
  const auto & columns = calibration.columns;
  // Every output normal owns one pixel; cap scheduling to four row groups.
  // This preserves the estimator while sharing its cost with the renderer.
  cv::parallel_for_(
    cv::Range(0, ranges.rows),
    [&](const cv::Range & rowsRange)
    {
      for (int r = rowsRange.start; r < rowsRange.end; ++r)
      {
        for (int c = 0; c < ranges.cols; ++c)
        {
          if (!valid.at<unsigned char>(r, c))
          {
            continue;
          }
          const size_t ec = ce.size() == 1 ? 0 : c;
          const auto center = points.at<cv::Vec3d>(r, c);
          auto tangent = [&](int dr, int dc, cv::Vec3d & direction)
          {
            bool found = false;
            double shortest = 0.;
            for (int side : {-1, 1})
            {
              const int row = r + side * dr, col = c + side * dc;
              if (
                row < 0 || row >= ranges.rows || col < 0 || col >= ranges.cols ||
                !valid.at<unsigned char>(row, col))
              {
                continue;
              }
              const cv::Vec3d delta = (points.at<cv::Vec3d>(row, col) - center) * side;
              const double length = delta.dot(delta);
              // Prefer the nearby side at a depth discontinuity; never bridge a
              // missing return or extrapolate a normal from an isolated hit.
              if (length > 1e-20 && (!found || length < shortest))
              {
                direction = delta;
                shortest = length;
                found = true;
              }
            }
            return found;
          };
          cv::Vec3d da, de;
          if (!tangent(0, 1, da) || !tangent(1, 0, de))
          {
            continue;
          }
          cv::Vec3d normal = da.cross(de);
          const double length = cv::norm(normal);
          if (length < 1e-12)
          {
            continue;
          }
          normal /= length;
          // Center relative coordinates avoid cancellation at long range. Reject
          // distant surfaces; keep the local one-sided tangent at mixed corners.
          const double reach = 3. * ranges.at<float>(r, c) * std::tan(radius);
          std::array<cv::Vec3d, 25> support;
          int count = 0;
          cv::Vec3d sum(0, 0, 0);
          for (int row : rows[ec][r])
          {
            if (row < 0)
            {
              continue;
            }
            for (int col : columns[c])
            {
              if (col < 0)
              {
                continue;
              }
              if (!valid.at<unsigned char>(row, col))
              {
                continue;
              }
              const cv::Vec3d delta = points.at<cv::Vec3d>(row, col) - center;
              if (delta.dot(delta) > reach * reach)
              {
                continue;
              }
              support[count++] = delta;
              sum += delta;
            }
          }
          if (count >= 6)
          {
            const cv::Vec3d mean = sum / count;
            cv::Matx33d covariance = cv::Matx33d::zeros();
            for (int i = 0; i < count; ++i)
            {
              const cv::Vec3d q = support[i] - mean;
              for (int j = 0; j < 3; ++j)
              {
                for (int k = 0; k < 3; ++k)
                {
                  covariance(j, k) += q[j] * q[k];
                }
              }
            }
            cv::Vec3d values;
            cv::Matx33d vectors;
            if (
              cv::eigen(covariance, values, vectors) && values[1] > 1e-12 &&
              values[2] <= .01 * values[1])
            {
              normal = cv::Vec3d(vectors(2, 0), vectors(2, 1), vectors(2, 2));
            }
          }
          const cv::Vec3d ray(ce[ec][r] * ca[c], ce[ec][r] * sa[c], se[ec][r]);
          if (normal.dot(ray) < 0)
          {
            normal = -normal;
          }
          result.at<cv::Vec3f>(r, c) = cv::Vec3f(
            normal.dot(cv::Vec3d(-sa[c], ca[c], 0)),
            normal.dot(cv::Vec3d(-se[ec][r] * ca[c], -se[ec][r] * sa[c], ce[ec][r])),
            std::min(1., std::max(0., normal.dot(ray))));
        }
      }
    },
    4);
  return result;
}
inline cv::Mat CartesianSurfaceNormals(
  const cv::Mat & measuredPoints, const cv::Mat & ranges, const std::vector<float> & azimuth,
  const std::vector<std::vector<float>> & elevations, double maximum)
{
  return CartesianSurfaceNormals(
    measuredPoints, ranges, MakeSurfaceNormalCalibration(azimuth, elevations), maximum);
}

// A spherical range grid supplies calibrated directions. Build its Cartesian
// measurements once, then use the same estimator as a direct geometry source.
inline cv::Mat RaySurfaceNormals(
  const cv::Mat & ranges, const std::vector<float> & azimuth, const std::vector<float> & elevation,
  double maximum)
{
  if (
    ranges.type() != CV_32FC1 || ranges.cols != int(azimuth.size()) ||
    ranges.rows != int(elevation.size()))
  {
    throw std::invalid_argument("invalid_sonar_surface_grid");
  }
  cv::Mat points(ranges.size(), CV_64FC3);
  std::vector<double> ca, sa, ce, se;
  for (float a : azimuth)
  {
    ca.push_back(std::cos(a));
    sa.push_back(std::sin(a));
  }
  for (float e : elevation)
  {
    ce.push_back(std::cos(e));
    se.push_back(std::sin(e));
  }
  for (int r = 0; r < ranges.rows; ++r)
  {
    for (int c = 0; c < ranges.cols; ++c)
    {
      const double distance = ranges.at<float>(r, c);
      points.at<cv::Vec3d>(r, c) = distance * cv::Vec3d(ce[r] * ca[c], ce[r] * sa[c], se[r]);
    }
  }
  return CartesianSurfaceNormals(points, ranges, azimuth, {elevation}, maximum);
}
}  // namespace sensors
}  // namespace gz
