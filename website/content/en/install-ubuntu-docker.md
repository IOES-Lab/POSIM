# Linux: Docker installation

Use the published POSIM image when your Linux desktop has a different Ubuntu version or another distribution. This guide uses the **AMD64 image on an x86-64 desktop**, with Gazebo displayed through X11 or XWayland. For Ubuntu 26.04, [native installation](install.md) is the recommended route.

## 1. Prepare Docker and the desktop

Install [Docker Engine for your Linux distribution](https://docs.docker.com/engine/install/). On Ubuntu, follow the [Ubuntu instructions](https://docs.docker.com/engine/install/ubuntu/). Complete Docker's [Linux post-installation steps](https://docs.docker.com/engine/install/linux-postinstall/) if your account cannot run `docker`.

Open a terminal in your desktop session. Install `xauth` with your distribution's package manager. On Ubuntu:

```bash
sudo apt update
sudo apt install -y xauth
```

The following checks should print your display address and the Docker server version:

```bash
printf '%s\n' "$DISPLAY"
docker version --format '{{.Server.Version}}'
```

## 2. Download POSIM

```bash
docker pull ioeslab/posim:latest
```

The [Docker environment guide](docker.md) explains source revision tags, image inspection and building from source. The commands below use software rendering so that the first launch does not depend on a vendor-specific container GPU setup. Rendering can be slower than a native installation.

## 3. Allow this container to use your display

Copy your session's X11 authorization into a file readable only by your account. This lets the container connect without opening the X server to other users:

```bash
mkdir -p "$HOME/.cache/posim"
export POSIM_XAUTH="$HOME/.cache/posim/xauthority"
touch "$POSIM_XAUTH"
chmod 600 "$POSIM_XAUTH"
xauth nlist "$DISPLAY" | sed -e 's/^..../ffff/' | xauth -f "$POSIM_XAUTH" nmerge -
test -s "$POSIM_XAUTH"
```

If the last command fails, check that this terminal belongs to your logged-in desktop session and that XWayland is available. Do not proceed with an empty authorization file.

## 4. Create the container

```bash
docker run -d --name posim --init --shm-size=1g \
  -e DISPLAY -e QT_QPA_PLATFORM=xcb -e LIBGL_ALWAYS_SOFTWARE=1 \
  -e XAUTHORITY=/tmp/posim.xauth \
  --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix,readonly \
  --mount "type=bind,source=$POSIM_XAUTH,target=/tmp/posim.xauth,readonly" \
  --mount type=volume,source=posim-data,target=/work \
  --entrypoint bash ioeslab/posim:latest -lc 'exec sleep infinity'
```

Save your own models and recordings under `/work`; the named volume survives container replacement. Use `docker stop posim` and `docker start posim` to stop and restart this container. After a new desktop login, refresh the authorization file if the display connection fails.

For subsequent exercises, [prepare two terminals](terminals.md), both connected to this same container. Review [GPU rendering](docker.md#graphics-and-gpu-access) before enabling hardware acceleration or CUDA sonar.

## 5. Launch the demo world

Enter the container:

```bash
docker exec -it posim bash
```

In that terminal, run the final installation check:

```bash
source /opt/ros/lyrical/setup.bash
source /opt/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

A Gazebo window should show the ocean world. Keep this terminal open while Gazebo runs; press **Ctrl+C** to stop it. Continue with [your first robot simulation](quickstart.md).
