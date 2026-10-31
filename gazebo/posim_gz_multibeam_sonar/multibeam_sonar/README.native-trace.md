# Bounded native sonar input recording

[한국어](README.native-trace.ko.md)

This opt-in diagnostic records the exact WGPU call operands and complete complex
output. It also records rendering-node poses at render start and at the ray
callback. The callback pose and capture timestamp travel with the cloned depth
snapshot to the asynchronous compute thread. Default sensor operation does not
record files. The Rust C ABI and published ROS topics are unchanged.

Use a new, existing, writable directory for each process. Existing files are
never overwritten. The limits are **four frames and 40 MiB total**. Invalid
configuration, unavailable storage, incomplete writes and exhausted quotas leave
the original compute call and returned pointer unchanged. A compute exception
closes the incomplete file and propagates to the caller.

```sh
export POSIM_SONAR_NATIVE_TRACE_DIR=/path/to/new-empty-directory
export POSIM_SONAR_NATIVE_TRACE_START_NS=20000000000
export POSIM_SONAR_NATIVE_TRACE_SPACING_NS=20000000000
```

The optional start and spacing values are nonnegative integer simulation
nanoseconds. Recording selects the first eligible call at or after each time
boundary. With spacing zero it records the first four eligible calls, without
filtering by intensity, geometry or return validity. Configuration is fixed for
the lifetime of the writer. Disable recording by leaving the directory variable
unset when starting the process. CPU and CUDA backends do not produce WGPU
records.

## Record format

Files are named `sonar-native-0.bin` through `sonar-native-3.bin`. Integer and
float fields use little-endian encoding. The fixed header has Python struct
layout `<8sI4I10f2QB`:

- Eight-byte magic `PSNWG001`, then the UTF-8 metadata length.
- Four uint32 values: beams, vertical rays, frequency samples, ray skips.
- Ten float32 values: sound speed, maximum distance, source level, attenuation,
  gain, bandwidth, corrector sum, area, horizontal FOV and vertical FOV.
- Two uint64 values: frame index and seed; one byte for the debug flag.

The JSON metadata follows the header. It contains the captured simulation stamp,
rendering-node names, callback sensor world/local poses, parent world pose and
render-start sensor world pose/stamp. Poses use XYZ position and WXYZ quaternion.
`context_source=render_node` identifies rendering-node observations. Supplied
offline replay poses use a separate context label. Nonfinite pose components
are preserved as `{"f64_bits": <uint64>}` rather than invalid JSON numbers.

The remaining float32 arrays are contiguous, in this order:

- Depth `[beams, rays]` and backend normals `[beams, rays, 3]`.
- Reflectivity `[beams, rays]`, beam corrector `[beams, beams]`, window `[samples]`.
- Complete complex output `[beams, samples, 2]`, with real/imaginary components.

The final eight bytes are `COMPLETE`. Consumers must verify exact expected byte
length, dimensions and the footer before accepting a record. Null output,
exceptions, interrupted or failed writes leave an incomplete record. Retain it
as a failure; do not treat a filename alone as successful capture.

## Validation boundary

`test/NativeTrace_TEST.cc` exercises argument/pointer preservation, exact IEEE
payloads, thread-local scope and bounded files. The CMake test
`bounded_native_trace` uses only the standard C++17 library. It also verifies
that assertions remain active in release builds.

Replay controls with supplied grids can prove operand and output byte identity.
They do not establish actual renderer range/pose/time agreement or SLAM map
accuracy. Join a real capture to the published complete raw image and timestamp,
then compare the actual renderer range grid with scene first hits at the recorded
pose before attributing discrepancies to an acoustic algorithm.
