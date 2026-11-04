# Install on Windows

Use Windows Subsystem for Linux 2 (WSL 2) to run POSIM in Ubuntu 26.04. Commands in the Ubuntu guide run inside the Linux distribution, not in PowerShell.

## 1. Install WSL and Ubuntu

In PowerShell opened as Administrator, install WSL and Ubuntu 26.04:

```powershell
wsl --install -d Ubuntu-26.04
```

Restart Windows if prompted. Start Ubuntu and create its Linux username and password. If WSL is already installed, [Microsoft's WSL instructions](https://learn.microsoft.com/en-us/windows/wsl/install) describe installing another distribution and troubleshooting. To check the installed distribution and WSL version, run in PowerShell:

```powershell
wsl --list --verbose
```

The Ubuntu distribution should show version 2. In its Ubuntu terminal, confirm the release:

```bash
cat /etc/os-release
```

## 2. Install POSIM inside Ubuntu

In the Ubuntu terminal, follow [Install on Ubuntu](install.md) from **Get the source** through **Build and source**. Use the Linux home directory for the workspace. Review the installer first: it uses `sudo`, configures repositories and runs a full system package upgrade inside the distribution.

## 3. Run and view a simulation

In each Ubuntu terminal, source the environment as described in the Ubuntu guide. Then follow [First simulation](quickstart.md). For a Gazebo window, use WSLg with a compatible graphics driver; see Microsoft's [Linux GUI app setup](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps). For a world launch, set `headless:=false`; for robot and sensor launches, set `gui:=true headless:=false`. Keep the terminal open until the launch stops.

WSL rendering and DDS network behavior depend on the Windows and GPU configuration. If image topics appear but frames stall, follow the optional [large-image transport setup](troubleshooting.md#image-topics-appear-but-frames-stall). Multibeam sonar also needs a compatible NVIDIA/CUDA setup as described in [sonar setup](sonar-tuning.md).

For container-based operation, use [Docker environment](docker.md) instead of installing the POSIM stack directly in Ubuntu.
