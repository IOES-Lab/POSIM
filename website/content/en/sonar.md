# Multibeam sonar

POSIM's multibeam sonar uses a ray-based point-scattering model to generate intensity–range data and sonar imagery. The same plugin offers CPU, WGPU and CUDA computation. WGPU supports GPU vendors through Vulkan on Linux or Metal in a native macOS environment; see [backend setup and selection](sonar-tuning.md).

## Run a sonar example

Use [prepared POSIM terminals A and B](terminals.md). Build the [sonar packages](sonar-tuning.md#build-the-sonar-packages) if they are not installed. Start the simulation in terminal A and inspect its outputs in terminal B. The first command selects a backend automatically; the dedicated example below explicitly selects WGPU and also opens RViz.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=blueview_p900 world_name:=posim_multibeam_sonar \
  paused:=false x:=4 z:=2.0 yaw:=3.14 gui:=true headless:=false
```

A dedicated example is also supplied by `posim_multibeam_sonar_demo`:

```bash
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

Use one launch at a time. Replace `wgpu` with `cpu`, `cuda` or `auto` to choose another backend. For the robot variant, select `bluerov2_heavy_multibeam_sonar` with a suitable sonar world.

## Observe outputs

```bash
ros2 topic list -t
ros2 topic echo /sensor/multibeam_sonar/sonar_image \
  sensor_msgs/msg/Image --once --no-arr --qos-reliability reliable
```

The image publisher uses reliable QoS. Use a reliable subscription when inspecting these large frames, including the RViz Image display. Add PointCloud2 for `/sensor/multibeam_sonar/point_cloud`. Raw returns use the acoustic message interface supplied by `marine_acoustic_msgs`; discover their type with `ros2 topic list -t`.

The BlueView sensor launch bridges Gazebo simulation time to ROS `/clock`. For consumers using `use_sim_time`, confirm that the world is running and `/clock` advances.

The sonar publishes image and raw acoustic messages directly to ROS 2. The sensor configuration bridges the Gazebo point cloud to ROS 2. Discover custom topic names with `ros2 topic list -t`.

## How the sensor works

Gazebo supplies scene geometry. The sonar model estimates surface normals, computes acoustic backscatter, applies the receive response and compresses returns into range bins. The resulting acoustic data and display image describe the same observation. Use raw data for analysis and retain the gain and display settings with recorded images.

With a positive `spec/receiveBeamWidth`, the physical readout uses measured XYZ from a depth camera covering the complete sonar fan. Geometry samples, receiver directions and range outputs are independent. Cells with centers inside the fan partition its full solid angle; their projected surface areas and signed receive response determine the beam contributions. A fixed-bandwidth, unit-peak baseband Hamming pulse uses a longer internal calculation interval than the published range interval. Range spacing is `soundSpeed / (2 × bandwidth)`; calculation length does not set pulse gain.

Set `ray/scan/horizontal/samples` for geometry columns, `ray/scan/vertical/rays` for image rows and `beams` for receiver directions. The physical mode requires a symmetric fan narrower than 180° on each axis. Range limits are radial; pixels outside the fan or range limits have invalid XYZ in the organized point cloud. `spec/receiveBeamWidth` specifies the full power half-maximum width in radians, and `spec/receiveGain` sets linear pressure gain (default 1). Missing or zero width selects the default grid-based readout. The independent physical mode supports CPU and WGPU; CUDA does not support this mode. There is no fixed 4096-sample frequency cap. Larger grids remain subject to backend resource limits and representable dimensions. The internal guard interval is not published and does not change the output spacing. Scattering coefficients are generated per frame and geometry cell. This model represents baseband returns; carrier-phase propagation and hardware amplitude calibration are not provided.

## Sensor configuration

Add the systems directly inside `<world>`:

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="libmultibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

Place this P900 sensor inside a model `<link>`:

```xml
<sensor name="multibeam_sonar" type="custom" gz:type="multibeam_sonar">
  <always_on>true</always_on>
  <update_rate>30.0</update_rate>
  <topic>/sensor/multibeam_sonar</topic>
  <visualize>true</visualize>
  <gz:multibeam_sonar>
  <ray degrees="false">
    <scan>
      <horizontal>
        <beams>512</beams>
        <min_angle>-1.13447</min_angle>
        <max_angle>1.13447</max_angle>
      </horizontal>
      <vertical>
        <rays>300</rays>
        <min_angle>-0.10472</min_angle>
        <max_angle>0.10472</max_angle>
      </vertical>
    </scan>
    <range>
      <min>0.1</min>
      <max>10.0</max>
    </range>
  </ray>
    <spec>
        <!-- Sonar properties -->
      <verticalFOV>12</verticalFOV>
      <sonarFreq>900e3</sonarFreq>
      <bandwidth>29.9e3</bandwidth>
      <soundSpeed>1500</soundSpeed>
      <sourceLevel>220</sourceLevel>
      <maxDistance>10</maxDistance>
      <raySkips>10</raySkips>
      <sensorGain>0.02</sensorGain>
      <blazingSonarImage>true</blazingSonarImage>
      <writeLog>false</writeLog>
      <debugFlag>false</debugFlag>
      <writeFrameInterval>5</writeFrameInterval>
      <!-- ROS publication topics -->
      <pointCloudTopicName>point_cloud</pointCloudTopicName>
      <sonarImageRawTopicName>sonar_image_raw</sonarImageRawTopicName>
      <sonarImageTopicName>sonar_image</sonarImageTopicName>
      <frameName>forward_sonar_optical_link</frameName>
    </spec>
  </gz:multibeam_sonar>
</sensor>
```

The descriptor uses `type="custom" gz:type="multibeam_sonar"`. The world loads `multibeam_sonar_system`. The BlueView P900 example requests:

| Setting | Value |
| --- | --- |
| Horizontal beams | 512, approximately 130° field of view |
| Vertical rays | 300, approximately 12° field of view |
| Range | 0.1–10 m |
| Frequency / bandwidth | 900 kHz / 29.9 kHz |
| Sound speed | 1500 m/s |
| Sensor update rate | 30 Hz requested |

The actual received rate depends on computation and rendering. Read [the sensor model](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf) for all values.

## Parameters to customize

| `<spec>` parameter | Meaning | Code default | P900 example |
| --- | --- | --- | --- |
| `verticalFOV` | Acoustic vertical beam width, degrees | 10 | 12 |
| `sonarFreq` | Acoustic frequency, Hz | 900,000 | 900,000 |
| `bandwidth` | Signal bandwidth, Hz | 29,500,000 | 29,900 |
| `soundSpeed` | Sound speed, m/s | 1500 | 1500 |
| `sourceLevel` | Source level, dB | 220 | 220 |
| `maxDistance` | Processing range, m; match ray range | 60 | 10 |
| `raySkips` | Elevation sampling stride; use a positive integer | 10 | 10 |
| `sensorGain` | Echo amplitude gain for the default CPU grid mode; unused by physical mode and WGPU/CUDA | 0.02 | 0.02 |
| `blazingSonarImage` | Use a wall-clock-dependent random seed; false uses seed 1234 | false | true |
| `pointCloudTopicName` | Gazebo point-cloud suffix | `point_cloud` | Same |
| `sonarImageRawTopicName` | ROS raw-return suffix | `sonar_image_raw` | Same |
| `sonarImageTopicName` | ROS image suffix | `sonar_image` | Same |
| `frameName` | Image optical frame ID | `forward_sonar_optical_link` | Same |
| `writeLog` | Write raw CSV files in a separate run directory | false | false |
| `writeFrameInterval` | Frames between CSV writes; use a positive integer | 10 | 5 |
| `debugFlag` | Diagnostic console output | false | false |
| `plotScaler` | Parsed and printed, but not used in image generation | 10 | Omitted |

### Topics and frames

Topic suffixes are appended to the sensor's `<topic>`, here `/sensor/multibeam_sonar`. Raw observations use `marine_acoustic_msgs/msg/ProjectedSonarImage`. Point clouds use Gazebo `gz.msgs.PointCloudPacked` and need the included ROS bridge. Match `pointCloudTopicName` to `sensor_config.py` and set `frameName` to the sonar's optical frame.

The BlueView P900 and BlueROV2 sonar launch files publish an identity transform from `world` to the sonar frame for RViz viewing. Point-cloud coordinates remain relative to the sensor. This static transform does not track the vehicle or account for the sonar's mounting pose. For mapping in world coordinates, replace it with a transform chain that uses both poses.

### Ray geometry

The `<ray>` block defaults to 256 horizontal beams, 3 vertical rays, horizontal angles ±π/4, vertical angles ±π/8 and range 0.1–5 m. Set `degrees="true"` to specify scan angles in degrees; otherwise use radians. The optional `<reference_frame>` inside `<gz:multibeam_sonar>` defaults to the zero pose. Keep the complete `<ray>` and `<spec>` blocks; values above are defaults for fields within them. The [implementation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc) is the authority for parsing and publication.

Keep the ray range and `maxDistance` consistent.

### Repeatable measurements

`writeLog`, `writeFrameInterval` and `debugFlag` support diagnostic recording and timing. Keep logging disabled for throughput measurements unless logging is part of the experiment. See [Configure and tune sonar](sonar-tuning.md) for a repeatable tuning workflow and [Citation](citation.md) for the sonar model's research source.

### Compute backend {#compute-backend}

| Selection | Behavior |
| --- | --- |
| `auto` | Try CUDA, then WGPU, then CPU; the default |
| `wgpu` | Use a compatible Vulkan/Metal adapter; fail sensor initialization if unavailable |
| `cuda` | Use an NVIDIA GPU with the CUDA backend built and installed; fail if unavailable |
| `cpu` | Compute acoustic returns on the CPU; Gazebo rendering is still required |

The dedicated launch accepts `compute_backend`. Other launches read the environment variable `POSIM_SONAR_COMPUTE_BACKEND`. Check `Using sonar compute backend:` in the launch output. [Configure and tune sonar](sonar-tuning.md) explains adapter selection, platform requirements and numerical differences.

## Add a sonar sensor and world

Copy `models/posim_sensor_models/description/blueview_p900/` to a new sensor identifier. Also prepare a matching `config/<sensor identifier>/sensor_config.py`; align SDF model/link/frame names with bridge topics. Set horizontal/vertical samples, angles and range in `<ray>` to match your sensor specification.

Include the Gazebo rendering-sensor system and sonar system in the world. Consult the [sonar world SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world) for physics, scene and other custom-sensor configuration.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="libmultibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

Prepare a compute backend and rendering, rebuild the workspace and load its environment, then select your sensor model using the [sensor launch arguments](quickstart.md). The ROS point-cloud bridge connects `sensor_msgs/msg/PointCloud2` to Gazebo `gz.msgs.PointCloudPacked`. Match `pointCloudTopicName` to the path configured in `sensor_config.py`.

## Record raw sonar data

With `writeLog=true`, each sensor creates a separate `SonarRawData_<run>_<suffix>/` directory under the simulator's working directory and prints its path. It contains numbered raw-data CSV files and `SonarRawData_beam_angles.csv`. Existing recordings are preserved. Set `writeFrameInterval` to a positive frame count; values below 1 are clamped to 1. If the directory cannot be created, logging is disabled and the sensor reports the error.

In the printed recording directory, open the first frame with the installed [plotting utility](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py). It reads the beam count, ranges and beam angles from the CSV files.

```bash
ros2 run posim_multibeam_sonar_demo plotdata.py SonarRawData_000001.csv
```

To save the plotted observations as PNG files:

```bash
ros2 run posim_multibeam_sonar_demo plotdata.py SonarRawData_000001.csv \
  --output-dir plots --no-show
```

The utility saves a sonar view and five evenly spaced beam profiles (or all beams when fewer than five were recorded). Colors show echo level relative to the frame's peak; use raw CSV values for comparisons between runs. `--range-max` limits the displayed distance in meters, `--beam-step` samples every Nth beam, and `--dynamic-range` sets the displayed span below the peak in dB (default 60). The plotting utility also installs without CUDA.

Monitor disk usage. `sonarImageRawTopicName` names raw acoustic observations; `sonarImageTopicName` names the display image. Record `blazingSonarImage` with the acoustic settings when comparing repeated experiments.

The CSV's first column contains range; the remaining columns contain complex beam returns. The plotting script reads the beam count from the file. Keep the matching beam-angle CSV alongside each recording, and use the recorded range values when plotting.

## Sonar scenes and observations

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png"><img width="1051" height="549" src="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png" alt="Point-scattering multibeam sonar processing" loading="lazy" decoding="async"></a><figcaption>Point-scattering multibeam sonar processing</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="RGB, depth, sonar imagery and point cloud from one scene" loading="lazy" decoding="async"></a><figcaption>RGB, depth, sonar imagery and point cloud from one scene</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif"><img width="764" height="472" src="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif" alt="Changing imagery in the sonar sensor example" loading="lazy" decoding="async"></a><figcaption>Changing imagery in the sonar sensor example</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif"><img width="800" height="561" src="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif" alt="Sonar observations and scene changes" loading="lazy" decoding="async"></a><figcaption>Sonar observations and scene changes</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif"><img width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif" alt="Sonar observations from BlueROV2" loading="lazy" decoding="async"></a><figcaption>Sonar observations from BlueROV2</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif"><img width="800" height="367" src="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif" alt="Local scene search with sonar" loading="lazy" decoding="async"></a><figcaption>Local scene search with sonar</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
