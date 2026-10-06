# Slocum glider

The `glider_slocum` description includes a body, thruster, hydrodynamics, IMU, navigation and pressure interfaces. Use it as a starting point for a glider experiment or a custom vehicle description.

## Run in an ocean world

With the environment loaded:

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=glider_slocum world_name:=posim_ocean_waves x:=4 z:=-1.5 \
  paused:=false gui:=false headless:=true \
  use_teleop:=false use_web_joystick:=false
```

For an empty-world composition check, replace the world and position with `world_name:=empty.sdf z:=0.2`. To view the model on a configured desktop, use `gui:=true headless:=false`.

## Inspect observations

```bash
ros2 topic list -t
gz topic -l
```

The descriptor defines IMU and NavSat Gazebo topics under `/model/glider_slocum/`. Inspect the installed bridge configuration to select corresponding ROS topics and types. The model also loads the [pressure plugin](pressure.md) and the Gazebo odometry publisher.

The launch creates a model and environment. Your control node supplies an experiment-specific mission; spawning the model does not start a dive or autonomous gliding sequence.

## Adapt the model

Read the [Slocum SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) alongside its bridge configuration. Change mass, inertia, collision shape and hydrodynamic coefficients together. Verify the frame conventions before connecting an estimator or controller.

Follow [Add a robot](custom-robots.md) to create a separate description, and [ROS 2 and control](ros.md) to record observations and issue commands. Keep the original model as a reference for comparisons.
