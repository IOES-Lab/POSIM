# Optional MAVROS 2.16.0 Router lifetime overlay

This profile backports the accepted Router ownership correction from
[mavros #2315](https://github.com/mavlink/mavros/pull/2315) and the upstream
non-owning-pointer follow-up. It does not update MAVROS to the moving `ros2`
branch or change `/opt/ros`, startup files, POSIM vehicle parameters or ROS QoS.

Scope: Ubuntu 26.04, ROS 2 Lyrical, installed MAVROS/libmavconn 2.16.0.
The two upstream patches and their attribution are retained unmodified.
The source archive and patches are SHA-256 checked. The base commit is
`5c68b905ab30de6ce630822dc46c33467e8f23ea`.

## Build and select explicitly

Use a new writable workspace. Standard ROS development dependencies and the
MAVROS dependencies must already be installed. The test build additionally
requires `ros-lyrical-ament-cmake-google-benchmark`; install this through your
normal package-management process if missing. The helper does not run sudo.

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

To roll back, start a fresh terminal and source only the original ROS/POSIM
setup. No installed library has been replaced. The standard POSIM installer
**does not automatically enable this optional overlay**. Vehicle connectivity,
control and shutdown must be checked in the intended release image before it
is distributed. A finite repetition test is not a guarantee of zero failures.
