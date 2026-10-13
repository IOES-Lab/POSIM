# Docker environment

Build an image from the same source revision you intend to run. The repository supplies separate Linux AMD64 and ARM64 recipes.

## Choose an image

This guide builds `posim:dev-*` images from source. Record the revision with `git rev-parse HEAD`. For published `ioeslab/posim` images, check source revision, architecture and package names, and record the digest.

## 1. Build the image on the host

Clone POSIM if you have not already done so:

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
```

For AMD64:

```bash
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
```

For ARM64, including an Apple Silicon Docker host:

```bash
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

Both recipes install the pinned external wave dependency. The ARM64 recipe also includes an RDP desktop environment; an RDP connection is not needed for the server-only examples below.

## 2. Open a headless container

Set the image to the one you built:

```bash
export POSIM_IMAGE=posim:dev-arm64-rdp
docker run --rm -it --init --name posim-quickstart --shm-size=1g \
  --entrypoint bash -e LIBGL_ALWAYS_SOFTWARE=1 \
  -e QT_QPA_PLATFORM=offscreen -e GZ_IP=127.0.0.1 "$POSIM_IMAGE"
```

For AMD64, change `POSIM_IMAGE` to `posim:dev-amd64`. This setup uses software rendering and publishes no host ports.

## 3. Source ROS and the workspace inside the container

```bash
source /opt/ros/lyrical/setup.bash
export POSIM_WS="${POSIM_WS:-${POSIM_UNDERLAY:-}}"
source "$POSIM_WS/install/setup.bash"
ros2 pkg prefix posim_demos
```

The AMD64 recipe uses `/opt/posim_ws`; the ARM64 recipe uses `/home/docker/posim_ws`. The environment variables above select the matching workspace.

## 4. Inspect from another terminal

Keep the container shell open. From a second host terminal:

```bash
docker exec -it posim-quickstart bash
```

Repeat the source commands inside this second shell, then follow [First simulation](quickstart.md). Both shells must use the same container and discovery settings.

## Rendering image sensors without a desktop

`headless:=true` starts Gazebo server mode (`-s`). Image sensors also need a rendering environment.

- Virtual display: use Xvfb below
- EGL: configure [Gazebo headless rendering](https://gazebosim.org/api/sim/10/headless_rendering.html) and `--headless-rendering`
- `QT_QPA_PLATFORM=offscreen`: configures Qt windows

For an initial software-rendering check, one option is a virtual X display. From a host terminal, install its tools in the disposable container:

```bash
docker exec -u root posim-quickstart bash -lc \
  'apt-get update && apt-get install -y --no-install-recommends xvfb xauth'
```

Then, in container Terminal A, open a shell under that display:

```bash
xvfb-run -a -s "-screen 0 1280x720x24" bash
```

Source the step-3 environment in this shell, then launch the camera example. Keep it open while terminal B checks data. Xvfb uses software rendering. POSIM launch arguments do not expose the EGL flag.

## Graphics, persistence and shutdown

For NVIDIA access, configure the host driver and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html), then configure the container renderer instead of forcing software rendering. CUDA sonar also needs the toolkit and plugin binaries inside the image.

Docker on macOS runs a Linux VM. Use [system requirements](requirements.md) to distinguish the container renderer from native host graphics.

`--rm` removes the container after its shell exits. Mount a host directory to retain recordings and assets.

1. Stop topic inspection tools.
2. Press Ctrl+C in the simulation and wait for shutdown.
3. Close the Xvfb and container shells.

Keep the virtual display available until Gazebo exits.
