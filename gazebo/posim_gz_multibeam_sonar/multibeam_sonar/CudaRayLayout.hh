#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

#if defined(__CUDACC__)
#define POSIM_CUDA_HOST_DEVICE __host__ __device__
#else
#define POSIM_CUDA_HOST_DEVICE
#endif

namespace NpsGazeboSonar
{
/// Multiply positive dimensions without overflowing the CUDA backend's int indices.
inline int CudaCheckedCount(int left, int right)
{
  if (left <= 0 || right <= 0)
  {
    throw std::invalid_argument("CUDA sonar dimensions must be positive");
  }
  if (left > std::numeric_limits<int>::max() / right)
  {
    throw std::overflow_error("CUDA sonar dimensions exceed int indexing");
  }
  return left * right;
}

inline std::size_t CudaCheckedBytes(int elements, std::size_t elementSize)
{
  if (
    elements <= 0 || elementSize == 0 ||
    elementSize > std::numeric_limits<std::size_t>::max() / elements)
  {
    throw std::overflow_error("CUDA sonar allocation size is not representable");
  }
  return static_cast<std::size_t>(elements) * elementSize;
}

/// Packed [beam][selected ray][frequency] storage. Only rows divisible by
/// raySkips are selected, including the final partial group of rows.
struct CudaRayLayout
{
  int selectedRays;
  int frequencies;
  int beamStride;
  int elements;

  static CudaRayLayout Make(int beams, int rays, int raySkips, int frequencies)
  {
    if (beams <= 0 || rays <= 0 || raySkips <= 0 || frequencies <= 0)
    {
      throw std::invalid_argument("CUDA sonar dimensions and raySkips must be positive");
    }
    // Unlike (rays + raySkips - 1) / raySkips, this cannot overflow.
    const int selectedRays = 1 + (rays - 1) / raySkips;
    const int beamStride = CudaCheckedCount(selectedRays, frequencies);
    return {selectedRays, frequencies, beamStride, CudaCheckedCount(beams, beamStride)};
  }

  std::size_t Bytes(std::size_t elementSize) const
  {
    return CudaCheckedBytes(elements, elementSize);
  }

  /// Coordinates must be inside the dimensions validated by Make.
  POSIM_CUDA_HOST_DEVICE int Index(int beam, int selectedRay, int frequency) const
  {
    return beam * beamStride + selectedRay * frequencies + frequency;
  }
};
}  // namespace NpsGazeboSonar

#undef POSIM_CUDA_HOST_DEVICE
