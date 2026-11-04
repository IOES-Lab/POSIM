# Add a robot

Create a model using a POSIM robot with a similar layout as a reference. SDF describes the mechanical model and systems; the ROS configuration connects observations and commands.

## 1. Choose a starting point

Use [REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) for a Gazebo-thruster vehicle or [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) for ArduSub/MAVROS integration. The [Slocum guide](gliders.md) covers another vehicle layout.

From `~/posim_ws`, make a complete working copy of REXROV. This keeps its meshes, bridge configuration and topic names together. The script stops if `my_robot` already exists.

```python
from pathlib import Path
import shutil

root = Path('src/posim/models/posim_robot_models')
directories = ('description', 'config', 'meshes')
if any((root / directory / 'my_robot').exists() for directory in directories):
    raise SystemExit('my_robot already exists; choose another model name.')
for directory in directories:
    shutil.copytree(root / directory / 'rexrov', root / directory / 'my_robot')
for relative in ('description/my_robot/model.sdf',
                 'description/my_robot/model.config',
                 'config/my_robot/robot_config.py'):
    target = root / relative
    target.write_text(target.read_text().replace('rexrov', 'my_robot'))
```

Save the script as `copy_robot.py` and run `python3 copy_robot.py`. The resulting layout includes:

```text
models/posim_robot_models/
  description/my_robot/model.sdf
  description/my_robot/model.config
  meshes/my_robot/RexROV_no_props.dae
  config/my_robot/robot_config.py
```

`my_robot` is your chosen identifier. The launch looks up `description/<namespace>/model.sdf` and the corresponding configuration.

## 2. Define mechanics and assets

Keep the copied mechanics for the first launch. When changing the hull, review these settings together:

| SDF path | Unit | What to check |
|---|---|---|
| `model/@name`, link and joint names | identifier | Match entity names used by plugins and ROS configuration |
| `link/inertial/mass` | kg | Positive mass for each moving link |
| `link/inertial/pose` | m, rad | Centre of mass and inertia-frame orientation |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | Moments about the inertia-frame axes |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | Off-diagonal terms; the full tensor must be physically valid |
| `link/visual`, `link/collision` | m, rad | Mesh scale and aligned geometry |
| `joint/axis` and thruster settings | plugin-specific | Actuator direction and force allocation |

STL supplies geometry. Define mass and dynamics in SDF, and confirm mesh units and scale.

Use installed resource URIs, following the package's existing mesh examples. Keep visual and collision poses aligned. Prefer a simple collision shape when a detailed visual mesh is unnecessarily expensive for contact.

## 3. Connect systems and ROS

Adapt hydrodynamics, buoyancy, thruster axes and sensor poses to your robot. Choose coefficients that match your robot's physical properties; values from another vehicle may give misleading motion.

Use [Ocean currents](currents.md) to connect uniform or depth-dependent flow. Follow [Add & configure sensors](custom-sensors.md) for model placement, world systems and ROS routes, then use the linked plugin's parameter reference. Set the world's geographic origin using [Spherical coordinates](coordinates.md) when your experiment needs latitude and longitude.

Copy a relevant `config/<namespace>/robot_config.py`, updating entity names, joint names, topics, frames and any controller ports together. The package already installs `description`, `meshes`, `config` and resource hooks. Do not edit generated `install/` files.

## 4. Build and launch

From a terminal with the workspace environment loaded:

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 launch posim_demos posim_robot.launch.py --show-args
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=my_robot world_name:=posim_ocean_waves z:=-2 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

Check each axis with small control commands. Launch from outside the source checkout too.

- Installed asset paths
- Mass, inertia and equilibrium pose
- Collision geometry and response
- Coordinate frames and message contents

After startup, confirm the copied model's topic in a second sourced terminal:

```bash
ros2 topic echo --no-daemon /model/my_robot/odometry nav_msgs/msg/Odometry --once
```

You should receive the renamed model's position and velocity. Keep one change per experiment: first make the copy work, then edit mechanics, then add sensors or propulsion changes. Use [View & record sensor data](visualization.md) to compare runs and [Troubleshooting](troubleshooting.md) for missing assets.
