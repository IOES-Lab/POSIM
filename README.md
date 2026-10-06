# POSIM — Platform for Ocean Simulation

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

[WWW-POSIM — World Wide Web Platform for Ocean Simulation](https://www-posim.vercel.app/)
builds on POSIM, ROS 2, Gazebo, and ArduPilot to bring maritime simulation to a
web browser and a standalone application. Choose a geographic starting point,
explore real terrain with surface or underwater robots, inspect live sensor data,
and test waypoint missions or your own ROS 2 autonomy code. Visit the platform
to watch the live voyage, explore its features, and find downloads and tutorials.

## Contributing

Report issues or propose changes in [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
See the [contribution guide](https://ioes-lab.github.io/POSIM/contributing.html)
for development checks and documentation updates.

## Origins and license

POSIM continues the [DAVE](https://github.com/IOES-Lab/dave) codebase, originally
developed as [Project DAVE](https://github.com/Field-Robotics-Lab/dave), and
preserves its source history and copyright notices.
POSIM-authored source uses [Apache License 2.0](LICENSE). Third-party source and
assets retain their original terms. External Wave Sim is built from its upstream
repository under GPL terms; see the
[wave dependency and distribution details](extras/surface/README.md#submodules-and-licensing).
