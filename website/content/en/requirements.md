# System requirements

Use a matching ROS, Gazebo and operating-system stack. POSIM's current build recipes target Ubuntu 26.04, ROS 2 Lyrical and Gazebo Jetty.

## Choose an environment

| Environment | Setup | Rendering |
| --- | --- | --- |
| Native Ubuntu | [Source installation](install.md) | Host graphics driver and a working display for the GUI |
| Linux AMD64 container | AMD64 Docker recipe | Software rendering or explicitly configured NVIDIA access |
| Linux ARM64 container | ARM64 Docker recipe | Renderer available inside the container |
| Docker on macOS | Linux VM using the ARM64 or AMD64 image | Container renderer; the host's Metal GPU is not exposed as a Linux renderer |

Run the native recipe in a Bash shell on Ubuntu. Docker commands run on the host; ROS and Gazebo commands run inside the selected container. Keep the architecture of the image consistent with the execution host whenever possible.

## Sensors and graphics

A headless server has no visible GUI, but camera, depth and DVL processing can still require a rendering backend. Software rendering is useful for an initial data check; a working hardware renderer is preferable for image-heavy experiments.

The multibeam sonar implementation requires an NVIDIA GPU, a compatible CUDA toolkit and the built sonar libraries. See [Sonar build and performance](sonar-tuning.md). CUDA is a separate requirement from successfully compiling the other packages.

## Plan resources for your experiment

Terrain complexity, image resolution, sensor rate, number of vehicles and collision geometry affect CPU, RAM and GPU use. Begin with one vehicle and the default sensor settings. Measure memory, GPU utilization and Gazebo's real-time factor before increasing the load. The library does not prescribe a fixed number of simulations per GPU.

Allow disk space for the source workspace, build and install trees, Docker layers, downloaded Gazebo Fuel assets and recorded ROS bags. Source installation also builds the external wave dependency.

## Before continuing

Check network access to ROS/Gazebo package repositories, GitHub and Gazebo Fuel. Some example assets are downloaded on first use. Follow [Ubuntu installation](install.md) or [Docker](docker.md), then verify the environment using [First simulation](quickstart.md).
