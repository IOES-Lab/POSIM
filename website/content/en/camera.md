# Underwater camera

The `UnderwaterCamera` plugin uses depth data to reduce RGB color intensity with distance. Adjust attenuation and background color to match the water conditions you want to simulate. For scene-wide haze, absorption, caustics, suspended specks (marine snow), seabed grids and live controls, use the public [underwater scene renderer](underwater-rendering.md).

## Run the example

This sensor example uses the depth-based camera filter. The scene renderer has its own [build, world plugin and appearance settings](underwater-rendering.md).

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

In a second terminal with the same environment loaded:

```bash
ros2 topic echo /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
ros2 topic hz /underwater_camera/simulated_image
```

Use an RViz Image display to inspect the resulting image. A working renderer is needed even when no GUI is shown. If the topic is discovered but images stop arriving, check [large-image transport](troubleshooting.md#image-topics-appear-but-frames-stall).

## Configure the plugin

The model uses an RGBD sensor and the plugin `UnderwaterCamera`, class `posim_gz_sensor_plugins::UnderwaterCamera`. Keep its sensor/data topics consistent with the model configuration.

Place this sensor inside a model `<link>`. The world also needs `gz-sim-sensors-system` with `<render_engine>ogre2</render_engine>`, as in [the camera world](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world). The plugin reads Gazebo RGB/depth images and directly publishes the ROS output `/underwater_camera/simulated_image`.

```xml
<sensor name="underwater_camera" type="rgbd_camera">
  <update_rate>10</update_rate>
  <topic>underwater_camera</topic>
  <camera>
    <horizontal_fov>1.05</horizontal_fov>
    <image><width>320</width><height>240</height></image>
    <clip><near>0.1</near><far>10.0</far></clip>
  </camera>
  <plugin filename="UnderwaterCamera"
          name="posim_gz_sensor_plugins::UnderwaterCamera">
    <attenuationR>0.8</attenuationR>
    <attenuationG>0.5</attenuationG>
    <attenuationB>0.2</attenuationB>
    <backgroundR>85</backgroundR>
    <backgroundG>107</backgroundG>
    <backgroundB>47</backgroundB>
  </plugin>
</sensor>
```

| SDF parameter | Meaning | Code default |
| --- | --- | --- |
| `attenuationR` | Red-channel attenuation per meter, finite and ≥ 0 | `1/30` |
| `attenuationG` | Green-channel attenuation per meter, finite and ≥ 0 | `1/30` |
| `attenuationB` | Blue-channel attenuation per meter, finite and ≥ 0 | `1/30` |
| `backgroundR` | Red background channel, integer 0–255 | `0` |
| `backgroundG` | Green background channel, integer 0–255 | `0` |
| `backgroundB` | Blue background channel, integer 0–255 | `0` |

The sensor's `<topic>` must produce a valid ROS topic name. Omitting it uses Gazebo's `/world/<world>/model/<model>/link/<link>/sensor/<sensor>` scope; the ROS image is published below that scope at `/simulated_image`. Nested models add their parent model scopes.

Depth-to-range conversion uses an explicit camera lens projection when supplied, then explicit lens intrinsics, otherwise image dimensions and horizontal field of view. Explicit zero principal-point coordinates are valid. Focal lengths must be positive, lens values finite and clipping distances satisfy `0 < near < far` in meters. Invalid settings produce a log error and disable the plugin's output.

Color and depth frames are paired by their acquisition timestamp, whichever arrives first. Each ROS image preserves the color frame's timestamp and frame ID. Unmatched frames are discarded when the pending buffer fills; resetting simulation time clears pending frames and allows timestamps to restart.

The example model uses these values. SDF settings override code defaults.

- RGB attenuation: **0.8, 0.5, 0.2**
- RGB background: **85, 107, 47**
- Image size: **320 × 240**
- Requested rate: **10 Hz**

## Tune and compare

Place a recognizable object at known distances. Keep pose, lighting, scene materials and camera settings fixed while changing one attenuation channel. Verify image timestamps and received rate in addition to the visual result.

Camera-specific attenuation changes the sensor image. Use the [scene renderer](underwater-rendering.md) to apply shared underwater effects to PBR surfaces in scene and camera views. When that native system is active, `POSIM_UNDERWATER_PLUGIN` enables pass-through in this compatibility output to avoid filtering the same pixels twice. The [camera model](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) defines input wiring and output names.

## Configure RGBD sensing and water appearance

The RGBD sensor supplies color and depth images. The plugin uses depth-derived range, per-channel attenuation and background values to produce underwater imagery. Attenuation follows a per-channel exponential model; background channels range from 0 to 255.

For an unattenuated comparison, explicitly set `attenuationR`, `attenuationG`, `attenuationB` and all three background values to zero. For murky coastal water, use the example values 0.8/0.5/0.2 and 85/107/47 above. Omitting tags uses the code defaults.

This sensor implements distance-dependent RGB attenuation and background blending. The [scene renderer](underwater-rendering.md) supplies visual sunlight, caustics and optional suspended specks. Studies of calibrated particle scattering need a suitable optical model beyond these visual approximations.

## Suspended particles

The native renderer can show suspended specks in RGB and RGBD color images. Specks are off by default. Enable them through the host application's appearance controls or the renderer's [appearance service](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering).

Particles occupy shared positions in the world and drift uniformly with simulation time. Different camera views observe the same positions at the same time. Opaque surfaces hide particles behind them. The visible interval is 0.05–3 m, clipped by the scene and the mean water level. Numeric depth and acoustic rays retain their values.

This is a procedural optical effect. Its drift is independent of simulated currents. When evaluating visual navigation, treat suspended particles as moving observations and keep appearance settings fixed across comparisons.

## Compare underwater images

<figure><a href="{{ASSET_PREFIX}}media/notion/camera-200c9419.png"><img width="2848" height="1726" src="{{ASSET_PREFIX}}media/notion/camera-200c9419.png" alt="Gazebo underwater-camera scene" loading="lazy" decoding="async"></a><figcaption>Gazebo underwater-camera scene</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-effc9419.png"><img width="1038" height="182" src="{{ASSET_PREFIX}}media/notion/camera-effc9419.png" alt="Underwater-camera image display" loading="lazy" decoding="async"></a><figcaption>Underwater-camera image display</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="Image with zero RGB attenuation and zero background" loading="lazy" decoding="async"></a><figcaption>Image with zero RGB attenuation and zero background</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="Murky coastal image: RGB attenuation 0.8/0.5/0.2 and background 85/107/47" loading="lazy" decoding="async"></a><figcaption>Murky coastal image: RGB attenuation 0.8/0.5/0.2 and background 85/107/47</figcaption></figure>

Figures and videos: POSIM Notion Wiki. See [Citation and licenses](citation.md) for DAVE documentation attribution. Use this page's code blocks for execution commands and topic names.
