# World library

Worlds live in `models/posim_worlds/worlds`. Choose a world by its filename without the `.world` extension.

Ocean demo worlds include `model://calm_waves`: Wave Sim FFT waves with a 3 m/s wind input, a 256 m periodic tile and a 768 × 768 m visual patch. The surface is visible from above and below. Native underwater effects shade RGB sensor images; the shallow underwater tutorial also enables them in the Gazebo world view. Wave Sim is included by the standard installers and images. Underwater vehicle buoyancy, marine GNSS and the optical boundary use mean water level z = 0, independently of visual wave elevation. WAM-V uses Wave Sim hydrodynamics coupled to the waves. The dry USBL and isolated sonar test worlds keep their own geometry.

## Start a world

Run commands in the [POSIM terminal prepared for your installation](terminals.md), whether you use native Ubuntu, WSL, Docker or the RDP desktop.

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

The filename and internal world name can differ. `posim_ocean_waves.world` declares `oceans_waves`, which appears in Gazebo topic and service paths. Read the `<world name="…">` element before constructing a path.

## Choose by experiment

| Experiment | World |
| --- | --- |
| Initial vehicle setup | `posim_ocean_waves` |
| Underwater image effects | `camera_tutorial` |
| DVL observations | `dvl_world` |
| USBL interrogation | `usbl_tutorial` |
| Current configuration | `ocean_current_plugin` |
| Coastal/geographic scene | `posim_Santorini` |
| Sonar scene | `posim_multibeam_sonar` and sonar-named ocean worlds |
| Manipulation/task objects | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

Sonar-specific worlds require the [sonar backend](sonar-tuning.md). First-use Fuel references may download models before a scene is ready.

## Source catalog

Open a world file to see its systems, included models, origin, lights and camera configuration.

| World SDF |
| --- |
{{WORLD_CATALOG}}

## Compose another environment

Copy a suitable SDF into `models/posim_worlds/worlds` with a new filename and world name. Use installed resource URIs for local models. Rebuild and source the workspace, then launch the new filename. See [Heightmap terrain](heightmaps.md) for terrain geometry and [Objects](objects.md) for task assets.

## Check first-use Fuel downloads

`posim_integrated` and `posim_ocean_models` include remote Fuel models. A clock
topic alone does not prove that these scenes have finished loading. Allow the
first-use downloads to finish; do not start repeated launches while they are
still downloading. Use `verbose:=true` to inspect resource loading.

In another terminal with the same environment, request the loaded scene:

```bash
# posim_integrated.world uses the internal name posim_integrated.
gz service -s /world/posim_integrated/scene/info \
  --reqtype gz.msgs.Empty --reptype gz.msgs.Scene --timeout 5000 --req ''
# For posim_ocean_models.world, use /world/posim_ocean_model/scene/info.
```

Confirm that the response includes the expected models, then check the simulation clock and sensor outputs. If a download stalls, inspect the resource-loading message and network access from the machine or container running Gazebo.

## Choose launch arguments

Use `--show-args` to inspect arguments accepted by a robot or sensor launch.

```bash
ros2 launch posim_demos posim_sensor.launch.py --show-args
```

| Argument | Meaning | Sensor launch default |
| --- | --- | --- |
| `namespace` | Select an installed model directory | Empty; set explicitly |
| `world_name` | Select a world file | `empty.sdf` |
| `paused` | Start paused | `true` |
| `gui` / `headless` | Show / hide Gazebo GUI | `true` / `false` |
| `use_sim_time` | Use the simulation clock in ROS nodes | `true` |
| `x`, `y`, `z` | Initial position, m | `0.0` each |
| `roll`, `pitch`, `yaw` | Initial attitude, rad | `0.0` each |
| `debug` / `verbosity_level` | Verbose logging / log level | `false` / `1` |
| `use_ned_frame` | Publish the `world_ned` transform (ENU to NED) | `false` |

Use `paused:=false` in sensor examples to produce data. Inspect robot/world launch arguments with that launch's `--show-args`.
