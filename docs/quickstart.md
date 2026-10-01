# POSIM Quickstart

Use either the sourced [published candidate container](docker.md) or the
sourced [Ubuntu workspace](installation.md). Do not mix setup files from the
host, another checkout or another ROS distribution into the container shell.
All commands below use retained `dave_*` compatibility names.

## First world

```bash
ros2 launch dave_demos dave_world.launch.py \
  world_name:=dave_ocean_waves headless:=true
```

`dave_world.launch.py` uses `headless:=true` for server-only execution and
starts the world running. For this world, advancing Gazebo clock data is at
`/world/oceans_waves/clock`. See the second-terminal command in the
[Docker guide](docker.md). Stop each launch with Ctrl+C and allow it to finish
before starting the next one.

## REXROV in waves

```bash
ros2 launch dave_demos dave_robot.launch.py \
  namespace:=rexrov world_name:=dave_ocean_waves z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

The runtime checks require the spawned `rexrov` model, advancing simulation
and data on `/model/rexrov/odometry`. Disabling the GUI does not establish
whether joystick input or a browser control session works.

## DVL

```bash
ros2 launch dave_demos dave_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world z:=-30 \
  paused:=false gui:=false headless:=true
```

The tested sensor is `nortek_dvl500_300`; its output includes a Gazebo topic
ending in `dvl/velocity`. Use `gz topic -l` to discover the full topic name,
then `gz topic -e -t <topic>` to inspect actual messages. A topic name alone
does not establish that messages are being published.

## Underwater camera

```bash
ros2 launch dave_demos dave_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

The checked image topic is `/underwater_camera/simulated_image`. These
server-only sensor examples still require a rendering backend. The Docker
Quickstart sets software-rendering/offscreen variables rather than assuming
GPU passthrough. Physical sensor fidelity is not inferred from payload receipt.

## What to check and where to report a failure

Check installed resource resolution, model presence, advancing clock, received
payloads and clean shutdown. On failure, retain the complete launch log,
source revision or image digest, host/container architecture and exact command.
Report issues in [POSIM](https://github.com/IOES-Lab/POSIM/issues), not DAVE.

For the published PR #5 candidate, these four commands are drawn from the
14-path headless matrix. This does not certify an arbitrary `main` build or
all 18 retained world files. Other examples remain in the
[demo guide](../examples/dave_demos/README.md), subject to
[backend limitations](support.md).

## GUI and control sessions are a separate check

A native desktop world can omit `headless:=true`. Robot and sensor launches
also expose `gui`; inspect available arguments with `--show-args` before
enabling it. Configure display access and a working renderer first.
Browser joystick, gamepad, RDP and CUDA/WGPU sonar were not tested by the
published headless matrix and should not be marked supported from that result.
