# Automatic external waves and optional WAM-V

This redesign follows [POSIM PR #7](https://github.com/IOES-Lab/POSIM/pull/7). Wave Sim stays a pinned **external build dependency**. Its wave solver, hydrodynamics implementation, meshes and textures are fetched from the upstream repository; none is copied into POSIM's source tree. `wamv.py` reads the external model at runtime and applies vehicle integration configuration. The original BlueROV2 launch remains unchanged. Both standard Docker image recipes and the Ubuntu stack installer now build the pinned wave dependency automatically. Installing the plugin does not implicitly replace the buoyancy systems in existing worlds.

## Build and run

This optional Dockerfile currently targets the ARM64 base layout (`docker` user, `/home/docker/ardupilot`, `/home/docker/posim_ws`). Build the ARM64 image as described in the [Docker guide](../../website/content/en/docker.md); Wave Sim is included by default, with no separate wave installation command. The optional surface overlay adds ArduRover and the WAM-V launch, reusing the wave installation when its recipe matches. From the repository root:

```sh
docker build -f extras/surface/Dockerfile --build-arg POSIM_BASE_IMAGE=posim:dev-arm64-rdp -t posim:surface .
docker run --rm -it --entrypoint bash posim:surface -lc 'source /opt/ros/lyrical/setup.bash; source /home/docker/posim_ws/install/setup.bash; ros2 launch /opt/posim-surface/wamv.launch.py headless:=true'
```

The same image contains **ArduSub and ArduRover** as separate binaries. Select one controller per vehicle/session; running both on the same ports is not supported by this single-vehicle example. Rover's `FRAME_CLASS=2` selects a boat. Independent left/right servo outputs 1/3 drive the aft propellers, with fixed engine steering joints. The demo starts on the surface. Native GUI use requires the usual display/GPU configuration.

The default parameters disable GPS and select external navigation. `external_navigation.py` explicitly publishes **simulated Gazebo ground truth** as MAVROS vision pose/speed and an EKF origin for this standalone demo. Never bypass failed navigation/arming checks. The downstream ocean service supplies explicitly simulated Gazebo external navigation and its bounded waypoint sequencer; it is not a real positioning sensor. QGC/MAVROS remain optional client interfaces.

Wave controls use `gz.msgs.Param` on `/world/wwos_gebco/waves`: double-valued `wind_speed`, `wind_angle` (degrees), and `steepness`. Configure the visual and physics model with matching parameters. A periodic 256 m field is used; its 768 m visual region can be recentered by whole periods. This is a spectral wave model, not a tide/current/weather forecast.

## Dependency maintenance and distribution

`dependency.json` pins the reviewed source revision. `extras/install-waves.sh` fetches that commit, compiles it and installs libraries under `/opt/waves`. It is called from the standard ARM64 Dockerfile and the Ubuntu installer (also used by the AMD64 Dockerfile). This is a source build dependency, not a Git submodule; users do not need `--recurse-submodules` for Wave Sim. `patch_external_waves.py` adapts CMake/package targets, library ordering and symbol visibility for Jetty's unversioned vendor packages, without copying the solver. Retain the patched upstream source and all GPL notices alongside distributed binaries; the Docker build does so under `/opt/asv_wave_sim`. CGAL/FFTW also have license obligations. Externalizing code solves the in-repository fork/maintenance problem; **it does not automatically remove GPL obligations for combined or distributed software**. Review the exact distribution before release. Fetching a dependency here is not a claim that all POSIM files must change license, or that any distribution is exempt.

Upstream WAM-V assets reference OSRF VRX. Their source and notices remain in the external checkout. Updating the dependency is explicit: change the SHA, rebuild, validate loaded libraries, camera rendering, buoyancy, thruster signs, navigation and teardown on each architecture.

## Validation

The new shared installer built the external dependency from a fresh checkout on ARM64, then a second invocation verified idempotent reuse. Gazebo loaded the new WavesModel and Hydrodynamics libraries. The external dependency built on ARM64 Ubuntu 26.04 / ROS 2 Lyrical / Gazebo Jetty, and ArduSub/ArduRover coexist in the same Docker image. The downstream generated-world test loaded actual FFT wave rendering and verified terrain with RGBD and physical collision probes. WAM-V controller/waypoint results and remaining limits are recorded in `VALIDATION.md`; no AMD64 or native GPU result is inferred from ARM64 software rendering.

## Submodules and licensing

A Git submodule records a separate repository URL and commit; it does not compile or install it. A submodule workflow would need `git submodule update --init --recursive` plus explicit CMake build/install steps. Fetching the pinned upstream revision during the image build gives the same automatic installation without requiring a recursive user clone. See [Git documentation](https://git-scm.com/docs/git-submodule).

Using a submodule is not inherently a GPL violation and is not an exemption either. Preserve the upstream license/notices, corresponding source and build modifications. GPL treatment depends on whether distributed components are separate works or one combined program, especially when plugins share an address space; repository boundaries do not decide that. POSIM-authored source retains Apache-2.0 notices. Apache-2.0 is compatible with GPLv3, but that does not permit distributing GPL wave code under Apache-2.0 alone. See [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.en.html#MereAggregation), [upstream license](https://github.com/srmainwaring/asv_wave_sim/blob/ca8629df4e191235753dfae92ef725d30b923364/LICENSE), and [Apache compatibility guidance](https://www.apache.org/licenses/GPL-compatibility.html). Docker image labels now declare both license families; this is a component inventory, not a complete legal determination of a distribution.
