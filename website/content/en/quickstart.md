# First simulation

Start one world, then add a robot or a sensor. Use two terminals with the same [Ubuntu](install.md) or [Docker](docker.md) environment loaded: Terminal A runs the simulation; Terminal B inspects data.

## 1. Start an ocean world

In Terminal A:

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=true
```

In Terminal B:

```bash
gz topic -e -t /world/oceans_waves/clock
```

Simulation time should advance. The world entity is `oceans_waves`, although the file is `posim_ocean_waves.world`. Stop the topic viewer and then the launch with Ctrl+C before starting the next example.

## 2. Spawn REXROV

The robot launch starts its world too; do not leave the previous world running.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

In Terminal B:

```bash
ros2 topic echo /model/rexrov/odometry nav_msgs/msg/Odometry --once
```

Inspect the timestamp, pose and twist. An unmoving robot can still produce valid odometry. See [ROVs](rovs.md) to select BlueROV2 and its control interfaces.

## 3. Receive a camera image

Stop REXROV. Before starting the camera, confirm that a sensor renderer is available. In a container without a display, follow [Docker rendering setup](docker.md) first: `headless:=true` only hides the GUI in the current launcher. Then run:

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

In Terminal B:

```bash
ros2 topic echo /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
```

Check nonzero width and height, encoding and timestamp. `--no-arr` hides the pixel array from terminal output. To see the image, use an RViz Image display subscribed to this topic. [Underwater camera](camera.md) explains the attenuation settings.

## Desktop viewing

Open Gazebo windows on a host with a display and renderer.

- Robot/sensor launch: `gui:=true headless:=false`
- World launch: `headless:=false`

## Check the results

Check these outputs from the examples.

- Advancing simulation time
- Odometry position and velocity
- Image size, format and timestamp

See [ROVs](rovs.md) for control and [Multibeam sonar](sonar.md) for sonar.

## Next experiments

Try [DVL](dvl.md), [USBL](usbl.md), [currents](currents.md) or the [world library](worlds.md). First-use Fuel downloads can delay startup. If data does not arrive, inspect the advancing clock, then plugin and resource logs using [Troubleshooting](troubleshooting.md).

## Choose launch arguments

Use `--show-args` to inspect arguments accepted by a robot or sensor launch.

```bash
ros2 launch posim_demos posim_sensor.launch.py --show-args
```

| Argument | Meaning | Sensor launch default |
| --- | --- | --- |
| `namespace` | Select an installed model directory | Empty; set explicitly |
| `world_name` | Select a world file | `empty.sdf` |
| `paused` | Start paused | `true` |
| `gui` / `headless` | Show / hide Gazebo GUI | `true` / `false` |
| `use_sim_time` | Use the simulation clock in ROS nodes | `true` |
| `x`, `y`, `z` | Initial position, m | `0.0` each |
| `roll`, `pitch`, `yaw` | Initial attitude, rad | `0.0` each |
| `debug` / `verbosity_level` | Verbose logging / log level | `false` / `1` |
| `use_ned_frame` | Use NED frame | `false` |

Use `paused:=false` in sensor examples to produce data. Inspect robot/world launch arguments with that launch's `--show-args`.
