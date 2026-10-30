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

// Range samples live on the calibrated spherical fan, not a pinhole depth grid.
// Recover Cartesian tangents first, then express each normal in that ray's
// (azimuth tangent, elevation tangent, forward ray) basis. The z component is
// the nonnegative incidence cosine consumed by all acoustic backends.
inline cv::Mat RaySurfaceNormals(
  const cv::Mat & ranges, const std::vector<float> & azimuth, const std::vector<float> & elevation,
  double maximum)
{
  if (
    ranges.type() != CV_32FC1 || ranges.cols != int(azimuth.size()) ||
    ranges.rows != int(elevation.size()) || !std::isfinite(maximum) || maximum <= 0)
  {
    throw std::invalid_argument("invalid_sonar_surface_grid");
  }
  for (const auto * angles : {&azimuth, &elevation})
  {
    for (size_t i = 0; i < angles->size(); ++i)
    {
      if (!std::isfinite((*angles)[i]) || (i && (*angles)[i] <= (*angles)[i - 1]))
      {
        throw std::invalid_argument("invalid_sonar_surface_angles");
      }
    }
  }
  cv::Mat result = cv::Mat::zeros(ranges.size(), CV_32FC3);
  if (ranges.rows < 2 || ranges.cols < 2)
  {
    return result;
  }
  cv::Mat points(ranges.size(), CV_64FC3), valid = cv::Mat::zeros(ranges.size(), CV_8UC1);
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
      const double d = ranges.at<float>(r, c);
      if (!std::isfinite(d) || d <= 0 || d > maximum)
      {
        continue;
      }
      points.at<cv::Vec3d>(r, c) = d * cv::Vec3d(ce[r] * ca[c], ce[r] * sa[c], se[r]);
      valid.at<unsigned char>(r, c) = 1;
    }
  }
  // A fixed angular support prevents a denser elevation fan from amplifying
  // raster-range quantization. At most 25 points enter the local plane fit.
  constexpr double radius = 1.5 * 3.14159265358979323846 / 180.;
  auto neighborhoods = [&](const std::vector<float> & angles)
  {
    std::vector<std::vector<int>> result(angles.size());
    for (size_t i = 0; i < angles.size(); ++i)
    {
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
          result[i].push_back(index);
        }
      }
    }
    return result;
  };
  const auto columns = neighborhoods(azimuth), rows = neighborhoods(elevation);
  for (int r = 0; r < ranges.rows; ++r)
  {
    for (int c = 0; c < ranges.cols; ++c)
    {
      if (!valid.at<unsigned char>(r, c))
      {
        continue;
      }
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
      for (int row : rows[r])
      {
        for (int col : columns[c])
        {
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
      const cv::Vec3d ray(ce[r] * ca[c], ce[r] * sa[c], se[r]);
      if (normal.dot(ray) < 0)
      {
        normal = -normal;
      }
      result.at<cv::Vec3f>(r, c) = cv::Vec3f(
        normal.dot(cv::Vec3d(-sa[c], ca[c], 0)),
        normal.dot(cv::Vec3d(-se[r] * ca[c], -se[r] * sa[c], ce[r])),
        std::min(1., std::max(0., normal.dot(ray))));
    }
  }
  return result;
}
}  // namespace sensors
}  // namespace gz
