# Sea pressure

The pressure plugin converts model depth to hydrostatic pressure and publishes ROS `FluidPressure` data. It can also publish a pressure-derived depth estimate.

## Observe pressure on REXROV

Start the [REXROV example](quickstart.md), then run in a second sourced terminal:

```bash
ros2 topic echo /model/rexrov/sea_pressure sensor_msgs/msg/FluidPressure --once
ros2 topic echo /model/rexrov/sea_pressure_depth geometry_msgs/msg/PointStamped --once
```

Pressure is published in **Pa**, with variance in Pa². The depth message uses meters.

## Add the plugin to a model

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
| `standard_pressure` | Surface reference pressure, **kPa**; default 101.325 |
| `kPa_per_meter` | Hydrostatic increase per meter; default 9.80638 |
| `estimate_depth_on` | Publish the derived depth; default true |
| `update_rate` | Requested publication rate in Hz; nonpositive means every physics update |

The model uses `depth = max(0, -z)` with the sea surface at local z = 0. It computes pressure in kPa and converts it to Pa for publication. Place your world and model consistently with this sea-level convention.

## Check a depth experiment

Compare measurements at two known z positions while keeping the pressure slope fixed. The pressure difference should follow the configured slope multiplied by the depth change. The derived depth uses the same model, so it is not an independent depth measurement. Configure your estimator's units and reference pressure accordingly.
