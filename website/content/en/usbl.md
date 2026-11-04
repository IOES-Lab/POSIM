# USBL positioning

Ultra-short baseline (USBL) positioning uses acoustic signals to locate underwater equipment. This example connects one transceiver to two transponders. Request positions from all transponders or select one by ID.

## Start the tutorial world

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=usbl_tutorial headless:=true
```

The world already contains the transceiver and transponders. Start them together with the world launch.

## Receive positions

In a second terminal with the same environment loaded:

```bash
ros2 topic echo /USBL/transceiver_manufacturer_168/transponder_location \
  posim_interfaces/msg/Location --once
ros2 topic echo /USBL/transceiver_manufacturer_168/transponder_location_cartesian \
  posim_interfaces/msg/Location --once
```

Both messages carry `transponder_id`, but their `x`, `y`, `z` fields have different meanings:

| Output | x | y | z |
| --- | --- | --- | --- |
| `transponder_location` | Bearing, degrees | Range, meters | Elevation, degrees |
| `transponder_location_cartesian` | Relative x, meters | Relative y, meters | Relative z, meters |

The current implementation computes directions from the world-coordinate position difference; account for this convention when combining it with body-frame data.

## SDF integration

Place the transceiver plugin inside the `box` model. Each transponder ID corresponds to the model name at the same position in `transponder_attached_object`.

```xml
<plugin filename="UsblTransceiver" name="posim_gz_sensor_plugins::UsblTransceiver">
  <namespace>USBL</namespace>
  <transceiver_device>transceiver_manufacturer</transceiver_device>
  <transceiver_ID>168</transceiver_ID>
  <transponder_device>transponder_manufacturer</transponder_device>
  <transponder_ID>1,2</transponder_ID>
  <transponder_attached_object>sphere,sphere2</transponder_attached_object>
  <sound_speed>1540</sound_speed>
  <enable_ping_scheduler>true</enable_ping_scheduler>
  <ping_frequency>0.5</ping_frequency>
  <interrogation_mode>common</interrogation_mode>
</plugin>
```

Place this plugin in `sphere`; add another to `sphere2` with `transponder_ID` set to `2`. Match the transceiver name, ID and namespace on both ends.

```xml
<plugin filename="UsblTransponder" name="posim_gz_sensor_plugins::UsblTransponder">
  <namespace>USBL</namespace>
  <transceiver_device>transceiver_manufacturer</transceiver_device>
  <transceiver_ID>168</transceiver_ID>
  <transceiver_model>box</transceiver_model>
  <transponder_device>transponder_manufacturer</transponder_device>
  <transponder_ID>1</transponder_ID>
  <sound_speed>1540</sound_speed>
  <mu>0</mu>
  <sigma>0</sigma>
</plugin>
```

## Parameters and defaults

| Parameter | Applies to | Meaning / default |
| --- | --- | --- |
| `namespace` | Both | Required topic namespace, e.g. `USBL` |
| `transceiver_device`, `transceiver_ID` | Both | Required device name and ID forming topic paths |
| `transponder_device`, `transponder_ID` | Both | Required name and ID; transceiver accepts comma-separated IDs |
| `transponder_attached_object` | Transceiver | Required comma-separated model names, ordered like the IDs |
| `transceiver_model` | Transponder | Required transceiver model name for position lookup |
| `sound_speed` | Both | Reference acoustic speed at 10 °C and z = 0, m/s; default 1540; finite and positive |
| `enable_ping_scheduler` | Transceiver | Required boolean; false uses manually published pings |
| `ping_frequency` | Transceiver | Scheduled rate in simulation time, Hz; default 1; example 0.5; finite and positive |
| `interrogation_mode` | Transceiver | `common` or `individual`; default `common` |
| `mu`, `sigma` | Transponder | Position noise mean and standard deviation, m; defaults 0 and 1; finite values with sigma ≥ 0; example 0 and 0 |

Use existing model names and unique transponder IDs. Keep the ID and attached-model lists the same length. See the [complete tutorial SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) for model geometry and placement.

IDs are decimal integers from 0 to 2147483647, without a sign or leading zeros. Each plugin requires a model containing a link. Invalid configuration leaves the plugin inactive. Unknown channel and command targets are ignored; an unknown channel keeps the current selection. The first listed transponder is the initial channel. Scheduled pings pause with simulation time and emit at most once per update.

## Acoustic timing

Each position request has two simulated travel legs: the transponder waits for the outgoing signal, then the transceiver waits for the returning position. Each leg uses distance divided by the local device's sound speed. Fractional delays advance with simulation time, pause with the world, and are discarded when simulation time rewinds. A pending leg keeps one sample per transponder; additional requests or replies during that leg are ignored. Choose a ping interval longer than the expected round trip to receive each request.

The effective speed is `sound_speed + 0.017 × depth + 4 × (temperature − 10)`, with speed in m/s, depth in meters below z = 0 and temperature in °C. Above z = 0, the depth term is zero. Temperature defaults to 10 °C. Publish `std_msgs/msg/Float64` on `/USBL/transceiver_manufacturer_168/temperature` or `/USBL/transponder_manufacturer_1/temperature` for the example devices. A change applies to subsequent legs. Nonfinite temperatures and changes that produce a nonpositive or nonfinite speed are ignored.

This simple depth/temperature approximation follows the [DOSITS sound-speed overview](https://dosits.org/decision-makers/tutorials/science/sound-speed/). Travel uses straight-line distances; it does not model refraction, salinity, multipath or acoustic interference. The return leg uses the received position, including configured position noise.

The transceiver retains the current interrogation mode so transponders joining later can receive it. Common mode emits one broadcast per scheduled ping; individual mode sends one ping to the selected channel.

## Request all transponders

```bash
ros2 topic pub --once /USBL/transceiver_manufacturer_168/interrogation_mode \
  std_msgs/msg/String "data: 'common'"
ros2 topic pub --once /USBL/common_interrogation_ping \
  std_msgs/msg/String "data: 'ping'"
```

Common mode can publish multiple IDs on the same output topic. Keep each response's ID so you can identify its source.

## Select one transponder

```bash
ros2 topic pub --once /USBL/transceiver_manufacturer_168/channel_switch \
  std_msgs/msg/String "data: '1'"
ros2 topic pub --once /USBL/transponder_manufacturer_1/individual_interrogation_ping \
  std_msgs/msg/String "data: 'ping'"
```

Switching the channel selects individual mode. Set the channel to the requested transponder ID. Return to `common` mode when requesting all devices.

## Configure an installation

Configure devices and interrogation in SDF. Match the names and IDs on both ends.

- Devices: `namespace`, names/IDs and attached objects
- Acoustics: sound speed
- Scheduled requests: `enable_ping_scheduler`, `ping_frequency`
- Tutorial request rate: **0.5 Hz**

See [the tutorial SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) when changing devices or the schedule.

## Layout and interrogation modes

<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-5f8c9419.png"><img width="925" height="679" src="{{ASSET_PREFIX}}media/notion/usbl-5f8c9419.png" alt="USBL transceiver and transponder layout" loading="lazy" decoding="async"></a><figcaption>USBL transceiver and transponder layout</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-b388acb0.png"><img width="636" height="710" src="{{ASSET_PREFIX}}media/notion/usbl-b388acb0.png" alt="Common and individual interrogation message flow" loading="lazy" decoding="async"></a><figcaption>Common and individual interrogation message flow</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-a96c9419.png"><img width="1537" height="993" src="{{ASSET_PREFIX}}media/notion/usbl-a96c9419.png" alt="Transponder position observations" loading="lazy" decoding="async"></a><figcaption>Transponder position observations</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
