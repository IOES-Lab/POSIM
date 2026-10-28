# Sonar build and performance

Build the multibeam sonar once, then select WGPU, CUDA or CPU when launching it. WGPU uses Vulkan on Linux and Metal on native macOS. All compute choices require Gazebo rendering for the ray sensor.

## Prepare the toolchain

Use the [POSIM workspace setup](install.md) with ROS 2 Lyrical, Gazebo Jetty and ROS–Gazebo bridge packages. Run the build from the workspace root, such as `~/posim_ws`.

- Rust and Cargo are required to build the WGPU library, including when you plan to select CPU processing.
- Linux WGPU needs a Vulkan loader and an appropriate GPU driver. `vulkaninfo --summary` lists available adapters.
- CUDA needs a compatible NVIDIA GPU, its driver, the CUDA compiler/runtime, cuFFT and cuBLAS. CUDA support is included when CMake finds the toolkit.
- Rendering needs Ogre2 and a working display, virtual display or configured headless renderer.

```bash
rustc --version
cargo --version
```

If Rust is unavailable, use the [official Rust installer](https://www.rust-lang.org/tools/install). For CUDA, also check:

```bash
nvidia-smi
nvcc --version
```

GPU access and compiler availability are separate checks. Linux containers using NVIDIA GPUs also need the host's [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) and the driver/runtime libraries inside the container.

## Build and source

On a fresh Ubuntu workspace, build and source the WGPU library before building the remaining packages:

```bash
source /opt/ros/lyrical/setup.bash
colcon build --merge-install --executor sequential --symlink-install \
  --packages-select wgpu_vendor
source install/setup.bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
```

In an already sourced native macOS workspace, use the same two build commands with that workspace's ROS environment. Rebuild the workspace after changing its compiler or GPU toolchain.

## Select a CUDA architecture

The sonar CMake variable `CUDA_ARCHITECTURE` defaults to `native`. It selects the local GPU's architecture when CUDA support is built. For a different deployment GPU, supply the SM value supported by that GPU and toolkit:

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --packages-select multibeam_sonar \
  --cmake-args -DCUDA_ARCHITECTURE=YOUR_GPU_SM
source install/setup.bash
```

Replace `YOUR_GPU_SM` with the numeric architecture. The project forwards this value to CMake's `CMAKE_CUDA_ARCHITECTURES`. Check the toolkit's supported targets before selecting a value.

## Verify installed targets

```bash
ros2 pkg prefix wgpu_vendor
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

On Linux, check the installed libraries:

```bash
test -f "$(ros2 pkg prefix wgpu_vendor)/lib/libsonar_wgpu.a"
test -f "$(ros2 pkg prefix multibeam_sonar)/lib/multibeam_sonar/libmultibeam_sonar.so"
test -f "$(ros2 pkg prefix multibeam_sonar_system)/lib/multibeam_sonar_system/libmultibeam_sonar_system.so"
```

Each command must exit with status 0. Run the [sonar example](sonar.md) and confirm the selected backend in its log, then receive image and raw data. A package prefix or library file alone does not confirm GPU access.

## Choose a WGPU adapter

Set these variables in the terminal that launches Gazebo:

| Variable | Behavior |
| --- | --- |
| `WGPU_BACKEND` | Selects the GPU API, such as `vulkan` on Linux or `metal` on native macOS |
| `WGPU_ADAPTER_NAME` | Selects the first adapter whose name contains the value, ignoring case |
| `WGPU_POWER_PREF` | Requests `high`, `low` or `none`; the default is `high` |

For example, select a Vulkan adapter whose name contains `RTX`:

```bash
WGPU_BACKEND=vulkan WGPU_ADAPTER_NAME=RTX WGPU_POWER_PREF=high \
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

Use an adapter name available on your machine. The log lists available names when a requested name has no match. Once selected, the WGPU adapter is shared by the sonar sensors in that Gazebo process; start a new process to change it.

## Apple Silicon

For native macOS execution, prepare a sourced ROS 2 workspace with Gazebo Jetty, Ogre2 and the ROS–Gazebo bridge. Build Rust/Cargo and the sonar packages as above. The WGPU compute backend uses the Apple GPU through Metal:

```bash
WGPU_BACKEND=metal \
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

Metal provides the acoustic computation; Gazebo still needs its rendering environment. Use `compute_backend:=cpu` to select CPU acoustic processing. CUDA requires a compatible NVIDIA environment.

For a Linux ARM64 container on an Apple Silicon host, follow [Docker setup and virtual-display rendering](docker.md). Run the [server-only sonar command](sonar.md) with `POSIM_SONAR_COMPUTE_BACKEND=cpu` inside the container. A Linux container requires a Vulkan adapter for WGPU; its compute API is separate from native macOS Metal.

## Resolve startup errors

| Symptom | Action |
| --- | --- |
| `wgpu_vendorConfig.cmake` is missing | Build `wgpu_vendor`, source `install/setup.bash`, then build the remaining packages |
| WGPU reports no usable adapter | Check the GPU driver, `vulkaninfo --summary` on Linux and adapter-selection variables; choose `auto` or `cpu` if GPU processing is unavailable |
| Explicit CUDA selection fails | Check CUDA build support, `nvidia-smi`, the runtime libraries and container GPU access |
| An unknown backend is reported | Use `auto`, `wgpu`, `cuda` or `cpu` |
| Another world with the same name is running | Stop the existing POSIM launch with Ctrl+C before relaunching |
| Backend initializes but no image arrives | Confirm the world is unpaused, then check the renderer and the [sonar topics](sonar.md) |

## Tune one variable at a time

| Setting | Effect to measure |
| --- | --- |
| Horizontal beams / vertical rays | Angular sampling and computation cost |
| Range / `maxDistance` | Scene coverage and range processing |
| `raySkips` | Sampling versus computation |
| Sensor `update_rate` | Requested sensor throughput |
| `writeLog`, `debugFlag` | Disk and console overhead |
| Sensor gain | Image presentation; distinguish it from physical return strength |

Record frame processing time, received topic rate, Gazebo real-time factor and CPU/GPU utilization in the same scene and pose. Separate physics, rendering, acoustic computation, publication and recording costs. Allow the renderer to warm up before measuring. The configured 30 Hz update rate is a request; received throughput depends on the scene, sensor settings and hardware.

Use raw intensity–range data when comparing backends or sensor settings, and keep gain and display settings fixed. Research sources and attribution are listed under [Citation](citation.md).

## GPU processing flow

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="GPU stages in sonar processing" loading="lazy" decoding="async"></a><figcaption>GPU stages in sonar processing</figcaption></figure>

Sonar processing includes backscatter, beam correction and range compression. Separate computation time from memory transfers when profiling. Figure: POSIM Notion Wiki.
