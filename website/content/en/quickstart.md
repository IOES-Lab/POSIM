# First simulation

Explore a shallow underwater inspection scene with BlueROV2, then a buoy course with WAM-V. Each example opens Gazebo and a configured RViz window. Run one simulation at a time.

## Before you start

Complete an installation route in [Choose an environment](requirements.md), then [prepare terminals A and B](terminals.md). Load the same ROS/POSIM workspace in both. Terminal A runs the simulator; Terminal B reads data and starts video. Docker users use the Linux desktop, WSLg or RDP connection configured during installation. These examples use the standard POSIM installation; the surface example does not require the separate surface image.

## Underwater inspection with BlueROV2

In **Terminal A**:

```bash
ros2 launch posim_demos underwater_tutorial.launch.py
```

BlueROV2 starts about 3.1 m below the mean water surface, above a seabed at 4 m, facing a box, pipe, pillar and sphere. Gentle waves, underwater haze, absorption, caustics and marine snow are enabled. The tutorial adjusts ballast to keep the unarmed vehicle near the objects. Gazebo shows the live camera beside the scene.

To add multibeam sonar, append `sonar:=true` to this command. It selects the packaged BlueROV2 Heavy sonar model, adds a sonar image panel in Gazebo, and opens the sonar image and point cloud in RViz. Sonar is resource intensive and **may reduce frame rate and control responsiveness**. Its `auto` backend tries CUDA, then WGPU, then CPU as supported by the host and sensor configuration. The launch keeps normal rendering defaults. See [sonar setup and tuning](sonar-tuning.md) for backend requirements.

The captures below use the sonar-enabled variant. RViz loads the vehicle model, camera and sensor displays automatically. The 1 m grid is a distance reference in the vehicle frame, not a measured seabed. **Move Camera** is selected: drag to orbit, use the middle button to pan and scroll to zoom. Displayed propellers stay at their rest angles; simulation dynamics are unchanged.

<figure><a href="{{ASSET_PREFIX}}media/guides/first-underwater-rviz.png"><img width="1280" height="800" src="{{ASSET_PREFIX}}media/guides/first-underwater-rviz.png" alt="BlueROV2, sonar points and underwater camera in RViz" loading="lazy" decoding="async"></a><figcaption>BlueROV2 and its sonar returns share a vehicle-relative view, alongside the camera and sonar image.</figcaption></figure>

### Check live data

In **Terminal B**:

```bash
ros2 topic echo --no-daemon /model/bluerov2/odometry nav_msgs/msg/Odometry --field pose.pose.position
```

For the sonar variant, replace `bluerov2` in this topic with `bluerov2_heavy_multibeam_sonar`. Position samples should keep arriving, even when the vehicle is nearly stationary. Stop the viewer with **Ctrl+C**, then check that simulation time advances:

```bash
ros2 topic echo --no-daemon /clock rosgraph_msgs/msg/Clock
```

Press Ctrl+C to stop this viewer. If messages are missing or the clock is paused, see [Troubleshooting](troubleshooting.md).

### Watch and control with QGC

Follow steps 1–3 of [QGC setup](qgroundcontrol.md) to open QGroundControl in the same Linux environment, enable its virtual joysticks and select **UDP h.264 Video Stream**, address **127.0.0.1:5600**. Keep **Disable Video Stream When Disarmed** off and continue with the tutorial already running. Start video from Terminal B:

```bash
gz topic -t /tutorial/camera/enable_streaming \
  -m gz.msgs.Boolean -p 'data: true' --duration 3
```

Click the small video panel in QGC to enlarge it. **No GPS Lock** is expected while the marine GNSS antenna is underwater; telemetry, video and Manual control remain available. Wait for **Ready**, use **Manual** mode, arm, make a short joystick movement, release to neutral and disarm. See the [QGC control steps](qgroundcontrol.md) for the controls. Do not start a second keyboard or joystick controller.

<figure><a href="{{ASSET_PREFIX}}media/guides/first-underwater-qgc.png"><img width="1280" height="800" src="{{ASSET_PREFIX}}media/guides/first-underwater-qgc.png" alt="Shaded underwater camera and virtual joysticks in QGroundControl" loading="lazy" decoding="async"></a><figcaption>The underwater camera in QGC. Loss of GNSS fix underwater does not mean loss of the vehicle connection.</figcaption></figure>

## Surface sensing with WAM-V

Stop the underwater session before starting this example. In **Terminal A**:

```bash
ros2 launch posim_demos surface_tutorial.launch.py
```

WAM-V starts on gentle waves facing four anchored buoys. RViz opens its camera and a shared view of the vehicle and lidar point cloud, with a 1 m reference grid and Move Camera selected. The lidar provides a horizontal scan, so its points outline the buoys at the sensor's height rather than forming a full 3D reconstruction.

In Gazebo's **Lidar** panel, click the refresh button and select `/tutorial/lidar`. Turn off **Show Non Hitting Rays** to show only rays that hit objects. The camera panel remains beside the scene.

<figure><a href="{{ASSET_PREFIX}}media/guides/first-surface-rviz.png"><img width="1280" height="800" src="{{ASSET_PREFIX}}media/guides/first-surface-rviz.png" alt="WAM-V, buoy lidar returns and forward camera in RViz" loading="lazy" decoding="async"></a><figcaption>WAM-V and lidar returns in the same view, with the forward camera beside them.</figcaption></figure>

The camera uses the same `/tutorial/camera` topic and video-start command as the underwater example. Keep the same QGC video settings. This vehicle uses ArduRover boat mode and simulated pose/velocity for external navigation; see [Surface robots and waves](surface.md). Its navigation input is separate from BlueROV2's marine GNSS receiver.

In QGC Manual mode, WAM-V uses the left stick vertically for throttle. Arm, make a short movement, release the stick to neutral and disarm.

<figure><a href="{{ASSET_PREFIX}}media/guides/first-surface-qgc.png"><img width="1280" height="800" src="{{ASSET_PREFIX}}media/guides/first-surface-qgc.png" alt="WAM-V buoy-course camera in QGroundControl" loading="lazy" decoding="async"></a><figcaption>WAM-V's forward camera streams the buoy course to QGC.</figcaption></figure>

To read one lidar scan in **Terminal B**:

```bash
ros2 topic echo --no-daemon --once /tutorial/lidar sensor_msgs/msg/LaserScan
```

The point cloud is `/tutorial/lidar/points`; vehicle odometry is `/model/wamv/odometry`.

## Stop the session

Disarm the vehicle, close RViz and QGC, then press **Ctrl+C in Terminal A** and wait for the launch processes to exit. If RViz asks to save view changes, discard them to keep the supplied preset. If it fails to exit cleanly, see [viewer troubleshooting](troubleshooting.md). Closing a topic viewer does not stop the simulation. Stop the current session before running another example.

## Continue learning

- [View & record sensor data](visualization.md): inspect images and save measurements.
- [Control with Python](python-control.md): launch the separate REXROV example and exercise one thruster.
- [ROVs & BlueROV2](rovs.md): use ArduSub, keyboard controls or the local joystick.
- [World library](worlds.md): choose an environment and inspect launch arguments.
