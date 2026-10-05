# POSIM multibeam sonar

The initial POSIM source retains DAVE's ray-based multibeam sonar implementation
for ROS 2 Lyrical and Gazebo Jetty. Its point-scattering model produces
intensity-range data and includes phase, reverberation, and speckle effects.

## Current backend

This implementation requires a compatible NVIDIA GPU and CUDA toolkit.
When CMake does not find the toolkit, it skips the CUDA-specific targets.
Building the remaining workspace successfully does not mean the sonar backend
is available.

The WGPU integration under review in
[DAVE PR #44](https://github.com/IOES-Lab/dave/pull/44) is not included in the
initial POSIM `main` branch. Metal and Vulkan support must be documented against
the implementation actually merged and tested here.

## Documentation

See the [POSIM sonar guide](../../website/content/en/sonar.md)
for the model and configuration. Use the
[POSIM installation guide](../../website/content/en/install.md) for current repository
and workspace setup.
