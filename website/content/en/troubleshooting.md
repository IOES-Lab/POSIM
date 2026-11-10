# Troubleshooting

Start with the first error in the launch log. Check environment paths, model resources, simulation time, plugins and data delivery in that order. Keep the first error and the nearby log lines needed to reproduce it.

## Package not found

```bash
printf 'ROS_DISTRO=%s\n' "$ROS_DISTRO"
ros2 pkg prefix posim_demos
```

Expect `lyrical` and the selected workspace path. Follow [terminal setup](terminals.md) for your [installation route](requirements.md), and check host/container paths.

## Missing models or meshes

Read the first missing URI in the log. Check that its package resources were installed and that you sourced the matching workspace. Fuel references need network access from the environment running Gazebo. First-use downloads can take time; follow progress before starting another launch.

## No image or rendering initialization failure

Check graphics drivers and display connections in your [installation guide](requirements.md). Gazebo and image sensors need a working renderer. For sonar, check the [selected backend and installed packages](sonar-tuning.md).

For [underwater scene effects](underwater-rendering.md), check the `/posim/appearance/<session_nonce>/state` service, renderer/shader paths and active camera count. Suspended specks are disabled by default; enable them explicitly. Confirm applied revision and readiness after rendering. The camera attenuation example alone does not load the scene-effects system.

For the documented **macOS arm64 / Ogre-Next 2.3.3** configuration, an [opt-in lifetime overlay helper](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) builds and selects an isolated Ogre core for one process. Follow its exact SDK/source checks and verify loaded-library paths. It is not enabled by the standard installer, and other Ogre versions or platforms require their own matching profile.

## GNSS has no fix

Read the [GNSS validity state](gnss.md), antenna world height, local water height and acquisition timestamp. Confirm world spherical coordinates, an unblocked dry antenna and enough advancing simulation time for reacquisition and latency. If a wave topic is configured, verify fresh compatible local height samples. Receiver installation alone does not attach it to a model or provide a ROS/autopilot adapter.

## No messages

First confirm that time advances, then discover the topic and its type:

```bash
gz topic -l
ros2 topic list -t
ros2 topic info /underwater_camera/simulated_image --verbose
```

Select the correct transport, keep the inspector in the same sourced environment, and check QoS and domain settings. Read bridge and plugin errors. For a sensor launch, `namespace` must name an existing descriptor. [USBL](usbl.md) is already embedded in its tutorial world and uses the world launch.

## Image topics appear but frames stall

If Gazebo produces images and ROS discovers the publisher, but few or no frames arrive, check the DDS data path. With Cyclone DDS, large UDP datagrams can encounter IP fragmentation limits. A smaller datagram size can help on a network with a 1500-byte MTU. See [Cyclone DDS fragmentation settings](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation).

For an environment with `rmw_cyclonedds_cpp` installed, create this profile once. If you already use a Cyclone DDS profile, merge the two size settings into its `General` section so its discovery and interface settings remain in effect.

```bash
mkdir -p "$HOME/.config/posim"
cat > "$HOME/.config/posim/cyclonedds-images.xml" <<'XML'
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain Id="any">
    <General>
      <MaxMessageSize>1400B</MaxMessageSize>
      <FragmentSize>1280B</FragmentSize>
    </General>
  </Domain>
</CycloneDDS>
XML
```

Stop the simulator and viewers. After sourcing POSIM, select the profile in each terminal before restarting them. Use your existing profile's path if you merged the settings there.

```bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI="file://$HOME/.config/posim/cyclonedds-images.xml"
```

If a ROS CLI daemon is already running in this domain, stop it once so subsequent CLI calls use the new configuration:

```bash
ros2 daemon stop
```

Repeat the [camera image and rate checks](camera.md#run-the-example). Check acquisition timestamps as well as reception rate. Smaller packets add overhead; networks with a lower MTU need correspondingly smaller sizes.

## A vehicle does not move

Inspect controller state, arming/navigation validity, command delivery and the selected mode. Confirm joint names and thruster axes. Keep only one active command source. The vehicle may be stationary because input is neutral, the world is paused or the autopilot is disarmed.

## Shutdown fails

Stop data viewers, then press Ctrl+C once in the launch terminal and allow child processes to finish. Preserve abort, segmentation-fault and forced-termination logs. Capture the exact command, revision, OS/architecture and renderer when [reporting an issue](https://github.com/IOES-Lab/POSIM/issues).

### RViz crashes after Discard

On Ubuntu 26.04, RViz 15.2.6 can intermittently crash when you close its window and choose **Discard**, even while its sensor views work. Stop the remaining launch processes with Ctrl+C before starting again. Include the RViz version and the closing action in your issue report.

## Depth-camera recreation on macOS

For a native arm64 SDK using Ogre-Next 2.3.3, the [isolated Ogre overlay](https://github.com/IOES-Lab/POSIM/tree/main/extras/ogre) supplies the material-lifetime correction used during depth-camera destruction and recreation. Follow its build helper and pass your Gazebo command to `run_overlay.py`. Building the overlay alone does not select it for a process. Use a matching SDK and retain the validation receipt; the helper does not replace system libraries or configure Linux containers.

## Make a useful report

Include source SHA or image digest, dependency versions, command/arguments, first error, expected behavior, relevant topic types and a short reproducible case. Note whether the topic only appears in the list or also delivers data.

## MAVROS 2.16.0 exits with SIGSEGV

The POSIM installer and official Docker builds include the
[Router lifetime correction](https://github.com/IOES-Lab/POSIM/tree/main/extras/mavros)
for MAVROS 2.16.0. After loading your POSIM environment, `ros2 pkg prefix mavros`
should print `/opt/posim/mavros-router/install`. Existing installations can use
the separate build instructions at that link. If a crash persists, include the
MAVROS process log and the loaded library paths in your issue report.
