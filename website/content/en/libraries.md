# Components & libraries

POSIM provides ocean environments, robot models, sensors and ROS 2 interfaces as an open-source maritime robotics simulation library. These related libraries add geographic terrain, route planning and vehicle control to your experiments.

## Inside the POSIM workspace

| Location | What it supplies |
|---|---|
| `models/posim_worlds` | World files and their environment resources |
| `models/posim_robot_models` | Robot mechanics, sensors and per-robot ROS configuration |
| `models/posim_sensor_models` | Standalone sensor descriptions and examples |
| `gazebo/` | Marine environment and sensor plugins |
| `posim_interfaces` | POSIM message and service definitions |
| `examples/posim_demos` | Launch files that connect worlds, models and ROS |

Use [First simulation](quickstart.md) to run these components together. Use [Add a robot](custom-robots.md) and [Create a world](custom-worlds.md) when you want to change the model or environment.

## Related libraries

| Repository | Responsibility | Inputs and outputs |
| --- | --- | --- |
| [POSIM-Terrain](https://github.com/IOES-Lab/POSIM-Terrain) | Land elevation, bathymetry and satellite textures | Geographic region → numeric terrain, meshes, SDF and source manifests |
| [POSIM-Routing](https://github.com/IOES-Lab/POSIM-Routing) | Global routes, coastal paths, port approaches and local collision checks | Departure, destination and terrain → routes and waypoints |
| [POSIM-Control](https://github.com/IOES-Lab/POSIM-Control) | Propulsion, leader/follower control and motion policies | Checked waypoints and measured state → propulsion commands and controller status |

## POSIM-Terrain

Generate a land/ocean region from latitude, longitude and dimensions. Available elevation and bathymetry sources form one numeric field for meshes and collision checks. Prepare Sentinel-2 imagery to add land textures. Exported manifests record coverage, hashes and source assumptions.

GEBCO bathymetry is supported by this public library, including the `gebco` source policy for GEBCO-only terrain generation. POSIM can load exported terrain in a world; its core engine and the [flat-seabed WAM-V example](surface.md) do not require GEBCO data.

- [Installation and generation commands](https://github.com/IOES-Lab/POSIM-Terrain#readme)
- [New York, Busan and Tokyo bundles and 3D captures](https://github.com/IOES-Lab/POSIM-Terrain/tree/main/examples)

## POSIM-Routing

Plan an itinerary between ports, then connect it to each port's arrival area. OSM land polygons guide port connections. Coastal and local planners use POSIM-Terrain's numeric field to check paths. A global graph route alone does not establish water depth.

- [Installation, route export and planner APIs](https://github.com/IOES-Lab/POSIM-Routing#readme)
- Terrain dependency: POSIM-Terrain

## POSIM-Control

Use Python policies for speed, rough water, stability and recovery. Native Gazebo plugins apply propulsion to a surface leader and follower. The leader follows supplied waypoints; the follower follows measured leader poses. The caller checks terrain before sending a path.

- [Python policies, native plugin builds and control topics](https://github.com/IOES-Lab/POSIM-Control#readme)
- Native build: configured POSIM environment with Gazebo development packages

## Use them together

POSIM-Terrain supplies the geographic field. POSIM-Routing plans and checks paths across it. POSIM-Control follows approved waypoints while Gazebo computes physical motion.

Follow each library's README to install it and connect its inputs and outputs. [WWW-POSIM](https://www-posim.vercel.app/) is one application that combines these libraries with POSIM in a web workspace and desktop app.

For POSIM experiments, start with [installation](install.md), [robot models](custom-robots.md) and [ROS 2 control](ros.md).
