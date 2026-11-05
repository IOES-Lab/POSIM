# Docker environment

POSIM images contain Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, MAVROS and QGroundControl. Choose the host-specific setup in [Choose an environment](requirements.md) for display and driver configuration.

## Download a published image

Use [POSIM on Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags). Docker selects the AMD64 or ARM64 image for your computer:

```bash
docker pull ioeslab/posim:latest
export POSIM_IMAGE=ioeslab/posim:latest
```

| Tag | Use |
|---|---|
| `latest` | The published main build, with AMD64 and ARM64 variants |
| `sha-<12-character-commit>` | Select a specific source revision; copy an existing tag from Docker Hub |
| `sha-<12-character-commit>-amd64` / `-arm64` | Select one architecture explicitly |
| Release version, when published | Select a tagged release |

The image label records the full source revision. Record the digest for an exact image when repeating experiments:

```bash
docker image inspect "$POSIM_IMAGE" \
  --format '{{index .Config.Labels "org.opencontainers.image.revision"}}'
docker image inspect "$POSIM_IMAGE" --format '{{json .RepoDigests}}'
```

Use documentation matching the selected revision. Pulling a new image does not update an existing container; stop and replace that container when ready to use the new image.

## Build from source

For a local modification, build the checkout you intend to run. On the host:

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
git rev-parse HEAD
```

Choose the build matching your CPU:

```bash
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
```

```bash
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

Use the local tag in place of `ioeslab/posim:latest` in your OS installation guide. Both images build the supplied source and pinned wave dependency. The ARM64 image also supplies an XFCE desktop and an RDP server.

## Start in your operating system

- [Linux: Docker installation](install-ubuntu-docker.md): use the Linux desktop's display connection.
- [Windows: WSL + Docker](install-windows-docker.md): use WSLg to display Linux windows.
- [macOS: Docker + RDP](install-macos.md): connect to the image's Linux desktop.

Each guide creates a container named `posim`. [Prepare your terminals](terminals.md) explains where to open commands and load the workspace.

## Workspaces and saved data

The AMD64 image uses `/opt/posim_ws`; the ARM64 image uses `/home/docker/posim_ws`. The images expose their workspace through `POSIM_WS` or `POSIM_UNDERLAY`.

The OS guides mount the Docker volume `posim-data` at `/work`. Save standalone SDF files and recordings there. Files elsewhere in a container survive a stop/start, but removing that container removes those files. Commit modified source or copy it out before replacing a container. A volume stays separate from the container until you explicitly remove the volume.

## Stop, return and update {#update-an-existing-container}

Stop the simulation with Ctrl+C in its terminal, then run Docker commands from the host:

```bash
docker stop posim
docker start posim
```

To update, save your work, stop the container, pull the selected image, remove the stopped container and repeat your OS guide's creation command. Reuse `posim-data` to retain files under `/work`.

## Graphics and GPU access

For GPU access on Linux, configure the host driver and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). Windows uses the [Docker Desktop WSL 2 GPU configuration](https://docs.docker.com/desktop/features/gpu/). CUDA sonar also needs its libraries inside the image; follow [Configure & tune sonar](sonar-tuning.md). The initial display recipes use software rendering and can be slower than a configured GPU.
