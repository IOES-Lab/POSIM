# Ocean currents

POSIM separates world-current generation, vehicle-local current processing and ROS interaction. Uniform and depth-stratified flows use different topics and configuration blocks.

## Run the example

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=ocean_current_plugin z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

Inspect both transports from a second terminal:

```bash
gz topic -e -t /ocean_current
ros2 topic echo /hydrodynamics/currentVelocityTopic --once
```

Stop each viewer before running the next command.

## Systems and topics

| Component | Responsibility |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | Generate world flow and depth-layer data |
| Model current plugin | Interpolate/apply the configured current at a model |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | Publish ROS observations and offer configuration services |

| Example topic | Transport / type |
| --- | --- |
| `/ocean_current` | Gazebo `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo layer data |
| `/hydrodynamics/currentVelocityTopic` | ROS `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS `posim_interfaces/msg/StratifiedCurrentDatabase` |

Read the world's SDF for the configured namespaces and topic names. Keep vehicle hydrodynamics connected to the same intended flow source.

## Configure a velocity

List service names and types before calling them:

```bash
ros2 service list -t
ros2 service call /set_current_velocity posim_interfaces/srv/SetCurrentVelocity \
  '{velocity: 0.3, horizontal_angle: 0.0, vertical_angle: 0.0}'
```

Velocity is in m/s and the two angles are in radians. The example requests a horizontal flow along the zero-angle direction; check the returned success flag and subsequent current payloads.

Model services use `GetCurrentModel` and `SetCurrentModel` for velocity, horizontal angle and vertical angle. Parameters include mean, lower/upper bounds, noise amplitude and `mu`. Depth-specific services use the stratified interface types. Inspect them with `ros2 interface show` before setting a layer.

## Depth variation and tidal inputs

The world plugin loads a configured depth database and uses stochastic current models. The optional tidal configuration uses a database or supplied harmonic constituents with a configured start time and ebb/flood direction. These are experiment inputs, rather than live current observations.

Preserve the database units, interpolation range, world origin and time reference when changing a profile. Compare received current and vehicle response with the same initial pose before interpreting a navigation result.
