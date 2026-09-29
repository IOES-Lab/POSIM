# POSIM Docker development images

Build images from the repository root so that Docker uses the exact checkout.
These are local development tags; they do not imply a published POSIM release.

## AMD64

```bash
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
docker run --rm -it posim:dev-amd64 bash
```

The workspace remains at `/opt/dave_ws`. In an interactive Bash shell its setup
is loaded through `.bashrc`. For a non-interactive command, source
`/opt/ros/lyrical/setup.bash` and `/opt/dave_ws/install/setup.bash` explicitly.
GUI forwarding and GPU passthrough must be configured for the host before using
graphical or GPU-dependent scenarios.

## ARM64 / Apple Silicon

```bash
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
docker run --rm -it --name posim-arm64 \
  -p 127.0.0.1:13389:3389 --shm-size=2g posim:dev-arm64-rdp
```

Connect an RDP client to `localhost:13389`. The inherited development image uses
the username and password `docker`; the command above exposes RDP only on the
local machine. The workspace remains at `/home/docker/dave_ws`.

For a shell without the RDP services:

```bash
docker run --rm -it --user docker --entrypoint bash posim:dev-arm64-rdp
```

Docker Desktop on Apple Silicon runs this Linux ARM64 image. It does not make
the Mac's Metal backend available to the CUDA sonar implementation. The initial
POSIM source does not include the WGPU work from DAVE PR #44.

## Publication

`ioeslab/posim` is the configured Docker Hub destination, pending maintainer
setup and successful publication. The workflows use architecture-specific tags:
`main-amd64` and `main-arm64-rdp` for branch builds, and version tags such as
`1.0.0-amd64` and `1.0.0-arm64-rdp` after a future `v1.0.0` tag.

Do not assume these tags exist until the corresponding publication has
completed. This initial configuration does not create a combined multi-platform
manifest. See [maintainer setup](maintainer-setup.md).
