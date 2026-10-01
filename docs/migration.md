# Moving from DAVE to POSIM

POSIM means **Platform for Ocean Simulation**. Development continues at
[IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM), on `main`.
Use the [documentation index](README.md), [Ubuntu installation](installation.md)
or [Docker candidate Quickstart](docker.md) for new installations.

The [original DAVE Wiki](https://dave-ros2.notion.site) documents an older
DAVE state. Its historical `ros2` branch commands, Docker image names and
release claims should not be read as current POSIM instructions. The
[working POSIM guide](https://caring-dibble-be5.notion.site/3ecc941998988150ad59f75d5bd105cf) records the transition and current limitations.
Keep the original DAVE references and attribution for the inherited work.

## Compatibility names that should not be renamed by hand

- ROS packages such as `dave_demos`, `dave_worlds` and `dave_sensor_models`.
- Launch filenames such as `dave_world.launch.py` and `dave_robot.launch.py`.
- The source checkout key `src/dave` used by repository import.
- Image workspace paths `/opt/dave_ws` and `/home/docker/dave_ws`.
- Environment variables `DAVE_EXTRAS_DIR`, `DAVE_WS` and `DAVE_UNDERLAY`.

See [compatibility](compatibility.md). Do not globally replace `dave` with
`posim` in executable commands or installed resource references.

## Open work and release status

- [PR #5](https://github.com/IOES-Lab/POSIM/pull/5): Docker/runtime fixes and
  published validation images, still unmerged.
- [PR #6](https://github.com/IOES-Lab/POSIM/pull/6): WGPU sonar transferred
  from DAVE #44, still under review.
- [PR #7](https://github.com/IOES-Lab/POSIM/pull/7): ocean waves/WAM-V
  transferred from DAVE #27, still under review.

The old PR discussions remain linked from the new PRs. New issues and review
belong in POSIM. These development checkpoints do not announce POSIM 1.0.

## Notice for the original DAVE Wiki maintainer

The original public Wiki and the working personal Wiki are different sites.
This notice is prepared for the original site's editor; its presence here
does **not** mean that the original Wiki has already been updated:

> This DAVE documentation is outdated for new installations. Development now
> continues as **POSIM — Platform for Ocean Simulation** at
> [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM). Please start with the
> [POSIM installation guide](https://github.com/IOES-Lab/POSIM/blob/main/docs/installation.md)
> and [working Quickstart/backend guide](https://caring-dibble-be5.notion.site/3ecc941998988150ad59f75d5bd105cf).
> The existing pages are retained as historical DAVE documentation. Published
> validation images are not an official POSIM 1.0 release.
