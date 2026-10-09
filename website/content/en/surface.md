# Surface robots and waves

The surface integration combines the upstream Wave Sim dependency, a WAM-V model and ArduRover boat-mode control. The standard POSIM installer and Docker recipes already build the external wave libraries.

## Build the WAM-V integration

The optional surface image uses the ARM64 base layout. From a POSIM checkout, first build `posim:dev-arm64-rdp` using [Docker](docker.md), then run:

```bash
docker build -f extras/surface/Dockerfile \
  --build-arg POSIM_BASE_IMAGE=posim:dev-arm64-rdp -t posim:surface .
docker run --rm -it --entrypoint bash posim:surface -lc \
  'source /opt/ros/lyrical/setup.bash; source /home/docker/posim_ws/install/setup.bash; ros2 launch /opt/posim-surface/wamv.launch.py headless:=true'
```

This image contains ArduSub and ArduRover as separate binaries. The single-vehicle example selects ArduRover's boat configuration (`FRAME_CLASS=2`). Separate vehicles need separate controller identities and network ports.

## Model, controller and navigation

`extras/surface/wamv.py` reads the model from the external dependency and configures the integration. The two aft propellers are driven through outputs 1 and 3. The example starts at the sea surface.

The external-navigation adapter publishes **simulated Gazebo pose and velocity** to MAVROS. Check controller state, navigation validity and thruster direction before sending a mission.

## Adjust waves

The surface world's wave-control topic is `/world/wwos_gebco/waves`, using `gz.msgs.Param`. The adjustable double-valued parameters are:

| Parameter | Unit | Meaning |
| --- | --- | --- |
| `wind_speed` | m/s | Wave-model wind speed |
| `wind_angle` | degrees | Wind direction for the wave model |
| `steepness` | dimensionless | Wave steepness |

Keep visual and hydrodynamic settings consistent. A spectral wave model provides the surface motion; [ocean currents](currents.md) are configured separately. Installing wave libraries makes their systems available; a world must still include and configure those systems.

## Dependency maintenance

The revision is pinned in `extras/surface/dependency.json`. The installer fetches upstream source and builds libraries under `/opt/waves`, retaining source under `/opt/asv_wave_sim`. This is a build dependency, rather than a Git submodule.

See the [surface integration source guide](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) for build details and [licenses](citation.md) for component attribution.
