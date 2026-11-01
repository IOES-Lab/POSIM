// SPDX-License-Identifier: Apache-2.0
#include "PerspectiveSonarGeometry.hh"
#include "RaySurfaceNormals.hh"

#include <iostream>
#include <numeric>
#include <stdexcept>

using namespace gz::sensors;
namespace
{
void Require(bool condition, const char * message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void CheckPartition()
{
  for (int columns : {7, 32, 129})
  {
    for (int rows : {8, 31, 256})
    {
      for (int stride : {1, 3, 17})
      {
        const auto grid = MakePerspectiveSonarGeometry(
          columns, rows, M_PI / 3, M_PI / 18, stride, {-.6f, 0.f, .6f}, .04);
        const auto & model = grid.readout;
        double area = 0.;
        for (size_t i = 0; i < model.elevationIndices.size(); ++i)
        {
          for (int c = 0; c < columns; ++c)
          {
            const double cell = model.angularAreas[i * columns + c];
            Require(cell >= 0 && std::isfinite(cell), "invalid cell area");
            Require(
              cell == 0 || grid.Inside(model.elevationIndices[i], c),
              "outside-center observation receives acoustic area");
            area += cell;
          }
        }
        Require(
          std::abs(area - 4 * M_PI / 3 * std::sin(M_PI / 18)) < 3e-8,
          "fan angular area is not conserved");
        Require(model.receiveAngles.size() == 3, "geometry changes receiver count");
      }
    }
  }
  // Independent midpoint integration in (azimuth, sin elevation), whose
  // measure is uniform. Include clipped bands crossing both curved edges.
  for (const auto band :
       {std::array<double, 4>{-.9, -.3, -.2, -.05}, std::array<double, 4>{-.12, .31, .05, .23},
        std::array<double, 4>{.7, 1.5, -.3, .29}})
  {
    const double a = std::atan(band[0]), b = std::atan(band[1]);
    const double s = std::sin(.17);
    constexpr int samples = 1600;
    int included = 0;
    for (int i = 0; i < samples; ++i)
    {
      const double azimuth = a + (b - a) * (i + .5) / samples;
      for (int j = 0; j < samples; ++j)
      {
        const double elevation = std::asin(-s + 2 * s * (j + .5) / samples);
        const double v = std::tan(elevation) / std::cos(azimuth);
        included += v >= band[2] && v <= band[3];
      }
    }
    const double numerical = (b - a) * 2 * s * included / (samples * samples);
    const double exact = PerspectiveBandArea(band[0], band[1], band[2], band[3], .17);
    Require(std::abs(exact - numerical) < 2e-6, "clipped cell integral differs from quadrature");
  }
}

void CheckMeasuredGeometry()
{
  const auto grid = MakePerspectiveSonarGeometry(65, 97, M_PI / 3, M_PI / 18, 1, {0.f}, .04);
  std::vector<float> frame(static_cast<size_t>(grid.columns) * grid.rows * 4);
  const cv::Vec3d plane(1., -.12, .08);
  for (int r = 0; r < grid.rows; ++r)
  {
    for (int c = 0; c < grid.columns; ++c)
    {
      const double u = grid.horizontalTangent * (2. * (c + .5) / grid.columns - 1.);
      const double v = grid.verticalTangent * (2. * (r + .5) / grid.rows - 1.);
      const cv::Vec3d ray(1., u, v);
      // A real discontinuity, not an interpolated gap between surfaces.
      const auto p = ray * ((c < 32 ? 2. : 4.) / plane.dot(ray));
      const size_t i =
        (static_cast<size_t>(grid.rows - 1 - r) * grid.columns + grid.columns - 1 - c) * 4;
      for (int k = 0; k < 3; ++k)
      {
        frame[i + k] = p[k];
      }
      frame[i + 3] = 123.f;  // RGB must not change range or acoustic material.
    }
  }
  cv::Mat points, ranges;
  ReadPerspectiveSonarFrame(frame.data(), grid, .2, 15., points, ranges);
  auto normals = CartesianSurfaceNormals(points, ranges, grid.azimuth, grid.elevation, 15.);
  int checked = 0;
  for (int r = 0; r < grid.rows; ++r)
  {
    for (int c = 0; c < grid.columns; ++c)
    {
      const auto p = points.at<cv::Vec3f>(r, c);
      if (!grid.Inside(r, c))
      {
        Require(std::isnan(ranges.at<float>(r, c)), "out-of-fan pixel is a sonar return");
        continue;
      }
      const size_t i =
        (static_cast<size_t>(grid.rows - 1 - r) * grid.columns + grid.columns - 1 - c) * 4;
      for (int k = 0; k < 3; ++k)
      {
        Require(p[k] == frame[i + k], "measured XYZ was reconstructed or interpolated");
      }
      Require(std::abs(cv::norm(p) - ranges.at<float>(r, c)) < 1e-6, "range is not radial");
      if (
        r > 0 && r + 1 < grid.rows && c > 0 && c + 1 < grid.columns && grid.Inside(r - 1, c) &&
        grid.Inside(r + 1, c))
      {
        const double expected = std::abs(cv::Vec3d(p).dot(plane)) / (cv::norm(p) * cv::norm(plane));
        Require(
          std::abs(normals.at<cv::Vec3f>(r, c)[2] - expected) < .001,
          "Cartesian plane or discontinuity has incorrect incidence");
        ++checked;
      }
    }
  }
  Require(checked > 1000, "insufficient plane coverage");
  cv::Mat isolated(points.size(), CV_32FC3, cv::Scalar(NAN, NAN, NAN));
  cv::Mat isolatedRange(ranges.size(), CV_32FC1, cv::Scalar(NAN));
  isolated.at<cv::Vec3f>(48, 32) = {2, 0, 0};
  isolatedRange.at<float>(48, 32) = 2;
  Require(
    cv::norm(CartesianSurfaceNormals(isolated, isolatedRange, grid.azimuth, grid.elevation, 15.)) ==
      0,
    "isolated point acquired a normal");
  ReadPerspectiveSonarFrame(frame.data(), grid, 3., 3.01, points, ranges);
  for (int r = 0; r < ranges.rows; ++r)
  {
    for (int c = 0; c < ranges.cols; ++c)
    {
      const float distance = ranges.at<float>(r, c);
      Require(
        std::isnan(distance) || (distance >= 3. && distance <= 3.01),
        "radial range limits ignored");
    }
  }
}
}  // namespace

int main()
{
  try
  {
    CheckPartition();
    CheckMeasuredGeometry();
    std::cout << "PASS perspective fan, measured XYZ and shared surface normals\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL " << error.what() << '\n';
    return 1;
  }
}
