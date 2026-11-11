#pragma once

#include "CudaRayLayout.hh"
#include "sonar_compute_backend.hh"

namespace PosimGazeboSonar
{
/// The wrapper copies step * rows bytes. Require tightly packed rows so an
/// offset ROI cannot make that copy extend past the final row's backing data.
inline bool CudaImageStorageMatches(const cv::Mat & image, int beams, int rays, int type)
{
  return beams > 0 && rays > 0 && image.dims == 2 && !image.empty() && image.cols == beams &&
         image.rows == rays && image.type() == type && image.isContinuous() &&
         static_cast<std::size_t>(beams) <=
           static_cast<std::size_t>(std::numeric_limits<int>::max()) / image.elemSize() / rays &&
         image.step == static_cast<std::size_t>(beams) * image.elemSize() &&
         image.step <= static_cast<std::size_t>(std::numeric_limits<int>::max()) / rays;
}

inline void ValidateCudaImages(
  const cv::Mat & depth, const cv::Mat & normals, const cv::Mat & reflectivity, int beams, int rays)
{
  if (
    !CudaImageStorageMatches(depth, beams, rays, CV_32FC1) ||
    !CudaImageStorageMatches(normals, beams, rays, CV_32FC3) ||
    !CudaImageStorageMatches(reflectivity, beams, rays, CV_32FC1))
  {
    throw std::invalid_argument(
      "CUDA sonar images must be tightly packed and match the beam/ray dimensions and int strides");
  }
}

/// Validate the host buffers and dimensions before computing angular steps or
/// entering CUDA. Raw window/correction pointers still require caller-owned
/// storage of the declared sizes; their allocation extents are not in the ABI.
inline void ValidateCudaComputeInput(const gz::sensors::SonarComputeInput & input)
{
  if (
    !input.depthImage || !input.normalImage || !input.reflectivityImage || !input.window ||
    !input.beamCorrector || input.nBeams < 2 || input.nRays < 2)
  {
    throw std::invalid_argument("CUDA sonar requires input buffers and at least two beams/rays");
  }
  const auto layout = CudaRayLayout::Make(input.nBeams, input.nRays, input.raySkips, input.nFreq);
  layout.Bytes(sizeof(std::complex<float>));
  CudaCheckedBytes(CudaCheckedCount(input.nBeams, input.nBeams), sizeof(float));
  ValidateCudaImages(
    *input.depthImage, *input.normalImage, *input.reflectivityImage, input.nBeams, input.nRays);
  if (input.elevation_angles.size() < static_cast<std::size_t>(input.nRays))
  {
    throw std::invalid_argument("CUDA sonar elevation angles do not cover every ray");
  }
  for (int beam = 0; beam < input.nBeams; ++beam)
  {
    if (!input.beamCorrector[beam])
    {
      throw std::invalid_argument("CUDA sonar correction matrix contains a null row");
    }
  }
}

/// Invalid input never reaches the CUDA calculation. Convert only validation
/// errors into backend failure; do not select a fallback or hide other errors.
template <typename Calculate>
bool TryCudaCalculation(const gz::sensors::SonarComputeInput & input, Calculate calculate)
{
  try
  {
    ValidateCudaComputeInput(input);
    calculate();
    return true;
  }
  catch (const std::invalid_argument &)
  {
    return false;
  }
  catch (const std::overflow_error &)
  {
    return false;
  }
}
}  // namespace PosimGazeboSonar
