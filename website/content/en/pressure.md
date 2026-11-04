# Sea pressure

The pressure plugin converts model depth to hydrostatic pressure and publishes ROS `FluidPressure` data. It can also publish a pressure-derived depth estimate.

## Observe pressure on REXROV

Start REXROV in a sourced terminal:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

In a second sourced terminal:

```bash
ros2 topic echo /model/rexrov/sea_pressure sensor_msgs/msg/FluidPressure --once
ros2 topic echo /model/rexrov/sea_pressure_depth geometry_msgs/msg/PointStamped --once
```

Pressure is published in **Pa**, with variance in Pa². The depth message uses meters.

## Add the plugin to a model

Place this block directly inside `<model>` and set `namespace` to that model's name.

```xml
<plugin filename="sea_pressure_sensor"
        name="posim_gz_sensor_plugins::SubseaPressureSensorPlugin">
  <namespace>my_robot</namespace>
  <topic>sea_pressure</topic>
  <standard_pressure>101.325</standard_pressure>
  <kPa_per_meter>9.80638</kPa_per_meter>
  <estimate_depth_on>true</estimate_depth_on>
  <update_rate>10</update_rate>
</plugin>
```

Set `namespace` to your model identifier. The output path is `/model/<namespace>/<topic>`; the depth output appends `_depth`.

## Parameters and units

| Parameter | Meaning |
| --- | --- |
| `namespace` | Required model name used to look up pose and form output topics |
| `topic` | Output name; default `sea_pressure` |
| `standard_pressure` | Surface reference pressure, **kPa**; default 101.325 |
| `kPa_per_meter` | Hydrostatic increase, **kPa/m**; default 9.80638; use a positive value |
| `estimate_depth_on` | Publish the derived depth; default true |
| `update_rate` | Requested publication rate in Hz; default 0; nonpositive means every physics update |
| `saturation` | Accepted value, default 3000; the implementation does not clamp pressure with it |

The model uses `depth = max(0, -z)` with the sea surface at local z = 0. It computes pressure in kPa and converts it to Pa for publication. Place your world and model consistently with this sea-level convention.

The source publishes a fixed variance of 9,000,000 Pa² and does not add random pressure noise. Account for this when using the variance in an estimator. See the [plugin implementation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc) for the pressure and depth calculations.

## Check a depth experiment

Compare measurements at two known z positions while keeping the pressure slope fixed. The pressure difference should equal the configured pressure increase per meter multiplied by the depth change. The derived depth uses the same model, so it is not an independent depth measurement. Configure your estimator's units and reference pressure accordingly.

## Pressure and depth outputs

<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-ba8c9419.png"><img width="1419" height="923" src="{{ASSET_PREFIX}}media/notion/pressure-ba8c9419.png" alt="ROS pressure observations" loading="lazy" decoding="async"></a><figcaption>ROS pressure observations</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-8cdc9419.png"><img width="1842" height="892" src="{{ASSET_PREFIX}}media/notion/pressure-8cdc9419.png" alt="Gazebo pressure inspection" loading="lazy" decoding="async"></a><figcaption>Gazebo pressure inspection</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-6c1c9419.png"><img width="1526" height="830" src="{{ASSET_PREFIX}}media/notion/pressure-6c1c9419.png" alt="Pressure-derived depth output" loading="lazy" decoding="async"></a><figcaption>Pressure-derived depth output</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
