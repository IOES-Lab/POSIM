// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcousticReadout.hh"
#include "sonar_compute_backend.hh"

namespace gz::sensors
{

struct PackedPhysicalSonar
{
  int geometryColumns{0};
  int selectedRows{0};
  std::vector<float> depth, normal, reflectivity, angularArea;
};

inline bool PackPhysicalSonar(const SonarComputeInput & input, PackedPhysicalSonar & packed)
{
  if (
    !input.physicalReadout || !input.depthImage || !input.normalImage || !input.reflectivityImage ||
    !input.window || input.nFreq < 1 || !std::isfinite(input.soundSpeed) || input.soundSpeed <= 0 ||
    !std::isfinite(input.bandwidth) || input.bandwidth <= 0 || !std::isfinite(input.maxDistance) ||
    input.maxDistance <= 0 || !std::isfinite(input.attenuation) || input.attenuation < 0 ||
    !std::isfinite(input.sourceLevel) || !std::isfinite(input.receiveGain) || input.receiveGain < 0)
  {
    return false;
  }
  if (
    input.maxDistance >= double(input.nFreq) * input.soundSpeed / (2. * input.bandwidth) ||
    input.nOutputRanges < 0 || input.nOutputRanges > input.nFreq ||
    (input.nOutputRanges > 0 && input.nOutputRanges > input.nFreq / 2))
  {
    return false;
  }
  const auto & model = *input.physicalReadout;
  const auto & depth = *input.depthImage;
  const auto & normal = *input.normalImage;
  const auto & reflectivity = *input.reflectivityImage;
  if (
    depth.empty() || depth.type() != CV_32FC1 || normal.type() != CV_32FC3 ||
    reflectivity.type() != CV_32FC1 || depth.size() != normal.size() ||
    depth.size() != reflectivity.size() || depth.cols != model.geometryColumns ||
    depth.rows != model.geometryRows || input.nRays != depth.rows ||
    input.nBeams != static_cast<int>(model.receiveAngles.size()) || input.nBeams < 1 ||
    model.elevationIndices.empty() ||
    model.angularAreas.size() != model.elevationIndices.size() * static_cast<size_t>(depth.cols) ||
    model.pressureResponse.size() != static_cast<size_t>(input.nBeams) * depth.cols)
  {
    return false;
  }
  for (int f = 0; f < input.nFreq; ++f)
  {
    if (!std::isfinite(input.window[f]))
    {
      return false;
    }
  }
  for (float response : model.pressureResponse)
  {
    if (!std::isfinite(response))
    {
      return false;
    }
  }
  packed.geometryColumns = depth.cols;
  packed.selectedRows = static_cast<int>(model.elevationIndices.size());
  const size_t size = static_cast<size_t>(packed.geometryColumns) * packed.selectedRows;
  packed.depth.assign(size, 0.f);
  packed.normal.assign(size * 3, 0.f);
  packed.reflectivity.assign(size, 0.f);
  packed.angularArea.assign(size, 0.f);
  for (int r = 0; r < packed.selectedRows; ++r)
  {
    const int row = model.elevationIndices[r];
    if (row < 0 || row >= depth.rows || (r && row <= model.elevationIndices[r - 1]))
    {
      return false;
    }
    for (int col = 0; col < depth.cols; ++col)
    {
      const size_t i = static_cast<size_t>(col) * packed.selectedRows + r;
      const float area = model.angularAreas[static_cast<size_t>(r) * depth.cols + col];
      if (!std::isfinite(area) || area < 0)
      {
        return false;
      }
      const float distance = depth.at<float>(row, col);
      if (!(distance > 0 && distance <= input.maxDistance))
      {
        continue;
      }
      const auto direction = normal.at<cv::Vec3f>(row, col);
      const float material = reflectivity.at<float>(row, col);
      if (
        !std::isfinite(material) || material < 0 || !std::isfinite(direction[2]) ||
        std::abs(direction[2]) > 1.00001f)
      {
        return false;
      }
      packed.depth[i] = distance;
      packed.reflectivity[i] = material;
      packed.angularArea[i] = area;
      packed.normal[i * 3 + 2] = direction[2];  // already ray-local incidence
    }
  }
  return true;
}

bool ComputePhysicalSonarCpu(const SonarComputeInput & input, SonarComputeOutput & output);

}  // namespace gz::sensors
