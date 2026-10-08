# Add a robot

Create a model using a POSIM robot with a similar layout as a reference. SDF describes the mechanical model and systems; the ROS configuration connects observations and commands.

## 1. Choose a starting point

Use [REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) for a Gazebo-thruster vehicle or [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) for ArduSub/MAVROS integration. The [Slocum guide](gliders.md) covers another vehicle layout.

Copy the description into a new directory, for example:

```text
models/posim_robot_models/
  description/my_robot/model.sdf
  description/my_robot/model.config
  meshes/my_robot/hull.stl
  config/my_robot/robot_config.py
```

`my_robot` is your chosen identifier. The launch looks up `description/<namespace>/model.sdf` and the corresponding configuration.

## 2. Define mechanics and assets

Configure physical properties in the model SDF.

- Model/link names
- Mass: kg
- Inertia: kg·m²
- Position/rotation: m/rad
- Collision geometry

STL supplies geometry. Define mass and dynamics in SDF, and confirm mesh units and scale.

Use installed resource URIs, following the package's existing mesh examples. Keep visual and collision poses aligned. Prefer a simple collision shape when a detailed visual mesh is unnecessarily expensive for contact.

## 3. Connect systems and ROS

Adapt hydrodynamics, buoyancy, thruster axes and sensor poses to your robot. Reusing coefficients from another vehicle changes the simulated dynamics; choose them from your model's properties.

Copy a relevant `config/<namespace>/robot_config.py`, updating entity names, joint names, topics, frames and any controller ports together. The package already installs `description`, `meshes`, `config` and resource hooks. Do not edit generated `install/` files.

## 4. Build and launch

From the sourced workspace:

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 launch posim_demos posim_robot.launch.py --show-args
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=my_robot world_name:=posim_ocean_waves z:=-2 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

Check each axis with small control commands. Launch from outside the source checkout too.

- Installed asset resolution
- Mass, inertia and equilibrium pose
- Collision geometry and response
- Frame conventions and message payloads

Save the model revision and observations for reproducible comparisons. Use [ROS 2 and control](ros.md) for recording and [Troubleshooting](troubleshooting.md) for missing assets.
