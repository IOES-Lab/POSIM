# Automatic external waves and optional WAM-V

Wave Sim is a pinned external build dependency. The standard Docker recipes and Ubuntu installer build it automatically. The upstream checkout supplies wave, hydrodynamics and WAM-V assets. `wamv.py` reads that model and configures its controller and propulsion.

- Dependency source: `/opt/asv_wave_sim`
- Installed libraries: `/opt/waves`
- Optional surface overlay: ArduRover and WAM-V
- World setup: include the wave and buoyancy systems in the selected SDF

## Build and run

This optional Dockerfile currently targets the ARM64 base layout (`docker` user, `/home/docker/ardupilot`, `/home/docker/posim_ws`). Build the ARM64 image as described in the [Docker guide](../../website/content/en/docker.md); Wave Sim is included by default, with no separate wave installation command. The optional surface overlay adds ArduRover and the WAM-V launch, reusing the wave installation when its recipe matches. From the repository root:

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

## Dependency maintenance and distribution

`dependency.json` pins the reviewed source revision. `extras/install-waves.sh` fetches that commit, compiles it and installs libraries under `/opt/waves`. It is called from the standard ARM64 Dockerfile and the Ubuntu installer (also used by the AMD64 Dockerfile). This is a source build dependency, not a Git submodule; users do not need `--recurse-submodules` for Wave Sim. `patch_external_waves.py` adapts CMake/package targets, library ordering and symbol visibility for Jetty's unversioned vendor packages, without copying the solver. Retain the patched upstream source and all GPL notices alongside distributed binaries; the Docker build does so under `/opt/asv_wave_sim`. CGAL/FFTW also have license obligations. Externalizing code solves the in-repository fork/maintenance problem; **it does not automatically remove GPL obligations for combined or distributed software**. Review the exact distribution before release. Fetching a dependency here is not a claim that all POSIM files must change license, or that any distribution is exempt.

Upstream WAM-V assets reference OSRF VRX. Their source and notices are included in the external checkout.

## Submodules and licensing

A Git submodule records a separate repository URL and commit; it does not compile or install it. A submodule workflow would need `git submodule update --init --recursive` plus explicit CMake build/install steps. Fetching the pinned upstream revision during the image build gives the same automatic installation without requiring a recursive user clone. See [Git documentation](https://git-scm.com/docs/git-submodule).

Using a submodule is not inherently a GPL violation and is not an exemption either. Preserve the upstream license/notices, corresponding source and build modifications. GPL treatment depends on whether distributed components are separate works or one combined program, especially when plugins share an address space; repository boundaries do not decide that. POSIM-authored source retains Apache-2.0 notices. Apache-2.0 is compatible with GPLv3, but that does not permit distributing GPL wave code under Apache-2.0 alone. See [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.en.html#MereAggregation), [upstream license](https://github.com/srmainwaring/asv_wave_sim/blob/ca8629df4e191235753dfae92ef725d30b923364/LICENSE), and [Apache compatibility guidance](https://www.apache.org/licenses/GPL-compatibility.html). Docker image labels declare both license families; this is a component inventory, not a complete legal determination of a distribution.
