# Marine GNSS

The public `posim_gz_gnss` system simulates a satellite receiver at a model's antenna link. It converts the antenna's world position to geographic coordinates, adds configurable Gaussian position/velocity error, and publishes an atomic measurement with a timestamp and validity state. It models loss of reception under water, explicit obstruction, reacquisition and latency.

## Build and mount a receiver

In [terminal A prepared for your installation](terminals.md), including the Linux desktop reached through RDP on macOS:

```bash
cd "$POSIM_WS"
source /opt/ros/lyrical/setup.bash
colcon build --merge-install --packages-select posim_gz_gnss posim_robot_models
source install/setup.bash
```

Your model needs a physical antenna link, usually `gnss_link`, with its intended pose and attachment to the vehicle. The world needs WGS 84 [spherical coordinates](coordinates.md). Add the following plugin inside the model SDF, replacing the topic and link name as needed:

```xml
<plugin filename="libposim_gnss.so" name="posim::Gnss">
  <link_name>gnss_link</link_name>
  <topic>/model/my_robot/gnss</topic>
  <enabled>true</enabled>
  <water_level>0</water_level>
  <update_rate>5</update_rate>
  <latency>0.2</latency>
  <reacquire_seconds>3</reacquire_seconds>
  <dry_margin>0.05</dry_margin>
</plugin>
```

The [receiver profiles](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json) provide mount poses relative to `base_link`: WAM-V at z = 1.8 m; BlueROV2 variants at 0.3 m; REXROV at 0.8 m. WAM-V and the three BlueROV2 profiles enable the receiver; REXROV disables it by default. These JSON profiles are data for model-building callers. Installing them does not insert a link or plugin into every robot. WAM-V also needs its external [Wave Sim model](surface.md).

BlueROV2, BlueROV2 Heavy and the sonar variant include the receiver and the `marine_gnss_input` adapter. With ArduSub enabled, the adapter sends MAVROS `GPSINPUT` messages to the autopilot instead of SITL's synthetic GPS. It converts ENU velocity to NED and WGS 84 ellipsoid altitude to EGM96 mean-sea-level altitude using the installed GeographicLib dataset. Submerged, blocked, invalid, future or more than 1 s old measurements produce no fix. Satellite count is unknown (`255`) for a valid fix; satellite geometry and dilution are not simulated. The demo receiver uses mean water level z = 0, independently of the visual waves.

## Parameters

| SDF parameter | Unit / meaning | Default |
| --- | --- | --- |
| `update_rate` | Samples per simulation second; >0 to 100 Hz | 5 |
| `latency` | Fix delivery delay in simulation seconds; 0–10 s | 0.2 |
| `reacquire_seconds` | Continuous dry acquisition time, s | 3 |
| `dry_margin` | Clearance above water needed to acquire, m | 0.05 |
| `horizontal_stddev` | Horizontal position standard deviation, m | 0.7 |
| `vertical_stddev` | Vertical position standard deviation, m | 1.2 |
| `velocity_stddev` | Velocity standard deviation, m/s | 0.1 |
| `water_level` | Mean surface z when no wave topic is set, m | 0 |
| `seed` | Receiver random seed | 42 |
| `enabled` | Receiver enabled | true |
| `link_name` | Antenna link within the model | `gnss_link` |
| `topic` | Gazebo measurement topic | `/model/<model-name>/gnss` |
| `wave_topic` | Optional sampled-water-height topic | Empty |

Numeric receiver settings must be finite. Latency, acquisition time, dry margin and standard deviations must be nonnegative.

An empty `wave_topic` uses the mean-water plane. A nonempty topic requires fresh local height samples; missing or invalid data produces `surface unavailable`. The protocol uses `gz.msgs.Double_V`: schema `1`, grid count, periodic X/Y size, wind speed/angle, steepness, simulation time and row-major heights. The grid count is an integer from 2 to 256, followed by a number of height values equal to the square of that count. Tile sizes must be finite and positive; the finite timestamp must be no more than 0.6 simulation seconds old and cannot be in the future. The receiver consumes this grid; configure a compatible publisher separately. Wind metadata does not control reception.

## Read measurements and obstruction state

After starting your configured model, use the prepared terminal B to read its topic:

```bash
gz topic -e -t /model/my_robot/gnss
```

Each message contains 15 doubles. Indices are zero-based; retain the acquisition timestamp even when a fix arrives after the configured delay.

| Index | Field / unit |
| --- | --- |
| 0–3 | Schema `1`, reset epoch, acquisition time in simulation seconds, state |
| 4–6 | Latitude degrees, longitude degrees, altitude meters |
| 7–9 | East, north and up velocity, m/s |
| 10–12 | Horizontal/vertical position variance in m², velocity variance in (m/s)² |
| 13–14 | Antenna world z and local water height, m |

States are `0 disabled`, `1 submerged`, `2 acquiring`, `3 fix`, `4 blocked` and `5 surface unavailable`. Invalid measurements have NaN coordinates and velocities. Water contact or blockage clears queued fixes. Reacquisition needs continuous clearance above the dry margin. Reset clears the timer and delayed samples and advances the epoch.

Publish an obstruction input; use `false` to clear it:

```bash
gz topic -t /model/my_robot/gnss/blocked -m gz.msgs.Boolean -p 'data: true'
```

## Interpretation and checks

Compare a stationary dry antenna, a submerged antenna, a blocked antenna and recovery. Check acquisition time, delay, epoch after reset and increasing measurement timestamps. Consumers must reject stale/future fixes and handle invalid states.

The supplied BlueROV2 adapter feeds ArduSub; other ROS `NavSatFix` consumers and custom autopilots still need an adapter.

The receiver is a simplified pose-based model. It does not compute satellite geometry, RF propagation, physical multipath or underwater acoustic positioning. Configurable accuracy values are simulation inputs, not a vendor accuracy guarantee. See [the receiver implementation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc) and [package guide](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss).
