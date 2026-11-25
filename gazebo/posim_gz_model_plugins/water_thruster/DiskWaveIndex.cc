#include "DiskWaveIndex.hh"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace posim::water_thruster
{
namespace
{
constexpr double kEps = 1e-10;
constexpr double kMarginM = 1e-8;
using V2 = std::array<double, 2>;

bool Same(const FieldIdentity& a, const FieldIdentity& b)
{
  return a.world == b.world && a.ownerModel == b.ownerModel &&
      a.fieldEntity == b.fieldEntity && a.generation == b.generation &&
      a.nonce == b.nonce;
}

bool Valid(const FieldIdentity& id)
{
  return id.world && id.ownerModel && id.fieldEntity && id.generation &&
      id.nonce.size() == 32 &&
      id.nonce.find_first_not_of("0123456789abcdef") == std::string::npos;
}

double Norm(const Point3& p) { return std::hypot(p[0], p[1], p[2]); }
bool Finite(const Point3& p)
{
  return std::all_of(p.begin(), p.end(), [](double x) { return std::isfinite(x); });
}
Point3 Difference(const Point3& a, const Point3& b)
{
  return {a[0]-b[0], a[1]-b[1], a[2]-b[2]};
}
Point3 Cross(const Point3& a, const Point3& b)
{
  return {a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
}

struct Plane { double a, b, c; }; // a*u + b*v <= c
struct Extent
{
  double lo{std::numeric_limits<double>::infinity()};
  double hi{-std::numeric_limits<double>::infinity()};
  bool found{};
};

Extent Extrema(const std::array<Plane, 7>& input, std::size_t count, V2 objective)
{
  std::array<Plane, 7> planes;
  std::size_t n = 0;
  for (std::size_t i = 0; i < count; ++i)
  {
    const auto [a, b, c] = input[i];
    const auto length = std::hypot(a, b);
    if (length > kEps)
      planes[n++] = {a/length, b/length, c/length};
    else if (c < -kEps)
      return {};
    else if (c < 0 || length != 0)
      throw std::domain_error("indeterminate constraint");
  }

  Extent out;
  const auto candidate = [&](V2 p) {
    const auto [u, v] = p;
    if (u*u + v*v > 1+kEps)
      return;
    for (std::size_t i = 0; i < n; ++i)
      if (planes[i].a*u + planes[i].b*v > planes[i].c+kEps)
        return;
    const auto value = objective[0]*u + objective[1]*v;
    out = {std::min(out.lo, value), std::max(out.hi, value), true};
  };
  candidate({0, 0});
  const auto g = std::hypot(objective[0], objective[1]);
  if (g > 0)
  {
    candidate({objective[0]/g, objective[1]/g});
    candidate({-objective[0]/g, -objective[1]/g});
  }
  for (std::size_t i = 0; i < n; ++i)
  {
    const auto [a, b, c] = planes[i];
    if (std::abs(c) <= 1+kEps)
    {
      const auto d = std::sqrt(std::max(0., 1-c*c));
      candidate({a*c-b*d, b*c+a*d});
      candidate({a*c+b*d, b*c-a*d});
    }
    for (std::size_t j = 0; j < i; ++j)
    {
      const auto [d, e, f] = planes[j];
      const auto det = a*e-b*d;
      const auto u = c*e-b*f;
      const auto v = a*f-c*d;
      if (std::abs(det) > kEps)
        candidate({u/det, v/det});
      else if (det && std::abs(u) <= std::abs(det)+kEps &&
               std::abs(v) <= std::abs(det)+kEps)
        throw std::domain_error("indeterminate line intersection");
    }
  }
  return out;
}

double Side(const Point3& a, const Point3& b, const Point3& c)
{
  return (b[0]-a[0])*(c[1]-a[1]) - (b[1]-a[1])*(c[0]-a[0]);
}

bool BordersCross(const std::vector<Point3>& first, const std::vector<Point3>& second)
{
  for (std::size_t i = 1; i < first.size(); ++i)
    for (std::size_t j = 1; j < second.size(); ++j)
    {
      const auto &a = first[i-1], &b = first[i], &c = second[j-1], &d = second[j];
      if (a == c || a == d || b == c || b == d)
        continue;
      if (std::max(a[0], b[0])+kEps < std::min(c[0], d[0]) ||
          std::max(c[0], d[0])+kEps < std::min(a[0], b[0]) ||
          std::max(a[1], b[1])+kEps < std::min(c[1], d[1]) ||
          std::max(c[1], d[1])+kEps < std::min(a[1], b[1]))
        continue;
      if (Side(a,b,c)*Side(a,b,d) <= kEps && Side(c,d,a)*Side(c,d,b) <= kEps)
        return true;
    }
  return false;
}

bool Ascending(const std::vector<Point3>& side, std::size_t axis)
{
  for (std::size_t i = 1; i < side.size(); ++i)
    if (side[i][axis] <= side[i-1][axis])
      return false;
  return true;
}

bool SameSeam(const std::vector<Point3>& from, const std::vector<Point3>& to,
              std::size_t axis, double tile)
{
  for (std::size_t i = 0; i < from.size(); ++i)
  {
    auto p = from[i];
    p[axis] += tile;
    if (Norm(Difference(p, to[i])) > kEps)
      return false;
  }
  return true;
}

int CellAt(double coord, double cell)
{
  const auto i = std::floor(coord/cell);
  if (!std::isfinite(i) || i < -1'000'000 || i > 1'000'000)
    throw std::domain_error("unsupported mesh coordinate");
  return static_cast<int>(i);
}
} // namespace

DiskWaveIndex DiskWaveIndex::Build(FieldStamp stamp, double tile, std::uint32_t cells,
                                  const std::vector<Point3>& points,
                                  const std::vector<Face>& faces)
{
  DiskWaveIndex out;
  out.stamp_ = std::move(stamp);
  if (!Valid(out.stamp_.identity) || !std::isfinite(tile) || tile <= 0 ||
      !cells || cells > 1024)
    return out;
  const auto stride = cells+1;
  if (points.size() != stride*stride || faces.size() != 2*cells*cells)
    return out;
  out.tileM_ = tile;
  out.cellM_ = tile/cells;
  if (!std::all_of(points.begin(), points.end(), Finite))
    return out;

  std::vector<Point3> west, east, south, north;
  auto& [wx, ex, sy, ny] = out.inner_;
  wx = sy = -std::numeric_limits<double>::infinity();
  ex = ny = std::numeric_limits<double>::infinity();
  for (std::uint32_t i = 0; i < stride; ++i)
  {
    west.push_back(points[i*stride]);
    east.push_back(points[i*stride+cells]);
    south.push_back(points[i]);
    north.push_back(points[cells*stride+i]);
    wx = std::max(wx, west.back()[0]);
    ex = std::min(ex, east.back()[0]);
    sy = std::max(sy, south.back()[1]);
    ny = std::min(ny, north.back()[1]);
  }
  if (!(wx < ex && sy < ny) || !Ascending(west, 1) || !Ascending(east, 1) ||
      !Ascending(south, 0) || !Ascending(north, 0) ||
      BordersCross(west, south) || BordersCross(west, north) ||
      BordersCross(east, south) || BordersCross(east, north))
    return out;
  out.seamX_ = SameSeam(west, east, 0, tile);
  out.seamY_ = SameSeam(south, north, 1, tile);

  struct BucketRect { int x0, x1, y0, y1; };
  std::vector<BucketRect> rects;
  rects.reserve(faces.size());
  int xMin=1'000'000, xMax=-1'000'000, yMin=1'000'000, yMax=-1'000'000;
  out.triangles_.reserve(faces.size());
  try
  {
    for (std::uint32_t id = 0; id < faces.size(); ++id)
    {
      const auto a = (id/2/cells)*stride + (id/2 % cells);
      const auto expected = id%2 == 0 ? Face{a, a+1, a+stride+1} :
                                        Face{a, a+stride+1, a+stride};
      if (faces[id] != expected)
        return out;
      const auto& p = points[expected[0]];
      const auto& q = points[expected[1]];
      const auto& r = points[expected[2]];
      const auto normal = Cross(Difference(q,p), Difference(r,p));
      if (!Finite(normal) || normal[2] <= kEps)
        return out;
      out.triangles_.push_back({{p,q,r}, -normal[0]/normal[2], -normal[1]/normal[2]});
      const auto x0 = CellAt(std::min({p[0],q[0],r[0]}), out.cellM_);
      const auto x1 = CellAt(std::max({p[0],q[0],r[0]}), out.cellM_);
      const auto y0 = CellAt(std::min({p[1],q[1],r[1]}), out.cellM_);
      const auto y1 = CellAt(std::max({p[1],q[1],r[1]}), out.cellM_);
      if (x1-x0 > 8 || y1-y0 > 8)
        return out;
      rects.push_back({x0,x1,y0,y1});
      xMin = std::min(xMin,x0);
      xMax = std::max(xMax,x1);
      yMin = std::min(yMin,y0);
      yMax = std::max(yMax,y1);
    }
  }
  catch (const std::domain_error&)
  {
    return out;
  }
  const auto width = xMax-xMin+1, height = yMax-yMin+1;
  const auto bucketCount = static_cast<std::int64_t>(width)*height;
  const auto visit = [&](const auto& add) {
    for (std::uint32_t i = 0; i < rects.size(); ++i)
    {
      const auto [x0,x1,y0,y1] = rects[i];
      for (int x = x0; x <= x1; ++x)
        for (int y = y0; y <= y1; ++y)
          add(x,y,i);
    }
  };
  if (bucketCount > 0 && bucketCount <= 4*static_cast<std::int64_t>(faces.size())+16)
  {
    out.firstBucketX_ = xMin;
    out.firstBucketY_ = yMin;
    out.bucketWidth_ = width;
    out.bucketHeight_ = height;
    out.bucketOffsets_.resize(bucketCount+1);
    const auto offset = [&](int x,int y) { return (y-yMin)*width+x-xMin; };
    visit([&](int x,int y,std::uint32_t) { ++out.bucketOffsets_[offset(x,y)+1]; });
    std::partial_sum(out.bucketOffsets_.begin(),out.bucketOffsets_.end(),out.bucketOffsets_.begin());
    auto next = out.bucketOffsets_;
    out.bucketFaces_.resize(next.back());
    visit([&](int x,int y,std::uint32_t face) { out.bucketFaces_[next[offset(x,y)]++] = face; });
  }
  else
  {
    visit([&](int x,int y,std::uint32_t face) { out.sparseBuckets_[{x,y}].push_back(face); });
  }
  out.valid_ = true;
  return out;
}

void DiskWaveIndex::AppendFaces(Cell cell, std::vector<std::uint32_t>& ids) const
{
  if (!bucketOffsets_.empty())
  {
    const auto x = cell.first-firstBucketX_, y = cell.second-firstBucketY_;
    if (x < 0 || y < 0 || x >= bucketWidth_ || y >= bucketHeight_)
      return;
    const auto offset = y*bucketWidth_+x;
    ids.insert(ids.end(),bucketFaces_.begin()+bucketOffsets_[offset],
                         bucketFaces_.begin()+bucketOffsets_[offset+1]);
    return;
  }
  const auto found = sparseBuckets_.find(cell);
  if (found != sparseBuckets_.end())
    ids.insert(ids.end(),found->second.begin(),found->second.end());
}

DiskBounds DiskWaveIndex::Query(const QueryStamp& query, const Disk& disk) const
{
  DiskBounds out;
  out.applied = stamp_;
  if (!Same(query.identity, stamp_.identity))
  {
    out.reason = DiskReason::Identity;
    return out;
  }
  if (query.iteration < stamp_.heldIteration || query.simNs < stamp_.heldSimNs ||
      query.simNs-stamp_.heldSimNs > query.maxAgeNs)
  {
    out.reason = DiskReason::Stale;
    return out;
  }
  if (!valid_) return out;
  const auto radius = disk.radiusM;
  const auto half = tileM_/2;
  if (!Finite(disk.center) || !Finite(disk.unitAxis) || !std::isfinite(radius) ||
      radius <= 0 || radius >= half || std::abs(Norm(disk.unitAxis)-1.) > kEps)
  {
    out.reason = DiskReason::InvalidDisk;
    return out;
  }

  const auto& n = disk.unitAxis;
  auto u = Cross(n, std::abs(n[0]) < .8 ? Point3{1,0,0} : Point3{0,1,0});
  const auto scale = radius/Norm(u);
  for (auto& value : u) value *= scale;
  const auto v = Cross(n, u);
  const auto [cx, cy, cz] = disk.center;
  const auto rx = std::hypot(u[0],v[0]);
  const auto ry = std::hypot(u[1],v[1]);
  Extent total;

  try
  {
    const auto k0 = CellAt(cx-rx+half-kEps, tileM_);
    const auto k1 = CellAt(cx+rx+half+kEps, tileM_);
    const auto l0 = CellAt(cy-ry+half-kEps, tileM_);
    const auto l1 = CellAt(cy+ry+half+kEps, tileM_);
    out.reason = DiskReason::UncertifiedSeam;
    if ((k0 != k1 && !seamX_) || (l0 != l1 && !seamY_)) return out;

    for (int k = k0; k <= k1; ++k)
      for (int l = l0; l <= l1; ++l)
      {
        const auto x = cx-k*tileM_;
        const auto y = cy-l*tileM_;
        std::array<Plane, 7> clip{{
          {u[0],v[0],half-x},{-u[0],-v[0],half+x},
          {u[1],v[1],half-y},{-u[1],-v[1],half+y}}};
        const auto xr = Extrema(clip, 4, {u[0],v[0]});
        const auto yr = Extrema(clip, 4, {u[1],v[1]});
        if (!xr.found || !yr.found) continue;
        const auto xl = x+xr.lo, xh = x+xr.hi, yl = y+yr.lo, yh = y+yr.hi;
        const auto [ax, bx, ay, by] = inner_;
        out.reason = DiskReason::UncertifiedSeam;
        if (xl < ax-kEps || xh > bx+kEps || yl < ay-kEps || yh > by+kEps)
          return out;

        std::vector<std::uint32_t> ids;
        for (int i = CellAt(xl-kEps,cellM_); i <= CellAt(xh+kEps,cellM_); ++i)
          for (int j = CellAt(yl-kEps,cellM_); j <= CellAt(yh+kEps,cellM_); ++j)
            AppendFaces({i,j},ids);
        std::sort(ids.begin(), ids.end());
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
        out.candidateFaces += ids.size();
        bool intersects = false;
        for (const auto id : ids)
        {
          const auto& t = triangles_[id];
          for (std::size_t i = 0; i < t.vertex.size(); ++i)
          {
            const auto& a = t.vertex[i];
            const auto& b = t.vertex[(i+1)%3];
            const auto ex = b[0]-a[0], ey = b[1]-a[1];
            clip[4+i] = {ey*u[0]-ex*u[1],ey*v[0]-ex*v[1],
                         ex*(y-a[1])-ey*(x-a[0])};
          }
          const auto [px,py,pz] = t.vertex.front();
          const auto base = pz+t.slopeX*(x-px)+t.slopeY*(y-py)-cz;
          const auto h = Extrema(clip, 7, {
              t.slopeX*u[0]+t.slopeY*u[1]-u[2],
              t.slopeX*v[0]+t.slopeY*v[1]-v[2]});
          if (h.found)
          {
            intersects = true;
            total = {std::min(total.lo,base+h.lo),std::max(total.hi,base+h.hi),true};
          }
        }
        if (!intersects)
        {
          out.reason = DiskReason::NoFaces;
          return out;
        }
      }
  }
  catch (const std::domain_error&)
  {
    out.reason = DiskReason::UnresolvedIntersection;
    return out;
  }

  if (!total.found || !std::isfinite(total.lo) || !std::isfinite(total.hi))
  {
    out.reason = DiskReason::NoFaces;
    return out;
  }
  out.lowerM = total.lo-kMarginM;
  out.upperM = total.hi+kMarginM;
  out.reason = DiskReason::None;
  if (out.lowerM > 0) out.status = WetState::Wet;
  else if (out.upperM < 0) out.status = WetState::Dry;
  else if (out.lowerM < -2*kMarginM && out.upperM > 2*kMarginM)
    out.status = WetState::Partial;
  else out.reason = DiskReason::Touching;
  return out;
}
} // namespace posim::water_thruster
