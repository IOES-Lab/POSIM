# POSIM — Platform for Ocean Simulation

POSIM is an open-source maritime robotics simulation library maintained by
IOES-Lab. It brings together ocean worlds, underwater vehicles, sensors, and
manipulation tasks for ROS 2 and Gazebo.

POSIM continues the [DAVE](https://github.com/IOES-Lab/dave) codebase, originally
developed as [Project DAVE](https://github.com/Field-Robotics-Lab/dave). This
repository preserves the source history, existing copyright notices, and
Apache-2.0 license.

## Current development version

The `main` branch targets **Ubuntu 26.04, ROS 2 Lyrical, and Gazebo Jetty**.
The initial POSIM import is based on DAVE `ros2` commit
[`8a3f6ab`](https://github.com/IOES-Lab/dave/commit/8a3f6abc2ba14ced787aff8befa0201f6c80ca8c).
POSIM 1.0 has not been released.

ROS packages, plugin namespaces and launch files use the `posim_` prefix.
Rebuild in a clean workspace when migrating from the earlier `dave_` names.
Existing third-party source attribution is retained.

## Get started

1. Run `extras/ros-lyrical-gz-jetty-install.sh`, or build `.docker/lyrical.amd64.dockerfile` / `.docker/lyrical.arm64v8.dockerfile` from this checkout.
2. Open a terminal with the installed ROS and workspace environments loaded.
3. Start a world:

   ```bash
   ros2 launch posim_demos posim_world.launch.py world_name:=posim_ocean_waves
   ```

For a server-only run, append `headless:=true`. On a desktop with rendering
available, start REXROV with:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false
```

World and model references may download assets from Gazebo Fuel on first use.
More examples are in [the demo guide](examples/posim_demos/README.md).

## Library contents

- `models/`: world, vehicle, sensor, and object descriptions and their resources.
- `gazebo/`: model, sensor, world, ROS bridge, and multibeam sonar plugins.
- `examples/posim_demos/`: world, robot, sensor, and object launch entry points.
- `posim_interfaces/`: ROS messages and services used by the library.
- `extras/`: dependency installation, repository manifests, and development tools.

## Documentation

Read the [English guide](website/content/en/index.md) or
[한국어 가이드](website/content/ko/index.md) for installation, examples, custom
models and plugin references. The guides adapt the IOES-Lab Notion Wiki to
the current source layout.

The [documentation website](website/README.md) uses the WWW-POSIM documentation
design and provides search, command copying and mobile navigation. It builds
to static HTML and can be deployed independently on Vercel or another static
host. Building it does not build or run Gazebo.

## Contributing

Open issues and pull requests in [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
Include the source revision, operating system, architecture, launch command,
and relevant logs when reporting a problem. Install and run the repository's
pre-commit hooks before submitting changes:

```bash
python3 -m venv ~/.venvs/posim-dev
source ~/.venvs/posim-dev/bin/activate
python3 -m pip install pre-commit
pre-commit install
pre-commit run --all-files
```


## License

POSIM-authored source uses [Apache License 2.0](LICENSE). Existing third-party
notices remain with their source files and assets. Docker builds also install
external Wave Sim under its upstream GPL terms, retaining corresponding source
and notices; the whole image is not licensed under Apache-2.0 alone. See
[dependency and distribution details](extras/surface/README.md#submodules-and-licensing).

## Waves and optional surface drones

The normal Docker image builds and Ubuntu stack installer automatically fetch and build the pinned external wave plugin. [WAM-V / ArduRover](extras/surface/README.md) remains an opt-in vehicle overlay. This redesign references PR #7 while keeping the wave solver in its upstream repository and preserving the regular BlueROV2 launch defaults.
