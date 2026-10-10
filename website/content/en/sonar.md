# Multibeam sonar

POSIM's multibeam sonar uses a ray-based point-scattering model to generate intensity–range data and sonar imagery. Prepare the [NVIDIA/CUDA backend](sonar-tuning.md) before running the sensor.

## Run a sonar example

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=blueview_p900 world_name:=posim_multibeam_sonar \
  paused:=false x:=4 z:=2.0 yaw:=3.14 gui:=false headless:=true
```

A dedicated example is also supplied by `posim_multibeam_sonar_demo`:

```bash
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py
```

Use one launch at a time. For the robot variant, select `bluerov2_heavy_multibeam_sonar` with a suitable sonar world.

## Observe outputs

```bash
ros2 topic list -t
ros2 topic echo /sensor/multibeam_sonar/sonar_image \
  sensor_msgs/msg/Image --once --no-arr
```

In RViz, add an Image display for the sonar image and PointCloud2 for `/sensor/multibeam_sonar/point_cloud`. Raw returns use the acoustic message interface supplied by `marine_acoustic_msgs`; discover their type with `ros2 topic list -t`.

## Sensor configuration

The descriptor uses `type="custom" gz:type="multibeam_sonar"`. The world loads `multibeam_sonar_system`. The BlueView P900 example requests:

| Setting | Value |
| --- | --- |
| Horizontal beams | 512, approximately 130° field of view |
| Vertical rays | 300, approximately 12° field of view |
| Range | 0.1–10 m |
| Frequency / bandwidth | 900 kHz / 29.9 kHz |
| Sound speed | 1500 m/s |
| Sensor update rate | 30 Hz requested |

Actual received rate depends on computation and rendering. Read [the descriptor](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf) for all values.

## Parameters to customize

The `<spec>` block controls `sonarFreq`, `bandwidth`, `soundSpeed`, `sourceLevel`, `maxDistance`, `raySkips`, `sensorGain` and publication names. Keep ray range and processing distance consistent. Match `pointCloudTopicName` to the ROS–Gazebo bridge configuration and set `frameName` for the sonar optical frame.

`writeLog`, `writeFrameInterval` and `debugFlag` support diagnostic recording and timing. Keep logging disabled for throughput measurements unless logging is part of the experiment. See [build and performance](sonar-tuning.md) for a repeatable tuning workflow and [Citation](citation.md) for the sonar model's research source.

## Add a sonar sensor and world

Copy `models/posim_sensor_models/description/blueview_p900/` to a new sensor identifier. Also prepare a matching `config/<sensor identifier>/sensor_config.py`; align SDF model/link/frame names with bridge topics. Set horizontal/vertical samples, angles and range in `<ray>` to match your sensor specification.

Include the Gazebo rendering-sensor system and sonar system in the world. Consult the [sonar world SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world) for physics, scene and other custom-sensor configuration.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="multibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

Prepare CUDA and rendering, rebuild/source the workspace, then select your descriptor using the [sensor launch arguments](quickstart.md). The ROS point-cloud bridge connects `sensor_msgs/msg/PointCloud2` to Gazebo `gz.msgs.PointCloudPacked`. Match `pointCloudTopicName` to the path configured in `sensor_config.py`.

## Record raw sonar data

Enabling `writeLog` creates CSV files in the launch working directory. Set the recording interval with `writeFrameInterval` and monitor disk usage. Use [plotdata.py](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py) to analyze recordings. `sonarImageRawTopicName` names raw acoustic observations; `sonarImageTopicName` names the display image. Record display settings such as `sensorGain` and `blazingSonarImage` with your experiment configuration.

## Sonar scenes and observations

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png"><img width="1051" height="549" src="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png" alt="Point-scattering multibeam sonar processing" loading="lazy" decoding="async"></a><figcaption>Point-scattering multibeam sonar processing</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="RGB, depth, sonar imagery and point cloud from one scene" loading="lazy" decoding="async"></a><figcaption>RGB, depth, sonar imagery and point cloud from one scene</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif"><img width="764" height="472" src="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif" alt="Changing imagery in the sonar sensor example" loading="lazy" decoding="async"></a><figcaption>Changing imagery in the sonar sensor example</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif"><img width="800" height="561" src="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif" alt="Sonar observations and scene changes" loading="lazy" decoding="async"></a><figcaption>Sonar observations and scene changes</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif"><img width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif" alt="Sonar observations from BlueROV2" loading="lazy" decoding="async"></a><figcaption>Sonar observations from BlueROV2</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif"><img width="800" height="367" src="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif" alt="Local scene search with sonar" loading="lazy" decoding="async"></a><figcaption>Local scene search with sonar</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
