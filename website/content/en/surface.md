# Surface robots and waves

Simulate a WAM-V surface vehicle with Wave Sim waves and ArduRover boat-mode control. The standard POSIM installer and Docker recipes build the external wave libraries.

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

For receiver-noise, water-contact and reacquisition experiments, POSIM also provides a [marine GNSS system](gnss.md). Attach its antenna and plugin explicitly and connect a suitable navigation adapter. The surface example's external-pose adapter is a separate navigation input.

## Adjust waves

Wave controls use `gz.msgs.Param` on `/world/<world_name>/waves`. Replace `<world_name>` with the SDF `<world name="...">` value; the topic identifies the running world, not its terrain provider. List the running simulation's topics with:

```bash
gz topic -l
```

The [WAM-V launcher](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py) defaults to `posim_surface`, so its wave topic is `/world/posim_surface/waves`. Set the `world_name` launch argument to match your application's existing Gazebo namespace. Inside the configured surface container, for example:

```bash
SURFACE_WORLD=harbour
ros2 launch /opt/posim-surface/wamv.launch.py headless:=true world_name:="$SURFACE_WORLD"
```

The generated SDF, vehicle spawner, IMU bridge and startup wait all use this name. The example above exposes `/world/harbour/waves`; the ROS IMU endpoint remains `/model/wamv/imu`. The argument sets the generated world's name; it does not select a terrain file.

This example uses a flat seabed at z = −10 m and loads no GEBCO bathymetry. Geographic terrain, including GEBCO data, is available through the separate public [POSIM-Terrain library](libraries.md#posim-terrain).

Each adjustable value uses the `double` data type:

| Parameter | Unit | Meaning |
| --- | --- | --- |
| `wind_speed` | m/s | Wave-model wind speed |
| `wind_angle` | degrees | Wind direction for the wave model |
| `steepness` | dimensionless | Wave steepness |

Keep visual and hydrodynamic settings consistent. A spectral wave model provides the surface motion; [ocean currents](currents.md) are configured separately. Installing wave libraries makes their systems available; a world must still include and configure those systems.

## Operating limits

The optional Dockerfile supports the ARM64 base layout. Hull, propulsion and drag parameters need calibration for the vessel and operating conditions. MAVROS and Gazebo can require supervisor termination during shutdown; ensure both processes exit before starting another session.

## Dependency maintenance

The revision is pinned in `extras/surface/dependency.json`. The installer fetches upstream source and builds libraries under `/opt/waves`, retaining source under `/opt/asv_wave_sim`. This is a build dependency, rather than a Git submodule.

See the [surface integration source guide](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) for build details and [licenses](citation.md) for component attribution.
