# First simulation

Start one REXROV in an ocean world, receive live measurements and stop cleanly. Keep the default robot model and world for this first run.

## Before you start

Complete the installation route in [Choose an environment](requirements.md). Then use [Prepare your terminals](terminals.md) to open two terminal windows and load ROS/POSIM in each. Terminal A runs the simulator; Terminal B reads its output. Docker users see the Gazebo window through their configured Linux desktop, WSLg or RDP connection.

## 1. Start the robot and world

In **Terminal A**, with the workspace loaded:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=true headless:=false use_teleop:=false use_web_joystick:=false
```

This command starts the world, places REXROV five metres below z = 0 and starts its ROS bridges. Keep Terminal A running. Starting the robot also starts its world, so run this as one simulation.

## 2. Read live position

In **Terminal B**, load the same workspace and run:

```bash
ros2 topic echo --no-daemon /model/rexrov/odometry nav_msgs/msg/Odometry --field pose.pose.position
```

Position samples should continue to arrive. Small or nearly constant values can be normal while the vehicle settles. Press **Ctrl+C in Terminal B** to finish watching.

## 3. Check simulation time

```bash
ros2 topic echo --no-daemon /clock rosgraph_msgs/msg/Clock
```

The clock should advance. Stop this viewer with Ctrl+C. If time is paused or no messages arrive, use [Troubleshooting](troubleshooting.md).

## 4. Stop the session

Press **Ctrl+C in Terminal A** and wait for the launch processes to exit. Closing a topic viewer does not stop the simulator. Stop the current simulation before starting a different example.

## Continue learning

- [View & record sensor data](visualization.md): run the underwater camera and inspect images.
- [Control with Python](python-control.md): observe REXROV and exercise one thruster.
- [ROVs & BlueROV2](rovs.md): use ArduSub, keyboard controls or the local joystick.
- [World library](worlds.md): choose another environment and inspect launch arguments.
