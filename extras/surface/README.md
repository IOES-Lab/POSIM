# Automatic external waves and optional WAM-V

Wave Sim is a pinned external build dependency. The standard Docker recipes and Ubuntu installer build it automatically. The upstream checkout supplies wave, hydrodynamics and WAM-V assets. `wamv.py` reads that model and configures its controller and propulsion.

- Dependency source: `/opt/asv_wave_sim`
- Installed libraries: `/opt/waves`
- Optional surface overlay: ArduRover and WAM-V
- World setup: include the wave and buoyancy systems in the selected SDF

## Build and run

This optional Dockerfile targets the ARM64 base layout (`docker` user, `/home/docker/ardupilot`, `/home/docker/posim_ws`). Build the ARM64 image as described in the [Docker guide](../../website/content/en/docker.md); Wave Sim is included by default, with no separate wave installation command. The optional surface overlay adds ArduRover and the WAM-V launch, reusing the wave installation when its recipe matches. From the repository root:

```sh
docker build -f extras/surface/Dockerfile --build-arg POSIM_BASE_IMAGE=posim:dev-arm64-rdp -t posim:surface .
docker run --rm -it --entrypoint bash posim:surface -lc 'source /opt/ros/lyrical/setup.bash; source /home/docker/posim_ws/install/setup.bash; ros2 launch /opt/posim-surface/wamv.launch.py headless:=true'
```

The image contains separate ArduSub and ArduRover binaries. This example runs one vehicle. Assign it one controller and port set.

- Rover boat frame: `FRAME_CLASS=2`
- Left/right propellers: servo outputs **1 / 3**
- Start position: water surface
- GUI: requires a display and renderer
- Navigation: GPS disabled; simulated external navigation enabled

The external-navigation adapter publishes **simulated Gazebo pose and speed** through MAVROS. Check navigation and arming status before sending commands. QGroundControl and MAVROS provide client interfaces.

Wave controls use `gz.msgs.Param` on `/world/wwos_gebco/waves`. Match visual and physics settings.

- `wind_speed`: m/s
- `wind_angle`: degrees
- `steepness`: dimensionless
- Periodic wave field: **256 m**
- Visual region: **768 m**, recentered by whole periods

The spectral wave model simulates surface waves. Tide, current and weather forecasts require separate data and models.

## Dependency configuration

`dependency.json` pins the upstream source revision. `extras/install-waves.sh` fetches that commit and installs libraries under `/opt/waves`. The standard ARM64 Dockerfile and Ubuntu installer call it; the AMD64 Dockerfile uses the Ubuntu installer.

`patch_external_waves.py` adapts CMake targets, library ordering and symbol visibility for Jetty's vendor packages. The upstream source remains under `/opt/asv_wave_sim`. Wave Sim is fetched during the build and requires no recursive Git clone.

## Operating limits

The optional Dockerfile supports the ARM64 base layout. Hull, propulsion and drag parameters need calibration for the vessel and operating conditions. MAVROS and Gazebo can require supervisor termination during shutdown; ensure both processes exit before starting another session.

## Licensing

POSIM-authored source uses Apache-2.0. Wave Sim uses GPL-3.0-or-later. Distributed images retain the patched upstream source and license notices under `/opt/asv_wave_sim`. CGAL and FFTW retain their own license obligations. WAM-V assets reference OSRF VRX; their source and notices are included in the external checkout.

Preserve corresponding source, build modifications and notices when distributing binaries. The distribution must comply with each component's terms, including requirements for combined programs. See the [upstream license](https://github.com/srmainwaring/asv_wave_sim/blob/ca8629df4e191235753dfae92ef725d30b923364/LICENSE), [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.en.html#MereAggregation) and [Apache compatibility guidance](https://www.apache.org/licenses/GPL-compatibility.html).
