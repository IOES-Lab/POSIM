# POSIM marine GNSS

`posim::Gnss` is an Apache-2.0 Gazebo Sim system. It measures the world pose of
`gnss_link`, applies Gaussian receiver errors, and publishes one atomic
`gz.msgs.Double_V` measurement. It uses the world's WGS 84 spherical coordinates.
It supports any model with a named antenna link; vehicle mounts and receiver
defaults are in `config/profiles.json`. BlueROV mounts are optional and disabled
by default. The WAM-V model requires the external Wave Sim `wam-v` model.

Build:

```sh
source /opt/ros/lyrical/setup.bash
colcon build --packages-select posim_gz_gnss posim_robot_models
source install/setup.bash
```

The model plugin accepts the receiver keys in the profile plus `enabled`,
`link_name`, `topic` and `wave_topic`. An empty wave topic selects the mean
`water_level` plane. A nonempty wave topic requires a fresh periodic sampled
heightfield (version 1, count, tile X/Y, wind speed/angle, steepness, simulation
time, row-major heights). Wind metadata does not gate reception.

## Atomic measurement

| Index | Field |
|---|---|
| 0–3 | schema=1, reset epoch, acquisition simulation seconds, state |
| 4–6 | latitude degrees, longitude degrees, altitude metres |
| 7–9 | east, north, up velocity in m/s |
| 10–12 | horizontal position variance, vertical position variance, velocity variance |
| 13–14 | world antenna height, local surface height in metres |

States: `0 disabled`, `1 submerged`, `2 acquiring`, `3 fix`, `4 blocked`,
`5 surface unavailable`. Invalid measurements contain NaN coordinates and
velocities. Immersion or blockage clears delayed fixes. Acquisition needs
continuous clearance above the dry margin. Reset clears the acquisition timer
and delay queue and advances the epoch. `/model/{name}/gnss/blocked` accepts an
explicit `gz.msgs.Boolean` obstruction input.

This is a simplified satellite receiver, with no acoustic UGPS, RF propagation,
satellite geometry or physical multipath solver. Nominal accuracy is configurable,
not a vendor product guarantee. Consumers are responsible for measurement
freshness, estimator source selection, coordinate conversion and distinguishing
sensor measurements from estimated state. This plugin publishes Gazebo sensor
measurements; it does not implement an autopilot integration or navigation filter.
