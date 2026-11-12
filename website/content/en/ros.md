# ROS terminals & topics

Keep the simulator running in one terminal and use another terminal for inspection or control. Both terminals must load the same ROS distribution and POSIM workspace.

## Follow the data path

<div class="docs-flow"><div><strong>ROS programs and commands</strong><span>Subscriptions, services and control inputs</span></div><div><strong>ROS interface</strong><span>ros_gz bridge or a plugin's ROS publisher</span></div><div><strong>Gazebo and POSIM plugins</strong><span>Physics, robot state and sensor measurements</span></div></div>

Sensor data travels toward your ROS program; commands travel toward the simulator. A Gazebo topic appears in ROS only when a bridge or ROS-enabled plugin publishes it. The robot launch loads the selected model's bridge configuration.

## Prepare Terminal A and Terminal B

Follow [Prepare your terminals](terminals.md) for your installation route. Keep [First simulation](quickstart.md) running in Terminal A and use Terminal B for the inspection commands below.

Use the BlueROV2 underwater example without sonar for these topic names. With sonar, replace `bluerov2` with `bluerov2_heavy_multibeam_sonar`; for the surface example, use `wamv`.

## Inspect the graph

The inspection commands use `--no-daemon` to query ROS directly, without depending on a cached CLI daemon.

```bash
ros2 topic list --no-daemon -t
ros2 service list --no-daemon -t
gz topic -l
ros2 topic info --no-daemon /model/bluerov2/odometry --verbose
```

`ros2 topic list` lists ROS topics; `gz topic -l` lists Gazebo Transport topics. Inspect the message type, publisher, subscriber and QoS information before connecting a program.

## Watch a changing value

```bash
ros2 topic echo --no-daemon /model/bluerov2/odometry nav_msgs/msg/Odometry --field pose.pose.position
```

Position samples should continue to arrive. A stationary robot can report nearly constant positions while its timestamps advance. Press **Ctrl+C in Terminal B** to stop the viewer; Terminal A keeps the simulation running.

Use `--once` when you want one message. Nodes that use simulation timestamps should set `use_sim_time:=true` and receive `/clock`.

## Connect another ROS computer

Use compatible ROS/message packages, matching domain settings and a reachable DDS discovery/data path. Install `posim_interfaces` when subscribing to POSIM-specific messages. If publishers exist but no samples arrive, check [Troubleshooting](troubleshooting.md) before changing the controller.

## Next steps

- [Control with Python](python-control.md): read odometry and send a bounded thruster command.
- [Thrusters & joystick control](robot-control.md): choose the interface for your vehicle.
- [View & record sensor data](visualization.md): display images and save observations.
