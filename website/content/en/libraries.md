# Terrain, routing and control libraries

POSIM is the simulation engine for ocean environments, robot models, sensors and ROS 2 interfaces. [WWW-POSIM](https://www-posim.vercel.app/) combines it with three public libraries for geographic worlds and automated voyages.

| Repository | Responsibility | Inputs and outputs |
| --- | --- | --- |
| [POSIM-Terrain](https://github.com/IOES-Lab/POSIM-Terrain) | Land elevation, bathymetry and satellite textures | Geographic region → numeric terrain, meshes, SDF and source manifests |
| [POSIM-Routing](https://github.com/IOES-Lab/POSIM-Routing) | Global routes, coastal paths, port approaches and local collision checks | Departure, destination and terrain → routes and waypoints |
| [POSIM-Control](https://github.com/IOES-Lab/POSIM-Control) | Propulsion, leader/follower control and motion policies | Checked waypoints and measured state → propulsion commands and controller status |

## POSIM-Terrain

Generate a land/ocean region from latitude, longitude and dimensions. Available elevation and bathymetry sources form one numeric field for meshes and collision checks. Prepare Sentinel-2 imagery to add land textures. Exported manifests record coverage, hashes and source assumptions.

- [Installation and generation commands](https://github.com/IOES-Lab/POSIM-Terrain#readme)
- [New York, Busan and Tokyo bundles and 3D captures](https://github.com/IOES-Lab/POSIM-Terrain/tree/main/examples)

## POSIM-Routing

Plan a global maritime itinerary, then connect it to arrival water. OSM land polygons guide port connections. Coastal and local planners use POSIM-Terrain's numeric field to check paths. A global graph route alone does not establish water depth.

- [Installation, route export and planner APIs](https://github.com/IOES-Lab/POSIM-Routing#readme)
- Terrain dependency: POSIM-Terrain

## POSIM-Control

Use Python policies for speed, rough water, stability and recovery. Native Gazebo plugins apply propulsion to a surface leader and follower. The leader follows supplied waypoints; the follower follows measured leader poses. The caller checks terrain before sending a path.

- [Python policies, native plugin builds and control topics](https://github.com/IOES-Lab/POSIM-Control#readme)
- Native build: configured POSIM environment with Gazebo development packages

## Use them together

POSIM-Terrain supplies the geographic field. POSIM-Routing plans and checks paths across it. POSIM-Control follows approved waypoints while Gazebo computes physical motion. WWW-POSIM manages sessions, terrain jobs, voyage progress and user interfaces.

Each library can be used directly through its README. WWW-POSIM pins all three and the POSIM engine as Git submodules. Its web workspace and installed app include the pinned implementations. See [WWW-POSIM components](https://www-posim.vercel.app/guide/components.html) for the application structure.

For engine use, start with [installation](install.md), [robot models](custom-robots.md) and [ROS 2 control](ros.md).
