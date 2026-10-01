# POSIM documentation

POSIM means **Platform for Ocean Simulation** and continues the DAVE codebase.
The development target is Ubuntu 26.04, ROS 2 Lyrical and Gazebo Jetty.
Existing `dave_*` package names are retained for compatibility.

## Choose your starting point

- **Try the published candidate:** [Docker Quickstart](docker.md). Select the
  image for your architecture and run from its installed workspace.
- **Develop from source on Ubuntu:** [installation](installation.md), then
  [Quickstart](quickstart.md).
- **Check sonar availability first:** [CUDA, WGPU and host limitations](support.md).
- **Move from DAVE:** [migration notice](migration.md) and
  [compatibility](compatibility.md).
- **Maintain builds or prepare a release:** [maintainer setup](maintainer-setup.md).

## Status, checked 2026-10-01

POSIM 1.0 has not been released. Two architecture-specific **validation tags**
were published on 2026-09-30 from the candidate in
[PR #5](https://github.com/IOES-Lab/POSIM/pull/5). The candidate includes runtime
fixes not yet merged into `main`. Do not apply its runtime results to a fresh
`main` build or assume that `latest`, `main-*` or release tags exist.

WGPU sonar is under review in [PR #6](https://github.com/IOES-Lab/POSIM/pull/6).
Ocean waves/WAM-V work is under review in
[PR #7](https://github.com/IOES-Lab/POSIM/pull/7). Neither is included in the
published PR #5 validation images.
