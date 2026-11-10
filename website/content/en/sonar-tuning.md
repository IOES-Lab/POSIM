# Configure and tune sonar

Choose a compute backend, then compare coverage, detail and update rate. The [multibeam sonar reference](sonar.md) contains the quickstart, SDF and parameter table.

## Choose the execution environment

Use the [prepared POSIM terminals](terminals.md). Edit, build and run inside the same native Ubuntu, WSL or Docker environment. `POSIM_WS` is that environment's workspace path. All compute backends need Gazebo's rendering sensors.

| Environment | Compute options and prerequisites |
| --- | --- |
| Native Linux | CPU; WGPU with a usable Vulkan adapter; CUDA with a compatible NVIDIA GPU and toolkit |
| WSL 2 or Linux container | CPU; GPU computation only when the required adapter and driver are visible **inside** that environment |
| macOS Docker/RDP | Start with CPU. The Linux container does not expose the Mac's Metal device. |
| Native macOS | WGPU can use Metal with a separately prepared native ROS/Gazebo toolchain; the [macOS installation guide](install-macos.md) covers Docker/RDP. |

`auto` tries CUDA → WGPU → CPU. For a controlled experiment, select `cpu`, `wgpu` or `cuda` explicitly. An unavailable explicit backend stops sensor initialization. See the [selection table](sonar.md#compute-backend).

## Build the sonar packages

Start from a [source installation](install.md) with its dependencies installed. The `wgpu_vendor` package builds the Rust library, including for CPU use. Confirm that Rust and Cargo are available; install them using [Rust's installation instructions](https://www.rust-lang.org/tools/install) if needed.

```bash
rustc --version
cargo --version
```

From the sourced workspace, build the vendor, sensor, world system and demo:

```bash
cd "$POSIM_WS"
colcon build --merge-install --executor sequential --symlink-install \
  --packages-up-to multibeam_sonar_system posim_multibeam_sonar_demo
source install/setup.bash
```

CUDA is optional. To build CPU/WGPU while deliberately skipping an installed CUDA toolkit, append `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`. Rust dependencies are locked by `Cargo.lock`; the first build needs network access to download them.

```bash
ros2 pkg prefix wgpu_vendor
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

Confirm all four packages, then run the [sonar quickstart](sonar.md#run-a-sonar-example) and receive image/raw data. A package listing alone does not verify that the sensor loads. For a container, check the packages inside it; if missing, use a source build rather than assuming an image tag includes them.

## Select a WGPU adapter

On Linux, check `vulkaninfo --summary` in the environment that runs Gazebo. Use the system's Vulkan driver and tools. A software adapter such as llvmpipe can execute Vulkan without providing hardware acceleration. The WGPU startup output identifies the selected adapter.

```bash
WGPU_BACKEND=vulkan \
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

| Environment variable | Meaning |
| --- | --- |
| `WGPU_BACKEND` | Restrict the API: `vulkan` on Linux; `metal` in a native macOS process. Omit it for platform defaults. |
| `WGPU_ADAPTER_NAME` | Select the first adapter whose name contains this text, ignoring case. Use a name shown by the driver/startup output. |
| `WGPU_POWER_PREF` | `high`, `low` or `none`; defaults to `high` when no adapter name is given. |

Set variables before launching. Stop and relaunch the simulation after changing them: adapter initialization is shared within the process. The [wgpu project](https://github.com/gfx-rs/wgpu) describes its graphics APIs. If initialization or resource checks fail, inspect the reported adapter and limits, then choose `cpu` explicitly or reduce the sensor dimensions.

## Optional CUDA setup

CUDA requires an NVIDIA driver, compiler/runtime, cuFFT and cuBLAS. Check the runtime environment:

```bash
nvidia-smi
nvcc --version
```

Use the host's [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) for Linux containers. On Windows, follow [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) and [Docker Desktop GPU setup](https://docs.docker.com/desktop/features/gpu/) when applicable. Install the Windows display driver; do not install a Linux display driver inside WSL. The container needs CUDA libraries too.

Choose an architecture supported by the GPU and toolkit. This example uses SM 75; replace `75` with your GPU's value. `OFF` re-enables toolkit discovery after a CPU/WGPU-only build.

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --packages-up-to multibeam_sonar_system posim_multibeam_sonar_demo \
  --cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=OFF \
  -DCMAKE_CUDA_ARCHITECTURES=75
source install/setup.bash
```

Use `nvcc --list-gpu-code` to inspect supported targets. Without an explicit architecture, CMake uses the compiler default. For multiple installations, select `-DCUDAToolkit_ROOT=/path/to/cuda` and put that toolkit's `bin` directory on `PATH`. Run the example with `compute_backend:=cuda` to check this backend explicitly.

## Measure a baseline

Keep the scene, target distance and sensor pose fixed. In terminal B, measure the received image rate:

```bash
ros2 topic hz /sensor/multibeam_sonar/sonar_image
```

Stop with Ctrl+C, then inspect simulation speed:

```bash
gz topic -e -t /world/default/stats
```

The sonar world's entity name is `default`. Record `real_time_factor` with the image rate. A requested 30 Hz in simulation time does not guarantee 30 frames per wall-clock second. Stop the viewer with Ctrl+C.

## Tune sampling and range

Change one value in a copy of the [sensor SDF](sonar.md#sensor-configuration), rebuild/source the workspace and relaunch at the same pose.

| Experiment | Comparison | Observe |
| --- | --- | --- |
| Angular detail | 512 versus 256 beams, fixed field of view | Separation of nearby targets and received rate |
| Vertical sampling | 300 versus 150 rays; `raySkips` 10 versus 5 separately | Target continuity and computation time |
| Coverage | Change ray range and `maxDistance` together | Farthest visible target and range scale |
| Update rate | 30 Hz versus 10 Hz | Received rate and real-time factor |

Range spacing is `soundSpeed / (2 × bandwidth)`. The published grid has `ceil(maxDistance / spacing) + 1` cells. For 1500 m/s, 29.9 kHz and 15 m, this gives 599 cells. The final grid coordinate can slightly exceed `maxDistance`; returns beyond the configured range are excluded. Read the actual ranges and beam count from the raw message or CSV.

WGPU preserves the requested transform length, including non-powers of two, and selects FFT stages automatically. Longer ranges and larger bandwidths increase memory and work. In the default grid-based mode, CPU assigns returns to the nearest physical range bin; GPU paths use frequency-domain processing. Treat backend comparisons as separate measurements, including amplitude and range response.

## Record a repeatable comparison

Warm up rendering. Keep `writeLog` and `debugFlag` off for throughput measurements; record data separately using the [sonar CSV workflow](sonar.md) or [ROS recording guide](visualization.md). `blazingSonarImage=false` fixes the seed at 1234.

Save the SDF, POSIM revision, selected backend/adapter, driver/toolkit versions, frame time, received rate, real-time factor and memory usage. Compare raw intensity/range data as well as screenshots. The [citation page](citation.md) identifies the research model.

## GPU processing flow

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="GPU stages in sonar processing" loading="lazy" decoding="async"></a><figcaption>GPU stages in sonar processing</figcaption></figure>

GPU processing includes ray summation, window/correction and FFT stages. Profile computation and memory transfers separately. Figure: POSIM Notion Wiki.
