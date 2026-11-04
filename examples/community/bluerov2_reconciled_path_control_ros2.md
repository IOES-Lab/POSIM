# External BlueROV2 Direct-Thruster Path Controller

This guide describes a community-maintained ROS 2 BlueROV2 path-following controller package
for POSIM / Gazebo:

[BlueROV2 reconciled path control](https://github.com/drwa92/bluerov2_reconciled_path_control_ros2)

Build and source the external package using its
[installation instructions](https://github.com/drwa92/bluerov2_reconciled_path_control_ros2#5-installation).
Use the same ROS distribution as your POSIM workspace. Source POSIM and the
controller workspace in both terminals below.

The package provides:

- go-to pose control;
- waypoint following;
- circle and spiral path following;
- generic trajectory following;
- stop and emergency-stop services;
- direct six-thruster BlueROV2 allocation;
- optional model-aided virtual-wrench reconciliation diagnostics.

In Terminal A, start POSIM's BlueROV2 simulator in direct-control mode:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  z:=-0.5 \
  namespace:=bluerov2 \
  world_name:=posim_ocean_waves \
  paused:=false \
  use_ardusub:=false \
  use_teleop:=false
```

Leave the simulator running. In Terminal B, start the controller:

```bash
ros2 launch bluerov2_path_control path_controller.launch.py \
  namespace:=path_controller \
  model_name:=bluerov2 \
  use_bridge:=true
```

The controller namespace puts its mission services under `/path_controller`,
matching the external project's command examples. Use those examples to choose a target or trajectory.
Check odometry reception and issue its stop command before ending a run.

The package is maintained externally by Waseem Akram.

Project webpage:

[BlueROV2 controller project page](https://drwa92.github.io/bluerov2_reconciled_path_control_ros2/)
