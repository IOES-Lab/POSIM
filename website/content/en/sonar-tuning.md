# Configure and tune sonar

Choose the coverage, detail and update rate needed by your sonar experiment. The [multibeam sonar reference](sonar.md) contains the quickstart, SDF integration and parameter definitions. This guide explains how to compare configurations and prepare the CUDA backend.

## Where to run this guide

Use the Linux terminals prepared in [Prepare your terminals](terminals.md) on an NVIDIA-equipped Ubuntu or supported WSL 2 host, or in a container with GPU access. Edit and build in that environment; `POSIM_WS` is its workspace path.

The macOS Docker/RDP setup does not expose a CUDA GPU. Run this sonar backend on an NVIDIA-equipped Linux or WSL 2 host.

## Establish a baseline

Start the [BlueView P900 example](sonar.md#run-a-sonar-example) and keep its scene, target distance and sensor pose fixed. Confirm that the image and raw observations arrive before measuring throughput. If the CUDA libraries are not installed, complete the toolchain steps below first.

In another sourced terminal, measure the received image rate:

```bash
ros2 topic hz /sensor/multibeam_sonar/sonar_image
```

Press Ctrl+C after collecting a stable sample. Inspect the simulation rate separately:

```bash
gz topic -e -t /world/default/stats
```

The sonar example's world entity is `default`. Record `real_time_factor` with the received image rate: a requested 30 Hz in simulation time does not guarantee 30 images per wall-clock second. Stop the viewer with Ctrl+C.

## Choose an experiment

Change one setting in a copy of the [sonar descriptor](sonar.md#sensor-configuration), rebuild/source the workspace, and restart the simulation at the same pose.

| Experiment | First comparison | What to inspect |
| --- | --- | --- |
| Angular detail | 512 versus 256 horizontal beams, keeping field of view fixed | Separation of nearby targets and received rate |
| Vertical sampling | 300 versus 150 vertical rays | Target continuity and computation time |
| Coverage | Change ray range and `maxDistance` together | Farthest visible target and range-axis scale |
| Update rate | Request 30 Hz versus 10 Hz | Received rate and simulator real-time factor |
| Ray skipping | Compare `raySkips` 10 and 5 | Vertical sampling detail and computation time |

These are comparisons to run, not promised speedups. Keep `writeLog` and `debugFlag` disabled when measuring throughput; enable them separately when investigating raw returns or timing. Save the SDF, POSIM revision, GPU/toolkit versions and observations with each run. The [sensor data guide](visualization.md) covers recording topics.

## Check the toolchain

The [multibeam sonar plugin](sonar.md) needs an NVIDIA driver, CUDA compiler/runtime, cuFFT, cuBLAS and Gazebo rendering.

On the machine or inside the container that will run sonar:

```bash
nvidia-smi
nvcc --version
```

GPU visibility and compiler availability are separate checks. For Docker Engine on Linux, configure the host's [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). On Windows, follow the [Docker Desktop WSL 2 GPU setup](https://docs.docker.com/desktop/features/gpu/). The container image also needs the CUDA libraries.

## Select a CUDA architecture

Set CMake's `CMAKE_CUDA_ARCHITECTURES` to an SM version supported by your GPU and toolkit. This example builds for SM 75; replace `75` with your GPU's architecture. Run it from the sourced workspace:

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --cmake-args -DCMAKE_CUDA_ARCHITECTURES=75
source install/setup.bash
```

Use `nvcc --list-gpu-code` to check the toolkit's supported targets. If you omit the architecture setting, CMake uses the compiler's default. With multiple CUDA installations, pass `-DCUDAToolkit_ROOT=/path/to/cuda` to select the toolkit. Ensure its `bin` directory is on `PATH` when running `nvcc` from your shell.

## Verify installed targets

```bash
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

Check library files to confirm CUDA targets were built. Packages can register even when CUDA targets are skipped.

```bash
test -f "$(ros2 pkg prefix multibeam_sonar)/lib/multibeam_sonar/libmultibeam_sonar.so"
test -f "$(ros2 pkg prefix multibeam_sonar_system)/lib/multibeam_sonar_system/libmultibeam_sonar_system.so"
```

Each command must exit with status 0. Check build/launch logs for skipped targets or load errors, then receive image/raw data in [Multibeam sonar](sonar.md).

## Tune one variable at a time

| Setting | Effect to measure |
| --- | --- |
| Horizontal beams / vertical rays | Angular sampling and computation cost |
| Range / `maxDistance` | Scene coverage and range processing |
| `raySkips` | Sampling versus computation |
| Sensor `update_rate` | Requested sensor throughput |
| `writeLog`, `debugFlag` | Disk and console overhead |
| `blazingSonarImage` | Varying random seed versus fixed seed 1234; compare repeatability |

Record frame processing time, received topic rate, Gazebo real-time factor, CPU/GPU utilization and memory with the same scene and pose. Separate physics, rendering, CUDA computation, publication and logging costs. Warm up the renderer before timing.

Retain a baseline dataset when changing numerical kernels. Compare intensity/range outputs, not only rendered screenshots. Upstream research and attribution are listed under [Citation](citation.md).

## GPU processing flow

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="GPU stages in sonar processing" loading="lazy" decoding="async"></a><figcaption>GPU stages in sonar processing</figcaption></figure>

Sonar processing includes ray summation, window/correction and FFT stages. Separate stage timings from GPU memory transfers when profiling. Figure: POSIM Notion Wiki.
