# Windows: WSL + Docker

Run the prepared POSIM image in Docker Desktop and display Gazebo through WSLg. All Docker commands on this page run in the **Ubuntu terminal inside WSL**, not PowerShell.

## 1. Prepare WSL and graphics drivers

Complete the first two steps in [Windows: WSL installation](install-windows.md): install Ubuntu 26.04 under WSL2, then update WSL and your **Windows** graphics driver. This also applies to NVIDIA cards. Do not install a Linux display driver inside WSL.

Install [Docker Desktop with the WSL2 backend](https://docs.docker.com/desktop/features/wsl/). In Docker Desktop, enable **Settings → Resources → WSL Integration → Ubuntu-26.04**. Use Docker Desktop's engine; a second Docker Engine inside Ubuntu is unnecessary.

In the Ubuntu terminal, check the server and display socket:

```bash
docker version --format '{{.Server.Version}}'
test -S /mnt/wslg/.X11-unix/X0
printf '%s\n' "$DISPLAY"
```

If the socket is absent, finish the [WSLg setup](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps) before continuing.

## 2. Download POSIM

```bash
docker pull ioeslab/posim:latest
```

The [Docker environment guide](docker.md) explains image versions and source builds. The initial setup below uses software rendering. For supported NVIDIA hardware, follow the linked [Docker Desktop GPU requirements](docker.md#graphics-and-gpu-access) before using GPU-dependent examples.

## 3. Create the container

```bash
docker run -d --name posim --init --shm-size=1g \
  -e DISPLAY -e QT_QPA_PLATFORM=xcb -e LIBGL_ALWAYS_SOFTWARE=1 \
  --mount type=bind,source=/mnt/wslg/.X11-unix,target=/tmp/.X11-unix,readonly \
  --mount type=volume,source=posim-data,target=/work \
  --entrypoint bash ioeslab/posim:latest -lc 'exec sleep infinity'
```

Save your own files under `/work` to keep them in the `posim-data` volume. Stop and restart the container with `docker stop posim` and `docker start posim`.

For later examples, open [two Ubuntu terminals](terminals.md) and connect both to this container. Gazebo appears as a Windows desktop window; Docker does not require a separate remote desktop here.

## 4. Launch the demo world

Enter the container:

```bash
docker exec -it posim bash
```

Run the final installation check inside it:

```bash
source /opt/ros/lyrical/setup.bash
source /opt/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

Gazebo should open the ocean world. Leave the terminal open while it runs; press **Ctrl+C** to stop. Next, [launch your first robot](quickstart.md).

Run the Linux version of QGC inside the same POSIM container. Do not open QGC on the Windows host or outside the container in WSL. Follow [QGC setup](qgroundcontrol.md).
