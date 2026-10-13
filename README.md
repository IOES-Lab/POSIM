# POSIM — Platform for Ocean Simulation

[![Build Lyrical / Jetty Docker image (ARM64)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-arm64v8.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-arm64v8.yml)
[![Build Lyrical / Jetty Docker image (AMD64)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM is an open-source maritime robotics simulation library maintained by
IOES-Lab. It combines ocean environments, underwater and surface robots,
simulated sensors, and ROS 2 control interfaces for Gazebo.

## Documentation

**[English documentation](https://ioes-lab.github.io/POSIM/)** ·
**[한국어 문서](https://ioes-lab.github.io/POSIM/ko/)**

Find installation instructions, first simulations, vehicle and sensor examples,
custom robot guides, and plugin references on the documentation website.
The `main` branch targets Ubuntu 26.04, ROS 2 Lyrical, and Gazebo Jetty.
Start with [installation](https://ioes-lab.github.io/POSIM/install.html) or
[Docker](https://ioes-lab.github.io/POSIM/docker.html), then run your
[first simulation](https://ioes-lab.github.io/POSIM/quickstart.html).

## Try WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) uses POSIM as its simulation engine.
Run it in a web workspace or desktop app.

- Generate seabed and coast from geographic coordinates.
- Operate surface or underwater robots and inspect sensors.
- Test ArduPilot waypoint missions or ROS 2 control code.
- Watch the public LIVE world voyage.

## Terrain, routing and control libraries

WWW-POSIM connects the POSIM engine to three public libraries. Each repository has English/Korean usage instructions and tests.

| Library | Responsibility |
| --- | --- |
| [POSIM-Terrain](https://github.com/IOES-Lab/POSIM-Terrain) | Geographic land/seabed generation and satellite textures |
| [POSIM-Routing](https://github.com/IOES-Lab/POSIM-Routing) | Global routes, coastal paths, port approaches and local collision checks |
| [POSIM-Control](https://github.com/IOES-Lab/POSIM-Control) | Vehicle propulsion, leader/follower control and motion policies |

Read the [library guide](https://ioes-lab.github.io/POSIM/libraries.html) or [한국어 안내](https://ioes-lab.github.io/POSIM/ko/libraries.html) for how they work together and how to use them directly.

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
[wave dependency and distribution details](extras/surface/README.md#submodules-and-licensing).
