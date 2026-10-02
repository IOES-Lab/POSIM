# Optional WAM-V and external waves

This redesign follows [POSIM PR #7](https://github.com/IOES-Lab/POSIM/pull/7). Wave Sim stays a pinned **external build dependency**. Its wave solver, hydrodynamics implementation, meshes and textures are fetched from the upstream repository; none is copied into POSIM's source tree. `wamv.py` reads the external model at runtime and applies vehicle integration configuration. The original BlueROV2 launch and base Dockerfiles remain unchanged.

## Build and run

This optional Dockerfile currently targets the ARM64 base layout (`docker` user, `/home/docker/ardupilot`, `/home/docker/dave_ws`). Build the ARM64 image as described in `docs/docker.md`, then from the repository root:

```sh
docker build -f extras/surface/Dockerfile --build-arg POSIM_BASE_IMAGE=posim:dev-arm64-rdp -t posim:surface .
docker run --rm -it --entrypoint bash posim:surface -lc 'source /opt/ros/lyrical/setup.bash; source /home/docker/dave_ws/install/setup.bash; ros2 launch /opt/posim-surface/wamv.launch.py headless:=true'
```

The same image contains **ArduSub and ArduRover** as separate binaries. Select one controller per vehicle/session; running both on the same ports is not supported by this single-vehicle example. Rover's `FRAME_CLASS=2` selects a boat. Independent left/right servo outputs 1/3 drive the aft propellers, with fixed engine steering joints. The demo starts on the surface. Native GUI use requires the usual display/GPU configuration.

The default parameters disable GPS and select external navigation. `external_navigation.py` explicitly publishes **simulated Gazebo ground truth** as MAVROS vision pose/speed and an EKF origin for this standalone demo. Never bypass failed navigation/arming checks. The downstream ocean service supplies explicitly simulated Gazebo external navigation and its bounded waypoint sequencer; it is not a real positioning sensor. QGC/MAVROS remain optional client interfaces.

Wave controls use `gz.msgs.Param` on `/world/wwos_gebco/waves`: double-valued `wind_speed`, `wind_angle` (degrees), and `steepness`. Configure the visual and physics model with matching parameters. A periodic 256 m field is used; its 768 m visual region can be recentered by whole periods. This is a spectral wave model, not a tide/current/weather forecast.

## Dependency maintenance and distribution

`dependency.json` pins the reviewed source revision. `patch_external_waves.py` adapts CMake/package targets, library ordering and symbol visibility for Jetty's unversioned vendor packages, without copying the solver. Retain the patched upstream source and all GPL notices alongside distributed binaries; the Docker build does so under `/opt/asv_wave_sim`. CGAL/FFTW also have license obligations. Externalizing code solves the in-repository fork/maintenance problem; **it does not automatically remove GPL obligations for combined or distributed software**. Review the exact distribution before release. Fetching a dependency here is not a claim that all POSIM files must change license, or that any distribution is exempt.

Upstream WAM-V assets reference OSRF VRX. Their source and notices remain in the external checkout. Updating the dependency is explicit: change the SHA, rebuild, validate loaded libraries, camera rendering, buoyancy, thruster signs, navigation and teardown on each architecture.

## Validation

The external dependency built on ARM64 Ubuntu 26.04 / ROS 2 Lyrical / Gazebo Jetty, and ArduSub/ArduRover coexist in the same Docker image. The downstream generated-world test loaded actual FFT wave rendering and verified terrain with RGBD and physical collision probes. WAM-V controller/waypoint results and remaining limits are recorded in `VALIDATION.md`; no AMD64 or native GPU result is inferred from ARM64 software rendering.
