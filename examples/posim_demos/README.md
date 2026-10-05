# POSIM scene examples

Build POSIM using the [installation guide](../../website/content/en/install.md), then
source the ROS environment and your workspace in each new terminal:

```bash
source /opt/ros/lyrical/setup.bash
source ~/posim_ws/install/setup.bash
```

The package remains named `posim_demos` for compatibility. Its four entry points
compose installed world, robot, sensor, and object descriptions. Existing
resource hooks are installed by the packages; no manual CMake or hook edits are
needed for the examples below.

## World

```bash
ros2 launch posim_demos posim_world.launch.py world_name:=posim_ocean_waves
```

Append `headless:=true` for server-only execution. World names correspond to
files in `models/posim_worlds/worlds`, without the `.world` suffix.

## Object

```bash
ros2 launch posim_demos posim_object.launch.py \
  namespace:=mossy_cinder_block paused:=false
```

The object descriptor resolves its model resources, including any Fuel URIs.
Remote assets may be downloaded on first use and cached by Gazebo.

## Sensor

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world paused:=false z:=-30
```

Use `gui:=false headless:=true` for a server-only run. Rendering sensors may
still need a working render context when the graphical client is disabled.
The [CUDA sonar](../../gazebo/posim_gz_multibeam_sonar/README.md) has additional
hardware and toolkit requirements.

## Robot

REXROV:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false
```

BlueROV2, with the ArduSub/MAVROS dependencies from the installation guide:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=bluerov2 world_name:=posim_ocean_waves z:=-0.5 paused:=false
```

Use `namespace:=bluerov2_heavy` for the heavy configuration. The
`bluerov2_heavy_multibeam_sonar` configuration also needs the CUDA sonar backend.

Slocum glider:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=glider_slocum world_name:=posim_ocean_waves x:=4 z:=-1.5 paused:=false
```

For non-interactive robot runs, append:

```text
gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

BlueROV runs can additionally use `open_qgc:=false open_virtual_joystick:=false`
to prevent those applications from opening. To inspect the available arguments:

```bash
ros2 launch posim_demos posim_robot.launch.py --show-args
```
