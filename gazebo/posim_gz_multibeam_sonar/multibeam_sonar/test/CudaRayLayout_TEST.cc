#include "CudaRayLayout.hh"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using NpsGazeboSonar::CudaRayLayout;

void Require(bool condition, const char * message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

template <typename Exception, typename Function>
void RequireThrows(Function function, const char * message)
{
  try
  {
    function();
  }
  catch (const Exception &)
  {
    return;
  }
  throw std::runtime_error(message);
}

// Exercise the same layout used by the CUDA writer, allocation and reducer.
// Guard cells and coverage detect both overruns and adjacent-beam overlap.
void CheckCoverage(int beams, int rays, int raySkips, int frequencies)
{
  const auto layout = CudaRayLayout::Make(beams, rays, raySkips, frequencies);
  std::vector<int> storage(static_cast<std::size_t>(layout.elements) + 2, -1);
  std::vector<int> visits(layout.elements, 0);
  for (int beam = 0; beam < beams; ++beam)
  {
    int selected = 0;
    for (int ray = 0; ray < rays; ++ray)
    {
      if (ray % raySkips != 0)
      {
        continue;
      }
      Require(ray / raySkips == selected++, "selected ray mapping");
      for (int f = 0; f < frequencies; ++f)
      {
        const int index = layout.Index(beam, ray / raySkips, f);
        Require(index >= 0 && index < layout.elements, "writer index outside allocation");
        ++visits.at(index);
        storage.at(index + 1) = beam * 1000 + ray * 10 + f;
      }
    }
    Require(selected == layout.selectedRays, "selected ray count");
    for (int f = 0; f < frequencies; ++f)
    {
      int expected = 0;
      for (int ray = 0; ray < rays; ++ray)
      {
        if (ray % raySkips == 0)
        {
          expected += beam * 1000 + ray * 10 + f;
        }
      }
      int reduced = 0;
      // Mirror the reducer's 32 lanes, including selected counts over 32.
      for (int lane = 0; lane < 32; ++lane)
      {
        for (int ray = lane; ray < layout.selectedRays; ray += 32)
        {
          reduced += storage.at(layout.Index(beam, ray, f) + 1);
        }
      }
      Require(reduced == expected, "reducer omitted or mixed selected rays");
    }
  }
  for (int visitCount : visits)
  {
    Require(visitCount == 1, "unwritten or overlapping storage cell");
  }
  Require(storage.front() == -1 && storage.back() == -1, "guard cell overwritten");
  Require(
    layout.Index(beams - 1, layout.selectedRays - 1, frequencies - 1) == layout.elements - 1,
    "last beam/ray/frequency must be the final allocated element");
}
}  // namespace

int main()
{
  try
  {
    // empty_return_sonar.sdf: 32 beams/rays, skip 10, with 400 range samples.
    const int frequencies = 400;
    const auto regression = CudaRayLayout::Make(32, 32, 10, frequencies);
    Require(regression.selectedRays == 4, "32/10 selects rows 0, 10, 20 and 30");
    Require(regression.beamStride == 1600 && regression.elements == 51200, "32/10 storage");
    Require(regression.Index(31, 3, 399) == 51199, "32/10 final write");
    CheckCoverage(32, 32, 10, frequencies);
    CheckCoverage(32, 30, 10, frequencies);  // Divisible: retain three selected rows.
    CheckCoverage(3, 65, 1, 7);              // More than one reduction iteration.
    CheckCoverage(3, 32, 32, 7);             // raySkips == rays selects row zero.
    CheckCoverage(3, 32, 33, 7);             // raySkips > rays still selects row zero.
    CheckCoverage(1, 1, 1, 1);

    int cases = 0;
    for (int rays = 1; rays <= 128; ++rays)
    {
      for (int skip = 1; skip <= 128; ++skip)
      {
        int selected = 0;
        int lastRow = 0;
        for (int ray = 0; ray < rays; ++ray)
        {
          if (ray % skip == 0)
          {
            ++selected;
            lastRow = ray;
          }
        }
        for (int beams : {1, 2, 32})
        {
          for (int freq : {1, 2, 17, 400})
          {
            const auto layout = CudaRayLayout::Make(beams, rays, skip, freq);
            Require(layout.selectedRays == selected, "exhaustive selected count");
            Require(layout.elements == beams * selected * freq, "exhaustive allocation");
            for (int beam = 0; beam < beams; ++beam)
            {
              Require(layout.Index(beam, 0, 0) == beam * selected * freq, "beam start");
              Require(
                layout.Index(beam, lastRow / skip, freq - 1) == (beam + 1) * selected * freq - 1,
                "non-overlapping beam end");
            }
            ++cases;
          }
        }
      }
    }

    const int intMax = std::numeric_limits<int>::max();
    for (int invalid : {0, -1, std::numeric_limits<int>::min()})
    {
      for (std::size_t field = 0; field < 4; ++field)
      {
        std::array<int, 4> dimensions = {32, 32, 10, 400};
        dimensions[field] = invalid;
        RequireThrows<std::invalid_argument>(
          [&] { CudaRayLayout::Make(dimensions[0], dimensions[1], dimensions[2], dimensions[3]); },
          "nonpositive dimension accepted");
      }
    }
    const auto largest = CudaRayLayout::Make(1, intMax, 1, 1);
    Require(largest.elements == intMax, "largest int element count");
    Require(largest.Index(0, intMax - 1, 0) == intMax - 1, "largest int index");
    Require(CudaRayLayout::Make(1, intMax, intMax, 1).selectedRays == 1, "ceil addition overflow");
    Require(
      CudaRayLayout::Make(1, intMax, 2, 1).selectedRays == intMax / 2 + 1,
      "large nondivisible count");
    Require(CudaRayLayout::Make(1, 1, intMax, intMax).elements == intMax, "frequency boundary");
    Require(
      CudaRayLayout::Make(intMax, 1, 1, 1).Index(intMax - 1, 0, 0) == intMax - 1, "beam boundary");
    RequireThrows<std::overflow_error>(
      [=] { CudaRayLayout::Make(1, intMax, 1, 2); }, "per-beam overflow accepted");
    RequireThrows<std::overflow_error>(
      [=] { CudaRayLayout::Make(2, 1, 1, intMax); }, "allocation overflow accepted");
    RequireThrows<std::overflow_error>(
      [=] { CudaRayLayout::Make(intMax, intMax, 1, intMax); }, "triple overflow accepted");
    RequireThrows<std::overflow_error>(
      [] { NpsGazeboSonar::CudaCheckedCount(46341, 46341); }, "correction matrix overflow");
    Require(
      NpsGazeboSonar::CudaCheckedCount(46340, 46340) == 2147395600, "correction matrix boundary");
    const auto twoElements = CudaRayLayout::Make(1, 1, 1, 2);
    const auto sizeMax = std::numeric_limits<std::size_t>::max();
    Require(regression.Bytes(8) == 409600, "allocation bytes");
    Require(twoElements.Bytes(sizeMax / 2) == sizeMax - 1, "byte size boundary");
    RequireThrows<std::overflow_error>(
      [&] { twoElements.Bytes(sizeMax / 2 + 1); }, "byte size overflow accepted");
    RequireThrows<std::overflow_error>([&] { twoElements.Bytes(0); }, "zero element size accepted");
    RequireThrows<std::overflow_error>(
      [] { NpsGazeboSonar::CudaCheckedBytes(0, 4); }, "zero element count accepted");
    RequireThrows<std::overflow_error>(
      [] { NpsGazeboSonar::CudaCheckedBytes(-1, 4); }, "negative element count accepted");
    std::cout << "PASS: CUDA production layout, six coverage/reduction cases, " << cases
              << " dimension cases, invalid inputs and overflow boundaries (host only)\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
