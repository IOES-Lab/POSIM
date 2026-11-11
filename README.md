# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![Build and publish POSIM Docker images](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM is an open-source maritime robotics simulation library maintained by
[IOES Lab. at Korea Maritime & Ocean University (KMOU)](https://lab.wschoi.com).
It combines ocean environments, underwater and surface robots,
simulated sensors, and ROS 2 control interfaces for Gazebo Sim.

## Documentation

**[Read the documentation](https://ioes-lab.github.io/POSIM/)**

[Changelog](CHANGELOG.md)

POSIM uses ROS 2 Lyrical and Gazebo Sim Jetty. Choose an installation route:

- **Ubuntu 26.04:** [native installation](https://ioes-lab.github.io/POSIM/install.html), the recommended setup. [Linux Docker](https://ioes-lab.github.io/POSIM/install-ubuntu-docker.html) provides another route for Linux systems.
- **Windows:** [WSL](https://ioes-lab.github.io/POSIM/install-windows.html) or [WSL + Docker](https://ioes-lab.github.io/POSIM/install-windows-docker.html).
- **macOS:** [Docker + RDP](https://ioes-lab.github.io/POSIM/install-macos.html).

Compare the routes in [Choose an environment](https://ioes-lab.github.io/POSIM/requirements.html),
then [prepare your terminals](https://ioes-lab.github.io/POSIM/terminals.html)
and run your [first simulation](https://ioes-lab.github.io/POSIM/quickstart.html).

The [feature catalogue](https://ioes-lab.github.io/POSIM/features.html)
links robot, world, sensor and control examples to their guides, including
[underwater scene effects and marine snow](https://ioes-lab.github.io/POSIM/underwater-rendering.html)
and [marine GNSS](https://ioes-lab.github.io/POSIM/gnss.html).

## Docker images

Download the current image:

```bash
docker pull ioeslab/posim:latest
```

Docker selects the AMD64 or ARM64 variant for the host. For repeatable
experiments, select a `sha-<12-character-commit>` tag from
[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags).
Follow [Choose an environment](https://ioes-lab.github.io/POSIM/requirements.html)
for Linux, Windows WSL and macOS display setup before creating a container.

## Related application

[WWW-POSIM](https://www-posim.vercel.app/) combines POSIM with geographic
terrain, routing and vehicle-control libraries, adding a GUI and web/desktop
applications. Its own documentation covers those application workflows.

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
