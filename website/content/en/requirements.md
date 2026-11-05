# Choose an environment

Choose a route for the operating system already on your computer. **Native Ubuntu 26.04 is recommended** for the most direct access to Gazebo graphics, ROS tools and NVIDIA/CUDA devices.

## Installation routes

| Operating system | Installation method | Installation guide |
|---|---|---|
| Ubuntu 26.04 | Build and run POSIM directly in Ubuntu | [Ubuntu: native installation](install.md) |
| Ubuntu or another Docker-supported Linux distribution, x86-64 | Run the Ubuntu-based POSIM image in Docker; show Gazebo on the Linux desktop | [Linux: Docker installation](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | Install Ubuntu 26.04 in WSL 2, then build POSIM there; WSLg shows Linux windows | [Windows: WSL installation](install-windows.md) |
| Windows 10/11, x86-64 | Use Docker Desktop with WSL 2; run the POSIM image and show its windows through WSLg | [Windows: WSL + Docker](install-windows-docker.md) |
| macOS on Apple silicon | Run the ARM64 Docker image and connect to its Linux desktop with RDP | [macOS: Docker + RDP](install-macos.md) |

A Docker image supplies its own Ubuntu userspace. The Linux host does not need to use the same Ubuntu release as the image. Follow Docker's supported host requirements for your distribution. The macOS route above uses Apple silicon; ARM64 emulation on Intel Macs is not covered here.

## Software inside the simulation environment

Every route uses **ROS 2 Lyrical** and **Gazebo Sim — Gazebo Jetty**. Native installation prepares these in Ubuntu; the Docker image contains them. Install the graphics driver on the host as explained in the selected guide.

## Graphics and sensors

Gazebo windows and camera/depth sensors need a working renderer. Start with one robot and its default sensors. Multibeam sonar offers CPU, WGPU and CUDA computation; [choose and prepare a backend](sonar-tuning.md). WGPU needs a usable Vulkan adapter in Linux or Metal in native macOS. The macOS Docker/RDP setup uses Linux and does not expose Metal; start with CPU.

## Resources and downloads

The workload depends on vehicle count, terrain detail, image size and sensor rate. Monitor memory, GPU load and the Gazebo real-time factor before increasing these settings. Allow disk space for source/build files, Docker images, downloaded Fuel assets and recordings.

Installation needs access to the ROS/Gazebo package repositories, GitHub and Docker Hub. Some worlds download Fuel models on first use. After the installation's demo works, continue with [Prepare your terminals](terminals.md).
