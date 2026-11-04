# POSIM — Platform for Ocean Simulation

[![Build and publish POSIM Docker images](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM is an open-source maritime robotics simulation library maintained by
IOES-Lab. It combines ocean environments, underwater and surface robots,
simulated sensors, and ROS 2 control interfaces for Gazebo.

## Documentation

**[English documentation](https://ioes-lab.github.io/POSIM/)** ·
**[한국어 문서](https://ioes-lab.github.io/POSIM/ko/)**

Find installation instructions, first simulations, vehicle and sensor examples,
custom robot guides, and plugin references on the documentation website.
The [public feature catalogue](https://ioes-lab.github.io/POSIM/features.html)
maps implementations to their guides, including
[underwater scene effects and marine snow](https://ioes-lab.github.io/POSIM/underwater-rendering.html)
and [marine GNSS](https://ioes-lab.github.io/POSIM/gnss.html).
The `main` branch targets Ubuntu 26.04, ROS 2 Lyrical, and Gazebo Jetty.
Start by [choosing an environment](https://ioes-lab.github.io/POSIM/requirements.html), then run your
[first simulation](https://ioes-lab.github.io/POSIM/quickstart.html).

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) combines POSIM with geographic
terrain, routing and vehicle-control libraries, adding a GUI and web/desktop
applications. See its [application guide](https://www-posim.vercel.app/guide/en.html)
for those workflows.

## Docker images

Download the current image with `docker pull ioeslab/posim:latest`.
Docker selects the AMD64 or ARM64 variant for the host. For repeatable
experiments, select a `sha-<12-character-commit>` tag from
[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags).
Follow [Choose an environment](https://ioes-lab.github.io/POSIM/requirements.html)
for Linux, Windows WSL and macOS display setup before creating a container.

## Contributing

Report issues or propose changes in [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
See the [contribution guide](https://ioes-lab.github.io/POSIM/contributing.html)
for development checks and documentation updates.

## License and attribution

POSIM includes source from [Project DAVE](https://github.com/Field-Robotics-Lab/dave)
with its author and copyright notices.
POSIM-authored source uses [Apache License 2.0](LICENSE). Third-party source and
assets retain their original terms. External Wave Sim is built from its upstream
repository under GPL terms; see the
[wave dependency and distribution details](extras/surface/README.md#licensing).
