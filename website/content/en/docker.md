# Docker environment

Build an image from the same source revision you intend to run. The repository supplies separate Linux AMD64 and ARM64 recipes.

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

## Graphics, persistence and shutdown

For NVIDIA access, configure the host driver and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html), then configure the container renderer instead of forcing software rendering. CUDA sonar also needs the toolkit and plugin binaries inside the image.

Docker on macOS runs a Linux VM. Use [system requirements](requirements.md) to distinguish the container renderer from native host graphics.

`--rm` removes the container after its shell exits. Store bags and custom assets in an explicit bind mount if you want to retain them. Stop ROS viewers, stop the simulation with Ctrl+C, then exit the container shell.
