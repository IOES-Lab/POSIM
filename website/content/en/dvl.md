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

These are simulation descriptions. Read each SDF for the configured sampling and geometry rather than treating its name as a sensor certification.

## Configure a DVL

The custom sensor uses `gz:type="dvl"`. Its configuration includes beam aperture/rotation/tilt, bottom and water-mass tracking, velocity noise, minimum/maximum range and a reference-frame rotation. The default DVL500-300 example requests 8 Hz and a range of 0.3–200 m.

The example converts ENU to its configured forward/starboard/down reference using `<reference_frame>`. The ROS bridge preserves the Gazebo frame ID. Check that your estimator uses the same frame and distinguishes bottom tracking from water-mass tracking.

## Inspect observations

Use `ros2 interface show posim_interfaces/msg/DVL` for the payload definition. Confirm received velocity, frame and status with a known stationary case, then a controlled translation. Bottom geometry, range and beam intersections affect the observations. Enable the descriptor's visualization settings when inspecting beams on a configured desktop.
