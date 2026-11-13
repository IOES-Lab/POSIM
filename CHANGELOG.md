# Changelog

Record changes under **Unreleased** as they land. Move those entries to a
versioned section and add the publication date when that version ships.

## Unreleased

## 1.0.1 - Unreleased

Release candidate; the publication date will be recorded when the release ships.

[Read the release notes](docs/releases/v1.0.1.md).

### Fixed

- Restore Wave Sim model/library paths and the terrain resource overlay in fresh
  Apple Silicon RDP desktop sessions.
- Preserve the active Ogre2 material when dynamic geometry buffers change, and
  unmap persistent vertex buffers before releasing them in gz-rendering and Wave Sim.
- Build and select the pinned rendering corrections in the normal Ubuntu/WSL
  installer and both Docker architectures, retaining ROS-owned libraries.
- Give the corrected renderer priority after sourcing ROS underlays, including
  when an RDP session already contains its library paths.
- Retain APT package indexes after Wave Sim installation so the next native
  workspace dependency step can find its packages.
- Exclude standalone renderer tests from colcon package discovery and resolve
  their Wave Sim library dependencies during native linking.

### Changed

- Run AMD64 Docker builds on GitHub-hosted Ubuntu 26.04, alongside the existing
  GitHub-hosted ARM64 builds. All POSIM workflows now use GitHub-hosted runners.
- Pin the image builder versions and remove persistent-server image pruning.

### Added

- Versioned Docker publication that verifies the release commit in both
  architectures and aliases its existing image digest without rebuilding.
- A graphical overview in all eight language editions of the project README.

### Tests

- Exercise material transitions and the installed renderer in image acceptance tests.

## 1.0.0

First stable release of POSIM, the Platform for Ocean Simulation.
[Read the release notes](docs/releases/v1.0.0.md).

### Added

- Ocean worlds, Wave Sim surface waves, uniform and stratified currents,
  terrain, and underwater rendering for maritime robotics simulation.
- REXROV, BlueROV2, Slocum glider and WAM-V examples with ROS 2 control and
  observation interfaces.
- Shallow BlueROV2 inspection and WAM-V buoy-course tutorials, with camera,
  optional sonar or lidar, configured RViz views and QGroundControl video.
- Shared underwater appearance for sensor cameras and the shallow tutorial's
  Gazebo view, including marine snow and gentle Wave Sim surfaces.
- Marine GNSS input for all three BlueROV2 ArduSub profiles, with submerged
  no-fix behavior, reacquisition and altitude/velocity conversion.
- Underwater camera, sea pressure, DVL, USBL, marine GNSS and multibeam sonar
  models. Sonar supports CPU, WGPU and CUDA where the selected platform and
  toolchain support them.
- Documentation in eight languages for installation, first simulations,
  sensor recording, plugin integration and custom models, with tutorial captures
  and links to the separate REXROV control example.

### Runtime

- ROS 2 Lyrical and Gazebo Jetty on Ubuntu 26.04, with AMD64 and ARM64 Docker
  recipes. Windows uses WSL 2, and Apple Silicon uses Docker with RDP.
- ArduSub and ArduRover SITL 4.7.2. Lockstep sensor interpolation preserves
  the Gazebo physics timestep, and the WAM-V example uses MAVROS vision pose
  and speed with all arming checks enabled.
- Scoped MAVROS Router, QGroundControl video and Ogre2 Terra corrections
  for the documented dependency versions.
- An isolated RViz 15.2.6 overlay in the Ubuntu/WSL installer and both Docker
  images stops updates after accepted close and releases view-owned ROS resources
  during shutdown. Cancel keeps updates running; system RViz packages are unchanged.
- QGroundControl 5.1.5 in both Docker images, with verified archive checksums.
- A smaller calm-wave grid reduces rendering work in ocean demos. Local QGC
  streams send a keyframe for every image to reduce video startup and recovery
  delays at low simulation frame rates.
