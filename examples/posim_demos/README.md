# POSIM scene examples

Build POSIM using the [installation guide](../../website/content/en/install.md), then
source the ROS environment and your workspace in each new terminal:

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
```

The `posim_demos` package provides two introductory tutorials and four generic
launch entry points for installed world, object, sensor and robot descriptions.
Packages install their resource hooks automatically.

## First simulations

Start BlueROV2 above a four-metre seabed with inspection objects and a shaded
underwater camera:

```bash
ros2 launch posim_demos underwater_tutorial.launch.py
```

Append `sonar:=true` to add multibeam sonar. The backend remains `auto`; sonar
may reduce frame rate and control responsiveness. Gazebo and RViz load the
camera, vehicle and optional sonar views.

After stopping that session, start WAM-V with a camera and lidar facing buoys:

```bash
ros2 launch posim_demos surface_tutorial.launch.py
```

Both tutorials use the standard installation. Follow
[First simulation](../../website/content/en/quickstart.md) for live-data checks,
QGroundControl video and controls, and shutdown.

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
The [multibeam sonar](../../gazebo/posim_gz_multibeam_sonar/README.md) has
backend-specific hardware and toolkit requirements.

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
`bluerov2_heavy_multibeam_sonar` configuration needs a supported CPU, WGPU or
CUDA sonar backend and a working renderer.

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
