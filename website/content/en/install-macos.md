# Install on macOS

Run POSIM in its Linux container and open its desktop over Remote Desktop Protocol (RDP). The desktop image targets ARM64 and is intended for Apple silicon Macs. An Intel Mac needs ARM64 emulation for this image; performance and compatibility depend on Docker's emulation support.

## 1. Prepare Docker and a remote desktop client

Install and start [Docker Desktop for Mac](https://docs.docker.com/desktop/setup/install/mac-install/). Install an RDP client that supports direct PC connections, such as [Windows App for macOS](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps). Allow enough Docker disk and memory for the image build and simulations.

## 2. Build the desktop image

In macOS Terminal, clone the repository and build the ARM64 image:

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

The build can take considerable time and downloads ROS, Gazebo and other dependencies. It builds the checked-out POSIM revision. See [Docker environment](docker.md) for the headless and AMD64 alternatives.

## 3. Start and connect to the desktop

Start the desktop container and publish RDP only on this Mac's loopback interface:

```bash
docker run -d --name posim-rdp --shm-size=1g \
  -p 127.0.0.1:3389:3389 posim:dev-arm64-rdp
```

In the RDP client, add a PC at `127.0.0.1:3389`. On the xrdp login screen, select **Xorg** and sign in with username `docker` and password `docker`, the image defaults. These are container credentials, not your macOS account. The image grants this account passwordless sudo; keep the port bound to loopback and change its password with `passwd` in the desktop terminal. If port 3389 is occupied, use `-p 127.0.0.1:3390:3389` when creating the container and connect to `127.0.0.1:3390`.

## 4. Run POSIM

Open a terminal in the remote desktop. The image configures ROS and the POSIM workspace in the `docker` user's shell. Start a world with its Gazebo window:

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

The first launch may download Gazebo Fuel models. Stop the launch with Ctrl+C before closing the desktop. Continue with [First simulation](quickstart.md) for robot and sensor examples; use `gui:=true headless:=false` where a robot or sensor example supports those arguments.

## Stop and return

From macOS Terminal, stop the container with `docker stop posim-rdp`. Later, use `docker start posim-rdp` and reconnect. Files inside this container persist across stop/start; keep important work in a separately mounted directory or volume if you remove or replace the container.

The container uses a Linux renderer; macOS Metal is not directly available to Gazebo. CUDA sonar requires a compatible NVIDIA/CUDA environment; see [sonar setup](sonar-tuning.md). For connection or renderer issues, see [Troubleshooting](troubleshooting.md).
