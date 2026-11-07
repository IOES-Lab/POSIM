# Automatic external waves and optional WAM-V

Wave Sim is a pinned external build dependency. The standard Docker recipes and Ubuntu installer build it automatically. The upstream checkout supplies wave, hydrodynamics and WAM-V assets. `wamv.py` reads that model and configures its controller and propulsion.

- Dependency source: `/opt/asv_wave_sim`
- Installed libraries: `/opt/waves`
- Optional surface overlay: ArduRover and WAM-V
- World setup: include the wave and buoyancy systems in the selected SDF

## Build and run

This optional Dockerfile targets the ARM64 desktop layout. Follow the
[WAM-V guide](../../website/content/en/surface.md#build-the-wam-v-integration)
to build from `ioeslab/posim:latest`, connect through RDP and launch the GUI.
Wave Sim is included in the base image. The overlay adds ArduRover and the
WAM-V launch, reusing the wave installation when its recipe matches.

The image contains separate ArduSub and ArduRover binaries. Run one instance of this example per environment: it uses fixed generated-file paths, vehicle names and controller ports. Changing `world_name` alone does not isolate another instance. Multiple vehicles need a custom integration with separate files, names, controller identities and ports, or separate isolated environments.

The controller starts after model creation and the first IMU sample. A failed model-creation process, model creation exceeding 180 seconds, or an IMU wait exceeding 180 seconds stops the launch with a nonzero status and reports the failed stage. Inspect the simulator and sensor logs before retrying.

- Rover boat frame: `FRAME_CLASS=2`
- Left/right propellers: servo outputs **1 / 3**
- Start position: water surface
- GUI: requires a display and renderer
- Navigation: GPS disabled; simulated external navigation enabled

The external-navigation adapter publishes **simulated Gazebo pose and speed** through MAVROS. Check navigation and arming status before sending commands. QGroundControl and MAVROS provide client interfaces.

## World name and wave controls

The `world_name` launch argument defaults to `posim_surface`. Wave controls use `gz.msgs.Param` on `/world/<world_name>/waves`, so the default topic is `/world/posim_surface/waves`. To match existing application adapters, pass their world name explicitly. Inside the configured surface container:

```sh
SURFACE_WORLD=harbour
ros2 launch /opt/posim-surface/wamv.launch.py headless:=true world_name:="$SURFACE_WORLD"
```

The generated SDF, vehicle spawner, IMU bridge and startup wait all use this name. The ROS IMU endpoint remains `/model/wamv/imu`. This argument names the generated world; it does not select a terrain file.

The example uses a flat seabed at z = −10 m and loads no GEBCO bathymetry. Prepare geographic terrain with the [heightmap guide](../../website/content/en/heightmaps.md). Match visual and physics settings.

- `wind_speed`: m/s
- `wind_angle`: degrees
- `steepness`: dimensionless
- Periodic wave field: **256 m**
- Visual region: **768 m**, recentered by whole periods

The spectral wave model simulates surface waves. Tide, current and weather forecasts require separate data and models.

## Dependency configuration

`dependency.json` pins the upstream source revision. `extras/install-waves.sh` fetches that commit and installs libraries under `/opt/waves`. The standard ARM64 Dockerfile and Ubuntu installer call it; the AMD64 Dockerfile uses the Ubuntu installer.

The pinned source includes ocean material ownership and render-thread cleanup. The build recipe includes the source commit, so a different pin cannot reuse an older wave installation. Rebuild the standard base image before building the optional surface image when its dependency revision changes. For an existing source installation, preserve it and use a fresh dependency directory with matching source/resource paths.

`patch_external_waves.py` adapts CMake targets, library ordering and symbol visibility for Jetty's vendor packages. The upstream source remains under `/opt/asv_wave_sim`. Wave Sim is fetched during the build and requires no recursive Git clone.

## Operating limits

The optional Dockerfile supports the ARM64 base layout. Hull, propulsion and drag parameters need calibration for the vessel and operating conditions. MAVROS and Gazebo can require supervisor termination during shutdown; ensure both processes exit before starting another session.

## Licensing

POSIM-authored source uses Apache-2.0. Wave Sim uses GPL-3.0-or-later. Distributed images retain the patched upstream source and license notices under `/opt/asv_wave_sim`. CGAL and FFTW retain their own license obligations. WAM-V assets reference OSRF VRX; their source and notices are included in the external checkout.

Preserve corresponding source, build modifications and notices when distributing binaries. The distribution must comply with each component's terms, including requirements for combined programs. See the [upstream license](https://github.com/srmainwaring/asv_wave_sim/blob/ca8629df4e191235753dfae92ef725d30b923364/LICENSE), [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.en.html#MereAggregation) and [Apache compatibility guidance](https://www.apache.org/licenses/GPL-compatibility.html).
