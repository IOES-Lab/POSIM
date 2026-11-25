#ifndef POSIM_WATER_THRUSTER_DISK_WAVE_INDEX_HH_
#define POSIM_WATER_THRUSTER_DISK_WAVE_INDEX_HH_

#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace posim::water_thruster
{
using Point3 = std::array<double, 3>;
using Face = std::array<std::uint32_t, 3>;

struct FieldIdentity
{
  std::uint64_t world{}, ownerModel{}, fieldEntity{}, generation{};
  std::string nonce;
};

struct FieldStamp
{
  FieldIdentity identity;
  std::uint64_t heldIteration{}, heldSimNs{};
};

struct QueryStamp
{
  FieldIdentity identity;
  std::uint64_t iteration{}, simNs{}, maxAgeNs{};
};

struct Disk
{
  Point3 center{}, unitAxis{};
  double radiusM{};
};

enum class WetState { Unknown, Dry, Wet, Partial };
enum class DiskReason {
  None, Identity, Stale, Mesh, InvalidDisk, UncertifiedSeam,
  NoFaces, UnresolvedIntersection, Touching
};

struct DiskBounds
{
  WetState status{WetState::Unknown};
  FieldStamp applied;
  double lowerM{std::numeric_limits<double>::quiet_NaN()};
  double upperM{std::numeric_limits<double>::quiet_NaN()};
  std::size_t candidateFaces{};
  DiskReason reason{DiskReason::Mesh};
};

class DiskWaveIndex
{
 public:
  static DiskWaveIndex Build(FieldStamp stamp, double tileM, std::uint32_t cells,
                             const std::vector<Point3>& vertices,
                             const std::vector<Face>& faces);

  [[nodiscard]] DiskBounds Query(const QueryStamp& stamp, const Disk& disk) const;

 private:
  struct Triangle
  {
    std::array<Point3, 3> vertex;
    double slopeX, slopeY;
  };
  using Cell = std::pair<int, int>;

  FieldStamp stamp_;
  double tileM_{}, cellM_{};
  bool valid_{}, seamX_{}, seamY_{};
  std::array<double, 4> inner_{};
  std::vector<Triangle> triangles_;
  std::map<Cell, std::vector<std::uint32_t>> sparseBuckets_;
  int firstBucketX_{}, firstBucketY_{}, bucketWidth_{}, bucketHeight_{};
  std::vector<std::uint32_t> bucketOffsets_, bucketFaces_;

  void AppendFaces(Cell cell, std::vector<std::uint32_t>& ids) const;
};
}
#endif
