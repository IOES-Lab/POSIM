# Thrusters & autopilot

Choose the command interface that matches the robot. Read sensor data first, then enable one controller at a time.

| Vehicle | Command path | Next step |
|---|---|---|
| REXROV | ROS force commands → Gazebo thrusters | [Control with Python](python-control.md) |
| BlueROV2 | ROS/joystick → MAVROS → ArduSub | [ROVs & BlueROV2](rovs.md) |
| WAM-V | Surface integration and vehicle controller | [Surface robots & waves](surface.md) |

## REXROV thruster commands

The included REXROV configuration bridges eight `cmd_thrust` topics as `std_msgs/msg/Float64`. The values are thrust commands in newtons. A neutral command for the first thruster is:

```bash
ros2 topic pub --once /model/rexrov/joint/thruster1_joint/cmd_thrust \
  std_msgs/msg/Float64 '{data: 0.0}'
```

A controller must allocate forces across the thrusters using their positions and axes. Controlling the vehicle's speed requires coordinating the thrusters. End an experiment by sending neutral values to every commanded thruster.

## BlueROV2 and autopilot

BlueROV2 connects to ArduSub through MAVROS. After starting the [BlueROV2 example](rovs.md#start-bluerov2), wait for a connected state and inspect the command interfaces:

```bash
ros2 topic echo --no-daemon /mavros/state mavros_msgs/msg/State \
  --qos-reliability best_effort --qos-durability volatile --once --filter 'm.connected'
ros2 service list --no-daemon -t
ros2 topic info --no-daemon /mavros/manual_control/send --verbose
```

Volatile durability selects newly published state samples. The filter waits for `connected: true` after the autopilot handshake. If it keeps waiting, inspect the MAVROS and ArduSub launch logs and connection settings.

The manual-control adapter maps `/joy` and `/keyboard/joy` to MAVROS commands.

- Publication rate: **20 Hz**
- Input timeout: **0.3 seconds**

Use one command source and check arming/navigation status. See [ROVs](rovs.md) for keyboard and WebSocket setup.
