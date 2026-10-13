# System requirements

Use compatible operating-system, ROS and Gazebo versions.

- **Ubuntu 26.04**
- **ROS 2 Lyrical**
- **Gazebo Jetty**

## Choose an environment

| Environment | Setup | Rendering |
| --- | --- | --- |
| Native Ubuntu | [Source installation](install.md) | Host graphics driver and a working display for the GUI |
| Linux AMD64 container | AMD64 Docker recipe | Software rendering or explicitly configured NVIDIA access |
| Linux ARM64 container | ARM64 Docker recipe | Renderer available inside the container |
| Docker on macOS | Linux VM using the ARM64 or AMD64 image | Container renderer; the host's Metal GPU is not exposed as a Linux renderer |

Run the native recipe in a Bash shell on Ubuntu. Docker commands run on the host; ROS and Gazebo commands run inside the selected container. Keep the architecture of the image consistent with the execution host whenever possible.

## Sensors and graphics

Camera, depth and DVL processing require a renderer even without a desktop window. Software rendering can provide an initial data check.

Multibeam sonar requires NVIDIA GPU access, CUDA and built sonar libraries. See [Sonar build and performance](sonar-tuning.md).

## Plan resources for your experiment

Start with one vehicle and default sensors, then measure load before expanding.

- CPU, memory and GPU use
- Gazebo real-time factor
- Terrain and collision complexity
- Image resolution and sensor rates
- Vehicle count

Allow disk space for the source workspace, build and install trees, Docker layers, downloaded Gazebo Fuel assets and recorded ROS bags. Source installation also builds the external wave dependency.

## Before continuing

Check network access to ROS/Gazebo package repositories, GitHub and Gazebo Fuel. Some example assets are downloaded on first use. Follow [Ubuntu installation](install.md) or [Docker](docker.md), then verify the environment using [First simulation](quickstart.md).
