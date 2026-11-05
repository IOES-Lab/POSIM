# Prepare your terminals

A **terminal** is a window where you type commands and read their output. The guides call two ordinary terminal windows **Terminal A** and **Terminal B**: A runs the simulation, while B reads sensor data or sends commands. They are labels for their roles, not different programs.

## Choose where to open the terminals

| Installation route | Terminal A and Terminal B |
|---|---|
| [Ubuntu native](install.md) | Open the Ubuntu Terminal application twice |
| [Linux Docker](install-ubuntu-docker.md) | Open two Linux terminals; enter the same running container from each with the command below |
| [Windows WSL native](install-windows.md) | Open two Ubuntu tabs in Windows Terminal, using the same WSL distribution |
| [Windows WSL + Docker](install-windows-docker.md) | Open two Ubuntu WSL tabs; enter the same Docker container from both |
| [macOS Docker + RDP](install-macos.md) | Connect to the Linux desktop with RDP, then open two terminal windows inside that desktop |

For the Linux and Windows Docker guides, the container is named `posim`. Run this in each host terminal to enter it:

```bash
docker exec -it posim bash
```

In the macOS RDP desktop, you are already inside the container. Run ROS commands in the desktop's Linux terminals. PowerShell and macOS Terminal are used for host commands such as starting Docker containers.

## Understand the workspace

The **ROS workspace** is the directory that holds source code and built packages. Its `src/` directory contains source packages; `build/` holds intermediate build files; `install/` contains the packages ROS loads.

| Installation | Workspace path |
|---|---|
| Ubuntu or WSL native | `~/posim_ws` |
| AMD64 Docker image | `/opt/posim_ws` |
| ARM64 RDP image | `/home/docker/posim_ws` |

In paths, `~` means the current Linux user's home directory. `POSIM_WS` below is a variable holding your workspace path, so later commands work in each installation environment.

## Load the environment in both terminals

For **Ubuntu or WSL native**, run in A and B:

```bash
export POSIM_WS="$HOME/posim_ws"
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source "$POSIM_WS/install/setup.bash"
cd "$POSIM_WS"
```

For **Docker**, including the Linux desktop reached through RDP, run in A and B:

```bash
export POSIM_WS="${POSIM_WS:-${POSIM_UNDERLAY:-/home/docker/posim_ws}}"
source /opt/ros/lyrical/setup.bash
source "$POSIM_WS/install/setup.bash"
cd "$POSIM_WS"
```

`source` loads the paths and settings that let this terminal find ROS and POSIM. Repeat this setup in each new terminal. The Docker images supply the workspace variable and their ArduSub paths.

## Check before starting

```bash
ros2 pkg prefix posim_demos
ros2 pkg prefix ros_gz_sim
```

Both paths should belong to the workspace's `install/` directory. If a package is missing, return to your installation guide. A `ros_gz_sim` path under `/opt/ros/lyrical` means the workspace overlay has not been selected.

Continue with [First simulation](quickstart.md). A command that keeps printing output is still running; use **Ctrl+C in that terminal** to stop it and wait for the prompt before entering another command.
