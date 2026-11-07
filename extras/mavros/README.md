# MAVROS Router runtime

The POSIM installer and official AMD64/ARM64 Docker builds install the MAVROS
Router lifetime corrections in `/opt/posim/mavros-router`. The installer loads
this runtime through `~/.ros_ardusub_env/env`. POSIM workspaces built with that
environment also load it when you source their `install/setup.bash`.

The runtime includes [MAVROS #2315](https://github.com/mavlink/mavros/pull/2315)
and the upstream [non-owning-pointer follow-up](https://github.com/mavlink/mavros/commit/2b788a04e69e0aaaef78b9175f816dfcc565b5a0).
The ROS packages in `/opt/ros` stay installed. Vehicle parameters and ROS QoS
retain their existing settings.

Scope: Ubuntu 26.04, ROS 2 Lyrical, MAVROS/libmavconn/mavros_msgs 2.16.0.
The two upstream patches and their attribution are retained unmodified.
The source archive and patches are SHA-256 checked. The base commit is
`5c68b905ab30de6ce630822dc46c33467e8f23ea`.

## Check the active runtime

After loading your POSIM environment, run:

```bash
ros2 pkg prefix mavros
# /opt/posim/mavros-router/install
```

For a running vehicle, inspect `/proc/<mavros_pid>/maps`. Both `libmavros.so`
and `libmavros_plugins.so` should come from this installation.

## Build separately for an existing installation

Use a new writable workspace. Standard ROS development dependencies and the
MAVROS dependencies must already be installed. Install the additional build and
test dependencies through your normal package-management process:

```bash
sudo apt-get install python3-colcon-override-check ros-lyrical-angles \
  ros-lyrical-ament-cmake-google-benchmark ros-lyrical-ament-cmake-gmock \
  ros-lyrical-ament-cmake-pytest ros-lyrical-ament-lint-auto \
  ros-lyrical-ament-lint-common
```

The build helper itself does not run sudo.

```bash
# From the POSIM checkout; existing destinations are deliberately rejected.
bash extras/mavros/build_overlay.sh /path/to/new-mavros-overlay

# In the terminal that will launch POSIM:
source /opt/ros/lyrical/setup.bash
source /path/to/posim_ws/install/setup.bash
source /path/to/new-mavros-overlay/install/local_setup.bash
ros2 pkg prefix mavros
# Must print /path/to/new-mavros-overlay/install, not /opt/ros/lyrical.
```

Keep any existing ArduPilot environment settings required by the POSIM
quickstart. While a vehicle is running, inspect `/proc/<mavros_pid>/maps`:
`libmavros.so` and `libmavros_plugins.so` must resolve to this overlay. Merely
building the helper does not select it. Do not overlay unrelated MAVROS or
libmavconn versions. Rebuild dependent plugins if they use changed Router
internals.

## Version updates

The helper checks the source and patch hashes, builds MAVROS, and runs its
Router tests before completing installation. It stops if any of the three ROS
packages has a different version. Review the fixes in that official release
and its ROS binary packages before updating or removing the backport.

To use the system MAVROS package again, rebuild dependent POSIM workspaces in
a fresh terminal that loads only `/opt/ros/lyrical/setup.bash`. Remove the
overlay source lines from `~/.ros_ardusub_env/env` as part of that change.
Previously built `setup.bash` files retain their recorded parent workspaces.
