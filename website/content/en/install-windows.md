# Windows: WSL installation

Install POSIM directly in Ubuntu 26.04 running under Windows Subsystem for Linux 2 (WSL2). WSLg displays Gazebo windows on your Windows desktop. To use a prepared image instead, choose [Windows: WSL + Docker](install-windows-docker.md).

## 1. Install WSL and Ubuntu

Open PowerShell as Administrator and run:

```powershell
wsl --install -d Ubuntu-26.04
```

Restart Windows if requested. Open Ubuntu and create your Linux username and password. In PowerShell, check that the Ubuntu distribution uses version 2:

```powershell
wsl --list --verbose
```

See [Microsoft's WSL installation guide](https://learn.microsoft.com/en-us/windows/wsl/install) for an existing WSL installation or a failed setup.

## 2. Prepare the graphics driver and WSLg

Install or update the **Windows graphics driver** for your Intel, AMD or NVIDIA GPU. NVIDIA users need this step too. WSLg uses the Windows driver to display accelerated Linux graphics; follow [Microsoft's GUI application setup](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps).

For NVIDIA, follow [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html): install the Windows driver, not a Linux display driver inside WSL. The CUDA toolkit is needed only when building the CUDA sonar backend.

Update WSL from PowerShell:

```powershell
wsl --update
```

Save work in any running WSL terminals before applying a requested restart. Reopen Ubuntu after the update.

## 3. Install POSIM in the Ubuntu terminal

Use the **Ubuntu** profile in Windows Terminal for Linux commands. Check the distribution:

```bash
cat /etc/os-release
```

Follow [Ubuntu: native installation](install.md) from downloading the source through dependency installation and building. Store the workspace at `~/posim_ws` in the Linux home directory. The installer uses `sudo` and updates packages inside Ubuntu.

If image topics are visible but frames stall, use [large-image transport troubleshooting](troubleshooting.md#image-topics-appear-but-frames-stall). For subsequent experiments, [Prepare your terminals](terminals.md) explains the two Ubuntu terminal windows.

## 4. Run the demo world

Run this in the Ubuntu terminal. A Gazebo window should appear on the Windows desktop and display the ocean scene. First use may download Fuel models. Stop the simulation with Ctrl+C before starting another example.

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

Run the Linux version of QGC in the same Ubuntu distribution as POSIM. Do not open the Windows-installed application for this setup. Follow [QGC setup](qgroundcontrol.md).
