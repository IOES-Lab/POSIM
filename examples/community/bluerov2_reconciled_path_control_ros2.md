# External BlueROV2 Direct-Thruster Path Controller

This guide describes a community-maintained ROS 2 BlueROV2 path-following controller package
for POSIM / Gazebo:

[BlueROV2 reconciled path control](https://github.com/drwa92/bluerov2_reconciled_path_control_ros2)

> [!NOTE]
> This package is maintained outside POSIM. Follow its installation instructions
> and ensure `bluerov2_path_control` is available in your sourced ROS 2 workspace.

The package provides:

- go-to pose control;
- waypoint following;
- circle and spiral path following;
- generic trajectory following;
- stop and emergency-stop services;
- direct six-thruster BlueROV2 allocation;
- optional model-aided virtual-wrench reconciliation diagnostics.

Run the controller with POSIM's BlueROV2 simulator in direct-control mode:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  z:=-0.5 \
  namespace:=bluerov2 \
  world_name:=posim_ocean_waves \
  paused:=false \
  use_ardusub:=false \
  use_teleop:=false

ros2 launch bluerov2_path_control path_controller.launch.py \
  model_name:=bluerov2 \
  use_bridge:=true
```

Check Odometry input and the six thruster command topics before sending a `GoTo` request.

The package is maintained externally by Waseem Akram at MARVIS LAB:

[MARVIS LAB](https://drwa92.github.io/marvis-lab/)

Project webpage:

[BlueROV2 controller project page](https://drwa92.github.io/bluerov2_reconciled_path_control_ros2/)
