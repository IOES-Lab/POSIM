// SPDX-License-Identifier: Apache-2.0
#include <atomic>
#include "sonar_native_trace.hh"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace gz::sensors::native_trace;
std::array<float, 32> data{}, output{};
std::atomic<int> calls{0};
bool nullOutput = false;
bool throwOutput = false;

float * ComputeStub(
  const float * d, const float * n, const float * r, const float * m, const float * w, uint32_t b,
  uint32_t y, uint32_t f, uint32_t skip, float speed, float distance, float source,
  float attenuation, float gain, float bandwidth, float sum, float area, float hfov, float vfov,
  uint64_t frame, uint64_t seed, bool debug)
{
  assert(d == data.data() && n == d && r == d && m == d && w == d);
  assert(skip == 7 && speed == 1500 && distance == 15 && source == 220);
  assert(attenuation == .1f && gain == 1 && bandwidth == 29900 && sum == 2 && area == .2f);
  assert(hfov == .3f && vfov == .4f && frame >= 100 && seed == 73 && !debug);
  assert(
    (b == 2 && y == 3 && f == 4) || (b == UINT32_MAX && y == UINT32_MAX && f == UINT32_MAX) ||
    (b == 8192 && y == 8192 && f == 599));
  ++calls;
  if (throwOutput)
  {
    throw std::runtime_error("original compute error");
  }
  return nullOutput ? nullptr : output.data();
}

int main(int argc, char ** argv)
{
  assert(argc == 2);
  const std::string mode = argv[1];
  std::filesystem::path testDirectory;
  if (mode == "ctest")
  {
    testDirectory = std::filesystem::temp_directory_path() /
                    ("posim-sonar-trace-" +
                     std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    assert(std::filesystem::create_directory(testDirectory));
#ifdef _WIN32
    _putenv_s("POSIM_SONAR_NATIVE_TRACE_DIR", testDirectory.string().c_str());
#else
    setenv("POSIM_SONAR_NATIVE_TRACE_DIR", testDirectory.string().c_str(), 1);
#endif
  }
  for (size_t i = 0; i < data.size(); ++i)
  {
    data[i] = float(i) / 7;
    output[i] = -float(i) / 3;
  }
  // Preserve payload IEEE values; recording is never a point-quality filter.
  const uint32_t nanBits = 0x7fc01234;
  std::memcpy(data.data() + 1, &nanBits, 4);
  std::memcpy(output.data() + 2, &nanBits, 4);
  Frame context;
  context.valid = mode != "no-frame";
  context.world = {1, 2, 3, 1, 0, 0, 0};
  context.local = {.1, .2, .3, 1, 0, 0, 0};
  context.parent = {4, 5, 6, 1, 0, 0, 0};
  context.hasParent = true;
  context.sensorName = "test\"sensor\n\\name";
  context.parentName = "parent";
  if (mode == "nonfinite-pose")
  {
    const uint64_t bits = 0x7ff8000000001234;
    std::memcpy(&context.world[2], &bits, 8);
  }
  nullOutput = mode == "null-output";
  throwOutput = mode == "throw-output";
  const bool large = mode == "oversized";
  const bool byteLimit = mode == "byte-limit";
  auto run = [&](int i)
  {
    Frame frame = context;
    frame.stampNs = i * 10;
    Scope scope(frame);
    const auto ptr = Call(
      ComputeStub, data.data(), data.data(), data.data(), data.data(), data.data(),
      large ? UINT32_MAX : (byteLimit ? 8192 : 2), large ? UINT32_MAX : (byteLimit ? 8192 : 3),
      large ? UINT32_MAX : (byteLimit ? 599 : 4), 7, 1500, 15, 220, .1f, 1, 29900, 2, .2f, .3f, .4f,
      100 + i, 73, false);
    assert(ptr == (nullOutput ? nullptr : output.data()));
    assert(currentFrame == (frame.valid && Requested() ? &frame : nullptr));
  };
  if (mode == "threads")
  {
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
    {
      threads.emplace_back(run, i);
    }
    for (auto & t : threads)
    {
      t.join();
    }
    assert(calls == 8);
  }
  else
  {
    for (int i = 0; i < 7; ++i)
    {
      try
      {
        run(mode == "reverse" ? 6 - i : i);
        assert(!throwOutput);
      }
      catch (const std::runtime_error & error)
      {
        assert(throwOutput && std::string(error.what()) == "original compute error");
        assert(currentFrame == nullptr);
      }
    }
    assert(calls == 7);
  }
  assert(currentFrame == nullptr);
  if (mode == "ctest")
  {
    uint64_t bytes = 0;
    size_t count = 0;
    for (const auto & entry : std::filesystem::directory_iterator(testDirectory))
    {
      std::ifstream input(entry.path(), std::ios::binary);
      const std::string record{std::istreambuf_iterator<char>(input), {}};
      assert(record.substr(0, 8) == "PSNWG001");
      assert(record.substr(record.size() - 8) == "COMPLETE");
      uint32_t metadataSize;
      std::memcpy(&metadataSize, record.data() + 8, 4);
      size_t position = 85 + metadataSize;
      assert(record.size() == position + 216 + 8);
      for (size_t size : {24u, 72u, 24u, 16u, 16u})
      {
        assert(std::memcmp(record.data() + position, data.data(), size) == 0);
        position += size;
      }
      assert(std::memcmp(record.data() + position, output.data(), 64) == 0);
      bytes += record.size();
      ++count;
    }
    assert(count == 4 && bytes <= Writer::maxBytes);
    std::filesystem::remove_all(testDirectory);
  }
  std::cout << "Original call arguments, pointer, IEEE payload and scope ownership retained: "
            << calls << " calls\n";
}
