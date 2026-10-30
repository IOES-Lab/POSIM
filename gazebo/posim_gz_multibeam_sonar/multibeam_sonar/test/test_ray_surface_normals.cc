// SPDX-License-Identifier: Apache-2.0
#include "RaySurfaceNormals.hh"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr double kPi = 3.14159265358979323846;

void Require(bool condition, const std::string &message)
{
  if (!condition)
    throw std::runtime_error(message);
}

std::vector<float> Angles(int count, double halfWidth)
{
  std::vector<float> result(count);
  for (int i = 0; i < count; ++i)
    result[i] = -halfWidth + 2.0 * halfWidth * i / (count - 1);
  return result;
}

cv::Vec3d Ray(double azimuth, double elevation)
{
  return {std::cos(elevation) * std::cos(azimuth),
          std::cos(elevation) * std::sin(azimuth), std::sin(elevation)};
}

void CheckPlane(int width, int height, cv::Vec3d planeNormal, double offset)
{
  const auto azimuth = Angles(width, kPi / 3.0);
  const auto elevation = Angles(height, kPi / 18.0);
  planeNormal /= cv::norm(planeNormal);
  cv::Mat ranges(height, width, CV_32FC1);
  cv::Mat valid = cv::Mat::zeros(height, width, CV_8UC1);
  for (int row = 0; row < height; ++row)
    for (int col = 0; col < width; ++col)
    {
      const double cosine = Ray(azimuth[col], elevation[row]).dot(planeNormal);
      const double distance = cosine > 0 ? offset / cosine : 100000.0;
      ranges.at<float>(row, col) = distance;
      valid.at<unsigned char>(row, col) = distance > 0 && distance < 10.0;
    }
  const auto result = gz::sensors::RaySurfaceNormals(ranges, azimuth, elevation, 10.0);
  int checked = 0;
  double maximumError = 0;
  for (int row = 1; row + 1 < height; ++row)
    for (int col = 1; col + 1 < width; ++col)
    {
      const auto value = result.at<cv::Vec3f>(row, col);
      Require(std::isfinite(value[0]) && std::isfinite(value[1]) && std::isfinite(value[2]),
              "non-finite normal");
      if (!valid.at<unsigned char>(row, col) ||
          !valid.at<unsigned char>(row - 1, col) ||
          !valid.at<unsigned char>(row + 1, col) ||
          !valid.at<unsigned char>(row, col - 1) ||
          !valid.at<unsigned char>(row, col + 1))
        continue;
      const double expected = std::abs(Ray(azimuth[col], elevation[row]).dot(planeNormal));
      maximumError = std::max(maximumError, std::abs(value[2] - expected));
      ++checked;
    }
  Require(checked > 0, "plane fixture has no supported points");
  Require(maximumError < 0.001, "metric plane incidence differs from analytic normal");
}

void CheckMissingAndStep()
{
  cv::Mat isolated(9, 13, CV_32FC1, cv::Scalar(100000.0));
  isolated.at<float>(4, 6) = 2.0;
  const auto unsupported = gz::sensors::RaySurfaceNormals(
      isolated, Angles(13, 1.0), Angles(9, 0.2), 10.0);
  Require(cv::norm(unsupported) == 0, "isolated return fabricates a surface normal");

  const auto azimuth = Angles(256, kPi / 3.0);
  const auto elevation = Angles(256, kPi / 18.0);
  cv::Mat ranges(256, 256, CV_32FC1);
  for (int row = 0; row < 256; ++row)
    for (int col = 0; col < 256; ++col)
    {
      const bool raised = row >= 80 && row < 180 && col >= 100 && col < 160;
      ranges.at<float>(row, col) = (raised ? 2.95 : 3.0) / Ray(azimuth[col], elevation[row])[0];
    }
  const auto result = gz::sensors::RaySurfaceNormals(ranges, azimuth, elevation, 10.0);
  double maximumError = 0;
  for (int row = 0; row < 256; ++row)
    for (int col = 0; col < 256; ++col)
      maximumError = std::max(maximumError,
          std::abs(result.at<cv::Vec3f>(row, col)[2] - Ray(azimuth[col], elevation[row])[0]));
  Require(maximumError < 0.001, "shallow step is replaced by an averaged ramp");
}
}  // namespace

int main()
{
  try
  {
    for (const auto &size : {cv::Size(64, 32), cv::Size(256, 256)})
    {
      CheckPlane(size.width, size.height, {1, 0, 0}, 3.0);
      CheckPlane(size.width, size.height, {1, 0.3, 0.2}, 3.0);
      CheckPlane(size.width, size.height, {0.35, 0, -0.94}, 1.3);
    }
    CheckMissingAndStep();
    std::cout << "PASS: six analytic planes, isolated return and shallow step\n";
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
