// SPDX-License-Identifier: Apache-2.0
#pragma once

// Opt-in diagnostic only. The compute function always receives its original
// arguments; errors and quota exhaustion affect recording, never computation.
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

namespace gz::sensors::native_trace
{
struct Frame
{
  bool valid{false};
  int64_t stampNs{0};
  // Position XYZ, then quaternion WXYZ. These are actual rendering-node poses.
  std::array<double, 7> world{}, local{}, parent{};
  bool hasParent{false};
  std::string sensorName, parentName;
  std::array<double, 7> beginWorld{};
  int64_t beginStampNs{0};
  bool hasBegin{false};
  std::string contextSource{"supplied"};
};

template <typename Pose>
inline std::array<double, 7> PoseArray(const Pose & pose)
{
  return {pose.Pos().X(), pose.Pos().Y(), pose.Pos().Z(), pose.Rot().W(),
          pose.Rot().X(), pose.Rot().Y(), pose.Rot().Z()};
}

template <typename Node>
inline Frame Capture(const Node & sensor, int64_t stamp)
{
  Frame frame;
  frame.valid = true;
  frame.contextSource = "render_node";
  frame.stampNs = stamp;
  frame.sensorName = sensor->Name();
  frame.world = PoseArray(sensor->WorldPose());
  frame.local = PoseArray(sensor->LocalPose());
  if (const auto parent = sensor->Parent())
  {
    frame.hasParent = true;
    frame.parentName = parent->Name();
    frame.parent = PoseArray(parent->WorldPose());
  }
  return frame;
}

inline std::string Quote(const std::string & text)
{
  const char * hex = "0123456789abcdef";
  std::string result = "\"";
  for (unsigned char c : text)
  {
    if (c == '"' || c == '\\')
    {
      result += '\\';
      result += char(c);
    }
    else if (c < 32)
    {
      result += "\\u00";
      result += hex[c >> 4];
      result += hex[c & 15];
    }
    else
    {
      result += char(c);
    }
  }
  return result + '"';
}

inline bool Requested()
{
  const char * path = std::getenv("POSIM_SONAR_NATIVE_TRACE_DIR");
  return path && *path;
}

inline thread_local const Frame * currentFrame = nullptr;

class Scope
{
public:
  explicit Scope(const Frame & frame) : previous(currentFrame)
  {
    currentFrame = frame.valid && Requested() ? &frame : nullptr;
  }
  ~Scope() { currentFrame = previous; }
  Scope(const Scope &) = delete;
  Scope & operator=(const Scope &) = delete;

private:
  const Frame * previous;
};

inline std::string Metadata(const Frame & frame)
{
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(std::numeric_limits<double>::max_digits10);
  out << "{\"capture_stamp_ns\":" << frame.stampNs
      << ",\"context_source\":" << Quote(frame.contextSource)
      << ",\"sensor_render_name\":" << Quote(frame.sensorName)
      << ",\"parent_render_name\":" << Quote(frame.parentName)
      << ",\"render_begin_available\":" << (frame.hasBegin ? "true" : "false")
      << ",\"render_begin_stamp_ns\":" << frame.beginStampNs;
  const std::array<std::pair<const char *, const std::array<double, 7> *>, 4> poses{
    {{"render_sensor_world_xyz_wxyz", &frame.world},
     {"render_sensor_local_xyz_wxyz", &frame.local},
     {"render_parent_world_xyz_wxyz", &frame.parent},
     {"render_begin_sensor_world_xyz_wxyz", &frame.beginWorld}}};
  for (const auto & pose : poses)
  {
    out << ",\"" << pose.first << "\":[";
    for (size_t i = 0; i < 7; ++i)
    {
      if (i)
      {
        out << ',';
      }
      const double value = (*pose.second)[i];
      if (std::isfinite(value))
      {
        out << value;
      }
      else
      {
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        out << "{\"f64_bits\":" << bits << '}';
      }
    }
    out << ']';
  }
  out << ",\"render_parent_available\":" << (frame.hasParent ? "true" : "false") << '}';
  return out.str();
}

struct Writer
{
  static constexpr uint64_t maxFrames = 4;
  static constexpr uint64_t maxBytes = 40 * 1024 * 1024;
  std::mutex mutex;
  uint64_t frames{0}, bytes{0};
  int64_t nextStamp{0}, spacingNs{0};
  std::string directory;
  bool warned{false}, configured{false};

  void Warn()
  {
    if (!warned)
    {
      std::fputs(
        "[sonar native trace] Incomplete or unavailable diagnostic; computation unchanged.\n",
        stderr);
      warned = true;
    }
  }

  bool Configure()
  {
    const char * path = std::getenv("POSIM_SONAR_NATIVE_TRACE_DIR");
    if (!path || !*path)
    {
      return false;
    }
    if (configured)
    {
      return directory == path;
    }
    configured = true;
    directory = path;
    for (const auto & setting :
         {std::make_pair("POSIM_SONAR_NATIVE_TRACE_START_NS", &nextStamp),
          std::make_pair("POSIM_SONAR_NATIVE_TRACE_SPACING_NS", &spacingNs)})
    {
      const char * text = std::getenv(setting.first);
      if (!text)
      {
        continue;
      }
      // Nonnegative integer nanoseconds only; no silent partial conversion.
      uint64_t value = 0;
      if (!*text)
      {
        directory.clear();
        Warn();
        return false;
      }
      for (const char * p = text; *p; ++p)
      {
        if (*p < '0' || *p > '9' || value > (uint64_t(INT64_MAX) - (*p - '0')) / 10)
        {
          directory.clear();
          Warn();
          return false;
        }
        value = value * 10 + (*p - '0');
      }
      *setting.second = static_cast<int64_t>(value);
    }
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error))
    {
      directory.clear();
      Warn();
      return false;
    }
    return true;
  }
};

inline Writer & State()
{
  static Writer writer;
  return writer;
}

using Compute = float * (*)(const float *, const float *, const float *, const float *,
                            const float *, uint32_t, uint32_t, uint32_t, uint32_t, float, float,
                            float, float, float, float, float, float, float, float, uint64_t,
                            uint64_t, bool);

// Header and all arrays are native little-endian, with an exact-length footer.
// The five input arrays are saved before the call; complete complex output is
// saved before the caller frees it. A missing footer is an invalid record.
inline float * Call(
  Compute compute, const float * depth, const float * normal, const float * rho,
  const float * mixing, const float * window, uint32_t beams, uint32_t rays, uint32_t frequencies,
  uint32_t skips, float speed, float distance, float source, float attenuation, float gain,
  float bandwidth, float correctorSum, float area, float hfov, float vfov, uint64_t frame,
  uint64_t seed, bool debug)
{
  FILE * file = nullptr;
  // Close and flush incomplete records when the original compute call throws.
  std::unique_ptr<FILE, int (*)(FILE *)> owner(nullptr, &std::fclose);
  bool complete = true;
  auto write = [&](const void * data, size_t size)
  {
    if (file && complete)
    {
      complete = std::fwrite(data, 1, size, file) == size;
    }
  };
  try
  {
    if (currentFrame)
    {
      auto & state = State();
      std::lock_guard<std::mutex> lock(state.mutex);
      if (
        state.Configure() && state.frames < Writer::maxFrames &&
        currentFrame->stampNs >= state.nextStamp)
      {
        const uint32_t endian = 1;
        const uint64_t cells = uint64_t(beams) * rays;
        const std::string meta = Metadata(*currentFrame);
        const bool valid = beams > 0 && beams <= 8192 && rays > 0 && rays <= 8192 &&
                           frequencies > 0 && frequencies <= 4096 && depth && normal && rho &&
                           mixing && window && meta.size() <= 65536 &&
                           *reinterpret_cast<const uint8_t *>(&endian) == 1;
        const uint64_t payload = valid ? 4 * (5 * cells + uint64_t(beams) * beams + frequencies +
                                              2 * uint64_t(beams) * frequencies)
                                       : Writer::maxBytes + 1;
        const uint64_t size = 8 + 4 + 16 + 40 + 16 + 1 + meta.size() + payload + 8;
        if (!valid || size > Writer::maxBytes - state.bytes)
        {
          state.Warn();
        }
        else
        {
          const auto path = std::filesystem::path(state.directory) /
                            ("sonar-native-" + std::to_string(state.frames) + ".bin");
          // Exclusive creation preserves previous runs, including partial files.
          file = std::fopen(path.string().c_str(), "wbx");
          owner.reset(file);
          if (!file)
          {
            state.Warn();
          }
          else
          {
            ++state.frames;
            state.bytes += size;
            if (state.spacingNs > 0)
            {
              state.nextStamp = currentFrame->stampNs > INT64_MAX - state.spacingNs
                                  ? INT64_MAX
                                  : currentFrame->stampNs + state.spacingNs;
            }
            const uint32_t metadataSize = static_cast<uint32_t>(meta.size());
            const std::array<uint32_t, 4> dimensions{beams, rays, frequencies, skips};
            const std::array<float, 10> scalars{speed,     distance,     source, attenuation, gain,
                                                bandwidth, correctorSum, area,   hfov,        vfov};
            const std::array<uint64_t, 2> ids{frame, seed};
            const uint8_t flag = debug ? 1 : 0;
            write("PSNWG001", 8);
            write(&metadataSize, 4);
            write(dimensions.data(), 16);
            write(scalars.data(), 40);
            write(ids.data(), 16);
            write(&flag, 1);
            write(meta.data(), meta.size());
            write(depth, cells * 4);
            write(normal, cells * 12);
            write(rho, cells * 4);
            write(mixing, uint64_t(beams) * beams * 4);
            write(window, uint64_t(frequencies) * 4);
          }
        }
      }
    }
  }
  catch (...)
  {
    complete = false;
  }

  // Preserve all original inputs and the returned pointer, even when recording
  // is disabled, invalid, unavailable, full, or out of space.
  float * result = compute(
    depth, normal, rho, mixing, window, beams, rays, frequencies, skips, speed, distance, source,
    attenuation, gain, bandwidth, correctorSum, area, hfov, vfov, frame, seed, debug);
  if (file)
  {
    if (result)
    {
      write(result, uint64_t(beams) * frequencies * 8);
      write("COMPLETE", 8);
    }
    else
    {
      complete = false;
    }
    const bool closed = std::fclose(owner.release()) == 0;
    if (!complete || !closed)
    {
      auto & state = State();
      std::lock_guard<std::mutex> lock(state.mutex);
      state.Warn();
    }
  }
  return result;
}
}  // namespace gz::sensors::native_trace
