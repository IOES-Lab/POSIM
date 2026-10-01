# Run POSIM with Docker

## 1. Understand which image you are using

**Status checked 2026-10-01:** the images below are public **validation
candidates**, not a POSIM 1.0 release. They come from
[PR #5](https://github.com/IOES-Lab/POSIM/pull/5), which is still unmerged.
They include candidate runtime fixes absent from `main` and do not include
WGPU from PR #6 or WAM-V from PR #7.

| Target | Published validation tag | Installed workspace |
| --- | --- | --- |
| Linux ARM64 / Apple Silicon Docker | `ioeslab/posim:validation-pr5-abad9d70-arm64-rdp` | `/home/docker/dave_ws` |
| Linux AMD64 / x86-64 | `ioeslab/posim:validation-pr5-abad9d70-amd64` | `/opt/dave_ws` |

These are separate architecture-specific tags, not a single cross-architecture
tag. `latest`, `main-amd64`, `main-arm64-rdp` and versioned release tags were
not published in this validation. Do not substitute one of those names.

Install Docker using the official instructions for
[Ubuntu](https://docs.docker.com/engine/install/ubuntu/) or
[macOS](https://docs.docker.com/desktop/setup/install/mac-install/).
Check `docker version` and `docker info` before continuing. Use Bash or zsh on
the host; on Windows, use a WSL shell with Docker Linux-container support.
The Windows host path was not part of the 2026-09-30 native-architecture checks.
Docker Desktop on Apple Silicon runs Linux in a VM; it is not native macOS
execution and does not expose the Mac's Metal backend to this Linux image.

## 2. Select one architecture and pull the pinned image

The tag names above are convenient labels. The commands below use immutable
registry digests so that you obtain the exact published candidate.

**Apple Silicon / Linux ARM64:**

```bash
export POSIM_PLATFORM=linux/arm64
export POSIM_IMAGE=ioeslab/posim@sha256:72179187c96a801184a15ab9cdaf3ca9714d3717ce04e97cd2ec60f34d83edb8
docker pull --platform "$POSIM_PLATFORM" "$POSIM_IMAGE"
```

**Linux AMD64 / x86-64:**

```bash
export POSIM_PLATFORM=linux/amd64
export POSIM_IMAGE=ioeslab/posim@sha256:9783525a2a18ecc2e275e9af43e82ccab6202bb99b63790ffd165d8885fa9784
docker pull --platform "$POSIM_PLATFORM" "$POSIM_IMAGE"
```

Use the native host architecture where possible. AMD64 execution on an ARM64
host requires emulation and is not equivalent to native AMD64 performance.
The images are large: check available Docker disk space before pulling.

## 3. Open an isolated shell

```bash
docker run --rm -it --init --name posim-quickstart \
  --platform "$POSIM_PLATFORM" --shm-size=1g \
  --entrypoint bash \
  -e LIBGL_ALWAYS_SOFTWARE=1 -e QT_QPA_PLATFORM=offscreen \
  -e GZ_IP=127.0.0.1 "$POSIM_IMAGE"
```

If a container named `posim-quickstart` already exists, choose another name;
do not delete an unrelated container. No host directory is mounted, no Docker
socket is exposed, and no RDP port is opened by this command. Exiting the shell
removes this disposable container, so copy out any files you want to keep first.

**Inside the container**, load the installed environments explicitly:

```bash
source /opt/ros/lyrical/setup.bash
export POSIM_WS="${DAVE_WS:-${DAVE_UNDERLAY:-}}"
test -n "$POSIM_WS" && test -r "$POSIM_WS/install/setup.bash"
source "$POSIM_WS/install/setup.bash"
cd /tmp
printf 'ROS_DISTRO=%s\n' "$ROS_DISTRO"
gz sim --versions
ros2 pkg prefix dave_demos
```

Expect `lyrical`, a Gazebo Sim 10.x (Jetty) version, and an installed
`dave_demos` prefix under the workspace listed above. `DAVE_WS`, `DAVE_UNDERLAY`
and the `dave_*` package names are compatibility names, not stale commands.

## 4. Start a headless world

```bash
ros2 launch dave_demos dave_world.launch.py \
  world_name:=dave_ocean_waves headless:=true
```

No GUI window is expected. In a second **host** terminal, inspect the clock:

```bash
docker exec -it posim-quickstart bash -c \
  'source /opt/ros/lyrical/setup.bash; gz topic -e -t /world/oceans_waves/clock'
```

The world name in the clock topic is `oceans_waves`, not the world filename.
Clock messages should advance. Stop the clock viewer with Ctrl+C, then stop
the launch with Ctrl+C in the first terminal and wait for the child processes
to exit. Check the complete log for segmentation faults, aborts or forced kills.
A running container alone is not a successful Quickstart.

Try the [REXROV, DVL and camera commands](quickstart.md) next, one at a time.
Those commands run inside the same sourced shell. The candidate includes a
Fuel asset cache for the checked cases. Other worlds/assets may still require
network access on first use; offline-camera success is not an all-world
offline guarantee. Server-only camera/DVL execution still needs rendering,
provided by software rendering in the recorded headless tests.

## 5. What was verified

On 2026-09-30, each published architecture was anonymously pulled by digest
and tested in fresh containers: **14/14 distinct headless Quickstarts**,
**1/1 additional camera trial with `--network none`**, installed inventory and
**5/5 installed transport churn probes** passed. Pulls could reuse cached
layers; this was not an all-layer cold-download test.

- [ARM64 successful publication and re-pull](https://github.com/IOES-Lab/POSIM/actions/runs/36693050122)
- [AMD64 successful publication job](https://github.com/IOES-Lab/POSIM/actions/runs/36679060248/job/109770314602)

The AMD64 job succeeded even though its enclosing run includes an unsuccessful
ARM64 publication attempt. The later ARM64 run linked above is the successful
record. Earlier prepublication evidence comprised 77 connected trials and
5 offline camera trials per architecture; those are a different test phase,
not the count of new re-pull tests.

GUI/RDP, joystick input, CUDA/WGPU sonar, physical sensor accuracy and execution
of every retained world are **outside this validation**. The `-rdp` suffix
describes the ARM64 image packaging, not a newly verified desktop session.

For audit, the tested application candidate is `abad9d70de843474478de5ef55371c049e40deef`.
The image revision label `32eedbacf781a0fd1e517481571e76bfaab73c51` identifies
GitHub's synthetic PR-test checkout, **not** a merge of PR #5 into `main`.
Publication-tooling commit `584953b5` did not rebuild the images.

## 6. Build a development image yourself (different source state)

From a checkout made using the [source guide](installation.md):

```bash
cd ~/posim_ws/src/dave
git rev-parse HEAD
# Choose ONE build for your target architecture:
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
# Or on ARM64:
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

These `posim:dev-*` tags are local, unpublished builds of **your checkout**.
They are not the validated registry images above. Building `main` does not
silently include PR #5's fixes. Record and test that source state independently.
For GPU/GUI requirements and the experimental WGPU path, see
[backend support](support.md). Publication settings are documented for
[maintainers](maintainer-setup.md); this Quickstart does not enable publication.
