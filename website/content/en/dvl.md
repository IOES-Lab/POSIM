# Doppler velocity log

A Doppler velocity log (DVL) estimates velocity relative to the seabed or surrounding water. POSIM uses Gazebo's DVL system and a ROS bridge that converts observations to `posim_interfaces/msg/DVL`.

## Run the sensor

Run in [terminal A prepared for your installation](terminals.md):

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world z:=-30 \
  paused:=false gui:=true headless:=false
```

In the prepared terminal B, inspect Gazebo, stop the viewer with **Ctrl+C**, then inspect ROS data:

```bash
gz topic -e -t /dvl/velocity
ros2 topic echo /dvl/velocity posim_interfaces/msg/DVL --once
```

Match the topic to your descriptor if you create a new sensor.

## SDF integration

Put these systems directly inside `<world>` for DVL processing and rendering:

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="gz-sim-dvl-system"
        name="gz::sim::systems::DopplerVelocityLogSystem"/>
```

Place the POSIM bridge inside the sensor's `<model>`. Match its topic to the DVL sensor's `<topic>` inside `<link>`:

```xml
<plugin filename="DVLBridge" name="posim_ros_gz_plugins::DVLBridge">
  <topic>/dvl/velocity</topic>
</plugin>
```

The [complete DVL500-300 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf) defines the `type="custom" gz:type="dvl"` sensor, four beams and tracking blocks. The [world SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world) supplies bottom-tracking geometry.

## Parameters and units

`DVLBridge` accepts one parameter, `topic`, defaulting to `/dvl/velocity`. It subscribes to Gazebo and publishes ROS data on that path. The remaining settings below belong to the Gazebo sensor and show the DVL500-300 example values.

| Sensor SDF field | Meaning | Example value |
| --- | --- | --- |
| `topic`, `update_rate` | Observation path and requested update rate (Hz) | `/dvl/velocity`, 8 |
| `gz:dvl/type` | Transducer arrangement | `phased_array` |
| `arrangement` beam `aperture`, `rotation`, `tilt` | Angles; `degrees="true"` selects degrees | Aperture 3°, rotations −135/135/45/−45°, tilt 25° |
| `minimum_range`, `maximum_range` | Acoustic range, m | 0.3, 200 |
| `reference_frame` | Reference pose: position in m, rotation in rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | Tracking selection | Both `best` |
| Tracking `noise/stddev` | Velocity noise standard deviation, m/s | Bottom 0.005; water mass 0.015 |
| Water-mass `water_velocity` | x/y/z components, m/s | 0/0/0 |
| Water-mass `boundaries/near`, `boundaries/far`, `bins` | Sample bounds in m and bin count | 10, 100, 10 |
| Tracking `visualize` | Show sampling geometry | true |

See the [bridge implementation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc) for message conversion. The depth sensor uses Gazebo rendering; complete your [environment setup](requirements.md) before launching it.

## Available descriptions

| Description | Configuration family |
| --- | --- |
| `nortek_dvl500_300` | Nortek DVL500 |
| `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300` | Nortek DVL1000 |
| `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000` | Teledyne Explorer |
| `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL plus [CUDA sonar](sonar-tuning.md) |

Check each sensor model's sampling and geometry in SDF.

## Configure a DVL

Configure `gz:type="dvl"` with beam angles/tilt, bottom/water tracking, velocity noise and reference frame.

- DVL500-300 requested rate: **8 Hz**
- Range: **0.3–200 m**

The example converts ENU to its configured forward/starboard/down reference using `<reference_frame>`. The ROS bridge preserves the Gazebo frame name, replacing `::` separators with `/`. Check that your estimator uses the same frame and distinguishes bottom tracking from water-mass tracking.

The aggregate velocity and each beam carry a ROS 6×6 twist covariance. The measured 3×3 linear-velocity covariance occupies its upper-left block (units m²/s²). The remaining entries are zero because the DVL supplies no angular-velocity estimate; do not treat them as measured zero uncertainty.

## Inspect observations

Use `ros2 interface show posim_interfaces/msg/DVL` to read the message definition. Check velocity, coordinate frame and status while the robot is stationary, then during a controlled movement. Bottom geometry, range and beam intersections affect the observations. Enable the descriptor's visualization settings when inspecting beams on a configured desktop.

## DVL beams and output

<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL acoustic beams directed at the seabed" loading="lazy" decoding="async"></a><figcaption>DVL acoustic beams directed at the seabed</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif"><img width="754" height="476" src="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif" alt="DVL velocity observations" loading="lazy" decoding="async"></a><figcaption>DVL velocity observations</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png"><img width="473" height="417" src="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png" alt="Gazebo DVL topic inspection" loading="lazy" decoding="async"></a><figcaption>Gazebo DVL topic inspection</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png"><img width="528" height="87" src="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png" alt="ROS DVL topic inspection" loading="lazy" decoding="async"></a><figcaption>ROS DVL topic inspection</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
