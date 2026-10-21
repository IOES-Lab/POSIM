# Troubleshooting

Start with the first error in the launch log. Check environment paths, model resources, simulation time, plugins and data delivery in that order. Keep the full log to help trace the cause.

## Package not found

```bash
printf 'ROS_DISTRO=%s\n' "$ROS_DISTRO"
ros2 pkg prefix posim_demos
```

Expect `lyrical` and the selected workspace path. Source [installation](install.md) or [Docker](docker.md) settings and check host/container paths.

## Missing models or meshes

Read the first missing URI in the log. Check that its package resources were installed and that you sourced the matching workspace. Fuel references need network access from the environment running Gazebo. First-use downloads can take time; follow progress before starting another launch.

## No image or rendering initialization failure

Image sensors need a renderer even without a desktop window. Check display/offscreen settings, [Docker Xvfb setup](docker.md) and [CUDA libraries](sonar-tuning.md).

## No messages

First confirm that time advances, then discover the topic and its type:

```bash
gz topic -l
ros2 topic list -t
ros2 topic info /underwater_camera/simulated_image --verbose
```

Select the correct transport, keep the inspector in the same sourced environment, and check QoS and domain settings. Read bridge and plugin errors. For a sensor launch, `namespace` must name an existing descriptor. [USBL](usbl.md) is already embedded in its tutorial world and uses the world launch.

## A vehicle does not move

Inspect controller state, arming/navigation validity, command delivery and the selected mode. Confirm joint names and thruster axes. Keep only one active command source. The vehicle may be stationary because input is neutral, the world is paused or the autopilot is disarmed.

## Shutdown fails

Stop data viewers, then press Ctrl+C once in the launch terminal and allow child processes to finish. Preserve abort, segmentation-fault and forced-termination logs. Capture the exact command, revision, OS/architecture and renderer when [reporting an issue](https://github.com/IOES-Lab/POSIM/issues).

## Make a useful report

Include source SHA or image digest, dependency versions, command/arguments, first error, expected behavior, relevant topic types and a short reproducible case. Note whether the topic only appears in the list or also delivers data.
