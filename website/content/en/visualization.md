# View & record sensor data

Start one sensor, confirm that fresh measurements arrive, then open a viewer or record a short run. The simulator stays running in Terminal A while these tools run in Terminal B.

## Receive a camera image

Follow [Prepare your terminals](terminals.md) to open Terminal A and Terminal B in the same environment. Stop the previous simulation in A with Ctrl+C, then start this camera scene in A. Gazebo opens in the desktop prepared by your installation guide.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=true headless:=false
```

In Terminal B:

```bash
ros2 topic echo --no-daemon /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
```

Check nonzero width and height, encoding and timestamp. `--no-arr` hides the pixel array from terminal output. To see the image, use an RViz Image display subscribed to this topic. [Underwater camera](camera.md) explains the attenuation settings.

## Visualization and recording

Choose RViz displays matching the message type.

- Cameras: Image
- Point clouds: PointCloud2
- Frames and state: TF/pose displays

POSIM-specific messages are defined in `posim_interfaces`. Install and source these definitions on each separate ROS client.

Find sensor topics and message types in the [camera](camera.md), [pressure](pressure.md), [DVL](dvl.md), [USBL](usbl.md) and [sonar](sonar.md) references. [Ocean currents](currents.md) and [spherical coordinates](coordinates.md) also expose ROS services for changing experiment inputs.

```bash
ros2 bag record -o camera_run --topics /underwater_camera/simulated_image /clock
```

Press Ctrl+C to finish recording. Choose a new output directory for each run. Inspect the saved topic counts:

```bash
ros2 bag info camera_run
```

Stop the simulator before replaying the observations on the same topics:

```bash
ros2 bag play camera_run
```
