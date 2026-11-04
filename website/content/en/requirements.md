# System requirements

Use compatible operating-system, ROS and Gazebo versions.

- **Ubuntu 26.04**
- **ROS 2 Lyrical**
- **Gazebo Jetty**

## Choose an environment

| Environment | Setup | Rendering |
| --- | --- | --- |
| Native Ubuntu | [Source installation](install.md) | Host graphics driver and a working display for the GUI |
| Windows with WSL 2 | [Install on Windows](install-windows.md) | WSLg and a compatible graphics driver for GUI apps |
| macOS with Docker and RDP | [Install on macOS](install-macos.md) | Linux container renderer; macOS Metal is not exposed to Gazebo |
| Linux AMD64 container | [AMD64 Docker recipe](docker.md) | Software rendering or explicitly configured NVIDIA access |
| Linux ARM64 container | [ARM64 Docker recipe](docker.md) | Renderer available inside the container |

Run the native recipe in a Bash shell on Ubuntu. Docker commands run on the host; ROS and Gazebo commands run inside the selected container. Keep the architecture of the image consistent with the execution host whenever possible.

## Sensors and graphics

Camera, depth and DVL processing require a renderer even without a desktop window. Software rendering can provide an initial data check.

Multibeam sonar requires NVIDIA GPU access, CUDA and built sonar libraries. See [Configure and tune sonar](sonar-tuning.md).

## Plan resources for your experiment

Start with one vehicle and default sensors, then measure load before expanding.

- CPU, memory and GPU use
- Gazebo real-time factor
- Terrain and collision complexity
- Image resolution and sensor rates
- Vehicle count

Allow disk space for the source workspace, build and install trees, Docker layers, downloaded Gazebo Fuel assets and recorded ROS bags. Source installation also builds the external wave dependency.

## Before continuing

Check network access to ROS/Gazebo package repositories, GitHub and Gazebo Fuel. Some example assets are downloaded on first use. Choose [Ubuntu](install.md), [Windows](install-windows.md), [macOS](install-macos.md) or [Docker](docker.md), then verify the environment using [First simulation](quickstart.md).
