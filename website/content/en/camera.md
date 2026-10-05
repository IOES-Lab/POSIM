# Underwater camera

The underwater-camera plugin applies distance-dependent color attenuation to rendered RGB data using depth information. Tune the attenuation and background color to the experiment's intended water appearance.

## Run the example

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

In a second sourced terminal:

```bash
ros2 topic echo /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
ros2 topic hz /underwater_camera/simulated_image
```

Use an RViz Image display to inspect the resulting image. A working renderer is needed even when no GUI is shown.

## Configure the plugin

The model uses an RGBD sensor and the plugin `UnderwaterCamera`, class `posim_gz_sensor_plugins::UnderwaterCamera`. Keep its sensor/data topics consistent with the model configuration.

| SDF parameter | Meaning | Code default |
| --- | --- | --- |
| `attenuationR` | Red-channel attenuation per meter | `1/30` |
| `attenuationG` | Green-channel attenuation per meter | `1/30` |
| `attenuationB` | Blue-channel attenuation per meter | `1/30` |
| `backgroundR` | Red background channel | `0` |
| `backgroundG` | Green background channel | `0` |
| `backgroundB` | Blue background channel | `0` |

The packaged example overrides these with attenuation `0.8`, `0.5`, `0.2` and background `85`, `107`, `47`. Its RGBD image is 320 × 240 with a requested 10 Hz update rate. Code defaults and descriptor values are different; inspect the chosen SDF when reproducing an image.

## Tune and compare

Place a recognizable object at known distances. Keep pose, lighting, scene materials and camera settings fixed while changing one attenuation channel. Verify image timestamps and received rate in addition to the visual result.

Camera-specific attenuation changes the sensor image, not the entire Gazebo desktop view. To change the world view, adjust world lighting/materials separately. Source implementation and the complete [camera descriptor](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) define the input wiring and output names.
