# Doppler velocity log

A DVL estimates velocity relative to the seabed or water mass. POSIM uses Gazebo's DVL system and a ROS bridge that converts observations to `posim_interfaces/msg/DVL`.

## Run the sensor

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world z:=-30 \
  paused:=false gui:=false headless:=true
```

In a second sourced terminal, inspect Gazebo and then ROS data:

```bash
gz topic -e -t /dvl/velocity
ros2 topic echo /dvl/velocity posim_interfaces/msg/DVL --once
```

Stop the Gazebo viewer before the ROS command. Match the topic to your descriptor if you create a new sensor.

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

The example converts ENU to its configured forward/starboard/down reference using `<reference_frame>`. The ROS bridge preserves the Gazebo frame ID. Check that your estimator uses the same frame and distinguishes bottom tracking from water-mass tracking.

## Inspect observations

Use `ros2 interface show posim_interfaces/msg/DVL` for the payload definition. Confirm received velocity, frame and status with a known stationary case, then a controlled translation. Bottom geometry, range and beam intersections affect the observations. Enable the descriptor's visualization settings when inspecting beams on a configured desktop.

## DVL beams and output

<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL acoustic beams directed at the seabed" loading="lazy" decoding="async"></a><figcaption>DVL acoustic beams directed at the seabed</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif"><img width="754" height="476" src="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif" alt="DVL velocity observations" loading="lazy" decoding="async"></a><figcaption>DVL velocity observations</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png"><img width="473" height="417" src="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png" alt="Gazebo DVL topic inspection" loading="lazy" decoding="async"></a><figcaption>Gazebo DVL topic inspection</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png"><img width="528" height="87" src="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png" alt="ROS DVL topic inspection" loading="lazy" decoding="async"></a><figcaption>ROS DVL topic inspection</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
