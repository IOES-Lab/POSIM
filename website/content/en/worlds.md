# World library

Worlds live in `models/posim_worlds/worlds`. Choose a world by its filename without the `.world` extension.

## Start a world

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=true
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

The list below is generated from the world files in this checkout when the website is built. Open a file to see its systems, included models, origin, lights and camera configuration.

| World SDF |
| --- |
{{WORLD_CATALOG}}

## Compose another environment

Copy a suitable SDF into `models/posim_worlds/worlds` with a new filename and world name. Use installed resource URIs for local models. Rebuild and source the workspace, then launch the new filename. See [Heightmap terrain](heightmaps.md) for terrain geometry and [Objects](objects.md) for task assets.
