# POSIM — Platform for Ocean Simulation

POSIM is an open-source maritime robotics simulation library maintained by
IOES-Lab. It brings together ocean worlds, underwater vehicles, sensors, and
manipulation tasks for ROS 2 and Gazebo.

POSIM continues the [DAVE](https://github.com/IOES-Lab/dave) codebase, originally
developed as [Project DAVE](https://github.com/Field-Robotics-Lab/dave). This
repository preserves the source history, existing copyright notices, and
Apache-2.0 license. See [origin and compatibility](docs/compatibility.md).

## Current development version

The `main` branch targets **Ubuntu 26.04, ROS 2 Lyrical, and Gazebo Jetty**.
The initial POSIM import is based on DAVE `ros2` commit
[`8a3f6ab`](https://github.com/IOES-Lab/dave/commit/8a3f6abc2ba14ced787aff8befa0201f6c80ca8c).
POSIM 1.0 has not been released.

Existing ROS package names and launch commands remain available. For example,
the demonstration package is still called `dave_demos`. The initial import
contains the CUDA sonar implementation; the WGPU work in
[POSIM PR #6](https://github.com/IOES-Lab/POSIM/pull/6) (moved from DAVE #44) is not included in `main`.
Without a CUDA toolkit, the CUDA-specific sonar targets are skipped during
configuration. A successful build on ARM64 therefore does not establish sonar
availability.

## Get started

1. Start with the [documentation index](docs/README.md). Follow the
   [Ubuntu source guide](docs/installation.md), or use a
   [published validation image](docs/docker.md). The PR #5 images are not a
   POSIM release and include fixes not yet merged into `main`.
2. Open a terminal with the installed ROS and workspace environments loaded.
3. Start a world:

   ```bash
   ros2 launch dave_demos dave_world.launch.py world_name:=dave_ocean_waves
   ```

For a server-only run, append `headless:=true`. On a desktop with rendering
available, start REXROV with:

```bash
ros2 launch dave_demos dave_robot.launch.py \
  namespace:=rexrov world_name:=dave_ocean_waves z:=-5 paused:=false
```

World and model references may download assets from Gazebo Fuel on first use.
More examples are in [the demo guide](examples/dave_demos/README.md).

## Library contents

- `models/`: world, vehicle, sensor, and object descriptions and their resources.
- `gazebo/`: model, sensor, world, ROS bridge, and multibeam sonar plugins.
- `examples/dave_demos/`: world, robot, sensor, and object launch entry points.
- `dave_interfaces/`: ROS messages and services used by the library.
- `extras/`: dependency installation, repository manifests, and development tools.

The [legacy DAVE Wiki](https://dave-ros2.notion.site) remains available for
background material. Follow this repository's installation instructions for
POSIM; the legacy Wiki is not a POSIM release manual. See the
[migration notice](docs/migration.md) and [CUDA/WGPU support limits](docs/support.md).

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

Maintainers should read [CI and image configuration](docs/maintainer-setup.md)
before enabling Docker builds or publication in this new repository.

## License

[Apache License 2.0](LICENSE). Existing third-party notices remain with their
source files and assets.
