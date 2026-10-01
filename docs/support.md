# Platform and sonar support

Status checked 2026-10-01. Distinguish source availability, build success,
runtime availability and physical accuracy; they are not interchangeable.

| Path | Current source/image state | Requirements and limits |
| --- | --- | --- |
| Ubuntu source build | `main` targets Ubuntu 26.04 / ROS 2 Lyrical / Gazebo Jetty | Record your commit and dependencies. PR #5's published-image results do not certify `main`. |
| Linux ARM64 candidate | Public PR #5 validation image | Headless 14/14 paths plus one offline camera passed. Docker on Apple Silicon is Linux in a VM. |
| Linux AMD64 candidate | Public PR #5 validation image | Same headless checks passed on native AMD64. Emulation is a different execution environment. |
| CUDA sonar | CUDA implementation exists in `main`; CUDA-specific targets are conditional | Needs a compatible NVIDIA GPU, driver, CUDA toolkit, built plugin/demo targets and a working Gazebo renderer. No CUDA sonar runtime claim is made for the published candidates. |
| WGPU sonar | Experimental [POSIM PR #6](https://github.com/IOES-Lab/POSIM/pull/6), unmerged | Not installed by `main` or PR #5 images. Backend-specific build and runtime validation is required. |
| WAM-V / added ocean-wave work | Experimental [POSIM PR #7](https://github.com/IOES-Lab/POSIM/pull/7), unmerged | Not part of the published candidate; do not confuse it with the existing `dave_ocean_waves` world. |
| GUI, RDP and joystick | Optional interfaces | Outside the published headless checks. The ARM64 image's `-rdp` name is not evidence of GUI validation. |

## CUDA is not provided by a successful CPU/ARM64 build

When no CUDA toolkit is found, the CUDA-specific sonar targets are skipped.
The rest of the workspace can build successfully while sonar libraries or
demo entry points are unavailable. Verify that the required plugin was built
and loaded and that sonar payloads arrive; a loaded scene or successful build
alone is insufficient.

For NVIDIA GPU access inside Linux containers, follow the official
[NVIDIA Container Toolkit guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html).
GPU passthrough does not install missing CUDA/plugin binaries. It also does
not turn an Apple Silicon Mac into a CUDA-capable machine.

Four CUDA-conditioned sonar tutorial paths were excluded from the candidate's
14-path headless acceptance set. They are **backend-unavailable in that test
environment**, not successful sonar trials and not evidence of a sensor defect.

## WGPU is a separate development branch

The earlier [DAVE PR #44](https://github.com/IOES-Lab/dave/pull/44) and its
discussion are preserved as provenance. Continue review and development in
[POSIM PR #6](https://github.com/IOES-Lab/POSIM/pull/6).
Metal on native macOS and Vulkan on an appropriate Linux/Windows GPU are
backend-specific experimental paths, not a cross-platform guarantee.
Do not infer their availability from the PR #5 Docker image.

Docker Desktop runs Linux containers on macOS; these containers do not obtain
the host's native Metal API just because the CPU is ARM64. Native macOS WGPU
work requires its own environment and revision-specific evidence. Windows/WSL
and GPU passthrough likewise require separate host-specific checks.

## Resource and performance limits

Sensor rendering can still be necessary without a visible GUI. Software
rendering may be slower than hardware rendering. Neither Docker use nor a
passing headless test guarantees real-time performance. Resource needs depend
on the selected scene; do not treat historical minimum RAM/GPU examples as a
benchmarked POSIM minimum specification.
