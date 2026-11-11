# Features & examples

Use this catalogue to find each public simulation capability, its implementation and its usage guide. POSIM provides Gazebo systems, robot models and ROS interfaces. World SDF and launch configuration select the systems used by a simulation.

## Environment and rendering

| Capability | Public implementation | Usage |
| --- | --- | --- |
| Underwater haze, RGB absorption and sunlight fading | `gazebo/posim_gz_rendering` | [Scene effects, defaults and live controls](underwater-rendering.md) |
| Seabed caustics, suspended specks / marine snow | `gazebo/posim_gz_rendering` | [Enable and inspect underwater effects](underwater-rendering.md) |
| Seabed texture, measurement grid and depth contours | `gazebo/posim_gz_rendering`; maps supplied by the terrain | [Spacing and material configuration](underwater-rendering.md) |
| Spectral waves and WAM-V wave-aware hydrodynamics | External Wave Sim; `extras/surface` integration | [Surface robots and waves](surface.md) |
| Uniform, depth-dependent and tidal currents | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [Current inputs and ROS services](currents.md) |
| Geographic origin and coordinate conversion | `posim_ros_gz_plugins` | [Spherical-coordinate services](coordinates.md) |
| Seabed, objects and task worlds | `models/posim_worlds`, `posim_object_models` | [Worlds](worlds.md), [objects](objects.md), [heightmaps](heightmaps.md) |

Underwater optics shade PBR surfaces and preserve numeric depth. The ocean's custom surface material has its own rendering. Marine snow is a procedural visual approximation and is disabled by default. See the [underwater guide](underwater-rendering.md) for the sea-level assumption and camera behavior.

## Sensors, robots and control

| Capability | Public implementation | Usage |
| --- | --- | --- |
| Marine GNSS, immersion loss, reacquisition and latency | `gazebo/posim_gz_gnss` | [Receiver setup and measurement schema](gnss.md) |
| RGBD-based underwater image attenuation | `posim_gz_sensor_plugins/UnderwaterCamera` | [Camera example and parameters](camera.md) |
| DVL measurements and ROS conversion | Gazebo DVL and `posim_ros_gz_plugins` | [Beam settings and output](dvl.md) |
| Hydrostatic pressure and derived depth | `posim_gz_sensor_plugins` | [Pressure configuration and units](pressure.md) |
| USBL acoustic interrogation and positioning | `posim_gz_sensor_plugins` | [Transceiver/transponder example](usbl.md) |
| Multibeam sonar | `gazebo/posim_gz_multibeam_sonar` | [Sonar model](sonar.md), [build and tuning](sonar-tuning.md) |
| REXROV, BlueROV2 variants and Slocum glider | `models/posim_robot_models` | [ROVs](rovs.md), [glider](gliders.md), [custom robots](custom-robots.md) |
| ArduRover WAM-V integration and ROS control | `extras/surface`, model launch configurations, `posim_interfaces` | [Surface vehicle](surface.md), [ROS 2](ros.md) |

Sensor availability depends on installed plugins and model configuration. GNSS profiles describe mounts; attach the link and receiver when integrating one. Sonar can use CPU, WGPU or CUDA; choose the backend for your environment. Use each sensor reference's quickstart to confirm live measurements.

## Source directories

The paths below are relative to the POSIM source repository.

| Location | What it supplies |
|---|---|
| `models/posim_worlds` | World files and their environment resources |
| `models/posim_robot_models` | Robot mechanics, sensors and per-robot ROS configuration |
| `models/posim_sensor_models` | Standalone sensor descriptions and examples |
| `gazebo/` | Marine environment and sensor plugins |
| `posim_interfaces` | POSIM message and service definitions |
| `examples/posim_demos` | Launch files that connect worlds, models and ROS |

Use [First simulation](quickstart.md) to run these components together. Use [Add a robot](custom-robots.md) and [Create a world](custom-worlds.md) when you want to change the model or environment.

## macOS Ogre lifetime overlay

The [Ogre overlay helper](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) builds an isolated Ogre core for the documented macOS arm64 / Ogre-Next 2.3.3 configuration. Follow its supported SDK profile and loaded-library checks when using that configuration.

Third-party license terms remain with their [components](citation.md).
