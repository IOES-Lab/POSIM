# POSIM multibeam sonar

The ray-based point-scattering model produces intensity/range data, with
phase, reverberation and speckle effects.

## Backend requirements

- ROS 2 Lyrical and Gazebo Jetty
- Compatible NVIDIA GPU and CUDA toolkit
- Built CUDA sonar libraries

CMake skips CUDA targets when the toolkit is unavailable. Check library
installation and sensor output with the [sonar build guide](../../website/content/en/sonar-tuning.md).

## Documentation

See the [POSIM sonar guide](../../website/content/en/sonar.md)
for the model and configuration. Use the
[POSIM installation guide](../../website/content/en/install.md) for current repository
and workspace setup.
