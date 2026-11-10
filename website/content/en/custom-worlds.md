# Create a world

Build a small, self-contained world before adding terrain and a moving robot. This example holds a pressure sensor at a known depth so you can change the world and measure the result.

## Where to run this guide

Use the Linux terminals prepared in [Prepare your terminals](terminals.md). This applies to native Ubuntu, Ubuntu in WSL, and POSIM containers. On macOS, use the Linux terminal inside the RDP desktop. Edit and build inside that environment; `POSIM_WS` is its workspace path.

## Save a complete world

In your sourced POSIM workspace, save this as `calm_water.sdf`. It uses primitive geometry and installed plugins, so it needs no downloaded meshes.

```xml
<?xml version="1.0"?>
<sdf version="1.12">
  <world name="calm_water">
    <physics name="physics" type="ignored">
      <max_step_size>0.001</max_step_size>
      <real_time_factor>1.0</real_time_factor>
    </physics>
    <gravity>0 0 -9.80665</gravity>
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
    <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands"/>
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
    <scene>
      <ambient>0.4 0.5 0.6 1</ambient>
      <background>0.05 0.15 0.25 1</background>
    </scene>
    <light name="sun" type="directional">
      <direction>-0.5 0.2 -1</direction>
      <diffuse>0.8 0.8 0.8 1</diffuse>
    </light>
    <model name="seabed">
      <static>true</static>
      <pose>0 0 -20 0 0 0</pose>
      <link name="ground">
        <collision name="collision">
          <geometry><box><size>40 40 0.2</size></box></geometry>
        </collision>
        <visual name="visual">
          <geometry><box><size>40 40 0.2</size></box></geometry>
          <material><diffuse>0.6 0.5 0.3 1</diffuse></material>
        </visual>
      </link>
    </model>
    <model name="pressure_station">
      <static>true</static>
      <pose>0 0 -2 0 0 0</pose>
      <link name="base_link">
        <visual name="marker">
          <geometry><box><size>0.2 0.2 0.2</size></box></geometry>
        </visual>
      </link>
      <plugin filename="sea_pressure_sensor"
              name="posim_gz_sensor_plugins::SubseaPressureSensorPlugin">
        <namespace>pressure_station</namespace>
        <topic>sea_pressure</topic>
        <standard_pressure>101.325</standard_pressure>
        <kPa_per_meter>9.80638</kPa_per_meter>
        <estimate_depth_on>true</estimate_depth_on>
        <update_rate>10</update_rate>
      </plugin>
    </model>
  </world>
</sdf>
```

This world contains:

- **Physics:** a maximum step of 0.001 s, a target real-time factor of 1 and downward gravity.
- **Light and scene:** a directional sun, ambient light and background colour.
- **Seabed:** a fixed box with matching visual and collision geometry at z = −20 m.
- **Pressure station:** a fixed marker at z = −2 m. Its model-level [Sea pressure](pressure.md) plugin publishes pressure and estimated depth.


The fixed station is a depth-measurement fixture. Its pressure reference is the world z = 0 plane. Wave shape, buoyancy and vehicle motion are additional systems you can add after this baseline works.

## Validate and start

In Terminal A, from the directory containing the file:

```bash
gz sdf -k calm_water.sdf
gz sim -r calm_water.sdf
```

Validation should succeed, then a Gazebo window opens. `-r` starts simulation time. Select `pressure_station` in the entity tree and focus the view on it to find the marker.

In a second sourced terminal:

```bash
ros2 topic echo --no-daemon /model/pressure_station/sea_pressure sensor_msgs/msg/FluidPressure \
  --once
ros2 topic echo --no-daemon /model/pressure_station/sea_pressure_depth geometry_msgs/msg/PointStamped \
  --once
```

Expect about **120,938 Pa** and **2 m** depth. The [Sea pressure](pressure.md) plugin publishes ROS messages directly, so this example does not need a separate ROS bridge.

## Edit and observe the change

1. Stop Terminal A with Ctrl+C.
2. Change the station's pose from `0 0 -2 0 0 0` to `0 0 -4 0 0 0`.
3. Validate and start the file again.
4. Repeat the two topic commands. Expect about **140,551 Pa** and **4 m** depth.

Keep the pressure coefficients unchanged for this comparison. The measured increase should be **19,612.76 Pa**. Save the original and edited files with your experiment's observations.

## Know which file owns each setting

| SDF path | Unit | Setting in this example |
|---|---|---|
| `world/physics/max_step_size` | s | Maximum integration step: `0.001` |
| `world/physics/real_time_factor` | — | Target simulation/wall-time ratio: `1.0` |
| `world/gravity` | m/s² | Gravity vector |
| `world/scene` and `world/light` | RGBA / direction | Background, ambient light and sunlight |
| `model/pose` | m, rad | Placement of the model in the world |
| `model/static` | boolean | Hold the station and seabed fixed |
| `model/plugin` | plugin-specific | Pressure measurement settings; see [Sea pressure](pressure.md) |

The real-time factor is a target; achieved speed depends on the simulation workload. A model's sensor definition and its world system have different scopes. Continue with [Add & configure sensors](custom-sensors.md) to add an IMU and a ROS bridge to this same file.

## Add marine terrain and systems

- [Heightmap terrain](heightmaps.md): replace the simple seabed and verify visual/collision alignment.
- [Add a robot](custom-robots.md): prepare mechanics, propulsion and the robot's ROS configuration.
- [Ocean currents](currents.md): define world flow and the model's response to it.
- [Spherical coordinates](coordinates.md): set a geographic origin and use coordinate services.
- [Underwater scene effects](underwater-rendering.md): configure the rendering systems separately from physics.

Keep a packaged world under `models/posim_worlds/worlds` and its assets under that package's resource directories. Rebuild and source the workspace after adding resources; use installed resource URIs when sharing the world.
