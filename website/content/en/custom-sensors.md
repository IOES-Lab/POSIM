# Add & configure sensors

A sensor needs its model definition, the system that updates it and a route to the consumer. Add one sensor at a time and verify its measurements before adding another.

## Where to run this guide

Use the Linux terminals prepared in [Prepare your terminals](terminals.md). This applies to native Ubuntu, Ubuntu in WSL, and POSIM containers. On macOS, use the Linux terminal inside the RDP desktop. Edit and build inside that environment; `POSIM_WS` is its workspace path.

## Choose the integration point

| Measurement | Model placement | World system / ROS path |
|---|---|---|
| IMU | `link/sensor type="imu"` | Gazebo IMU system; bridge `gz.msgs.IMU` to ROS `Imu` |
| [Underwater camera](camera.md) | RGBD sensor and its image-effects plugin | Gazebo Sensors system with a renderer; configured camera topics |
| [Doppler velocity log](dvl.md) | DVL sensor and beam configuration | DVL system and the supplied acoustic-message bridge |
| [Sea pressure](pressure.md) | Plugin directly under `model` | Publishes ROS pressure/depth directly |
| [Marine GNSS](gnss.md) | Receiver plugin on the robot model | Gazebo `gz.msgs.Double_V` messages; a separate adapter is needed for ROS |
| [USBL positioning](usbl.md) | Transceiver and transponder plugins | Matching acoustic IDs and configured ROS interfaces |
| [Multibeam sonar](sonar.md) | Custom sonar sensor under a link | Sensors renderer plus multibeam world system; CUDA backend |

Use each reference's SDF block and parameter table. A plugin's placement matters: world systems do not belong inside a sensor, and model plugins need the model entity they operate on.

## Add an IMU to the example world

Start with `calm_water.sdf` from [Create a world](custom-worlds.md). Add this system directly inside `<world>`, alongside the physics system:

```xml
<plugin filename="gz-sim-imu-system" name="gz::sim::systems::Imu"/>
```

Add this sensor inside the `pressure_station` model's `<link name="base_link">`:

```xml
<sensor name="imu" type="imu">
  <pose>0 0 0.1 0 0 0</pose>
  <always_on>true</always_on>
  <update_rate>20</update_rate>
  <topic>/tutorial/imu</topic>
  <imu/>
</sensor>
```

The mount pose is relative to its link: position is in metres and rotation in radians. The rate is in simulation-time Hz. Here the station is fixed, so angular velocity should remain near zero; accelerometer values include the gravity response.

## Connect and inspect

Stop the previous world, validate the edited file and start it again in Terminal A:

```bash
gz sdf -k calm_water.sdf
gz sim -r calm_water.sdf
```

In sourced **Terminal B**, start the bridge and leave it running:

```bash
ros2 run ros_gz_bridge parameter_bridge \
  '/tutorial/imu@sensor_msgs/msg/Imu[gz.msgs.IMU'
```

In sourced **Terminal C**, inspect the ROS measurement:

```bash
ros2 topic echo --no-daemon /tutorial/imu sensor_msgs/msg/Imu --once
```

Confirm a timestamp, frame ID, angular velocity and acceleration. To watch updates continuously, remove `--once`; Ctrl+C stops the viewer. Stop the bridge and simulator when finished. [ROS terminals & topics](ros.md) explains how the terminals share an environment.

## Change a setting and check the result

Change `update_rate` from `20` to `5`, then restart the world and bridge. Watch the differences between successive message timestamps. At a steady simulated rate they should change from roughly `0.05 s` to `0.2 s`; received wall-time frequency also depends on real-time factor and transport.

For cameras, check resolution, field of view and near/far range together. For scanning sensors, check beam geometry and range limits. Move a sensor only after confirming the link frame and its orientation.

## Keep an integration checklist

- Model: valid mount pose, sensor settings and unique topic.
- World: the required system and, for image sensors, a working renderer.
- ROS: the correct bridge direction, message type, frame and QoS.
- Observation: fresh timestamps and a response to a controlled scene change.

For seabed measurements, verify terrain placement with [Heightmap terrain](heightmaps.md). Use [Configure & tune sonar](sonar-tuning.md) for a repeatable sonar comparison, with the [Multibeam sonar](sonar.md) parameter reference beside it.
