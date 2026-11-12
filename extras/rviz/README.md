# RViz close and ownership corrections

The Ubuntu/WSL installer and AMD64/ARM64 Dockerfiles build an isolated
`rviz_common` overlay at `/opt/posim/rviz-close`. It contains two local corrections:

- Stop the visualization update timer after a close is accepted. Cancel leaves
  updates running.
- Detach and destroy the owned `ViewManager` before its display context is
  destroyed. This releases view-controller ROS services and their DDS participant
  resources.

The overlay leaves ROS package files in `/opt/ros` unchanged. These corrections
address specific timer and ownership contracts; they do not establish that every
graphics driver or shutdown path is free of errors.

## Source and compatibility

The builder requires Ubuntu 26.04 and ROS 2 Lyrical, with `rviz_common`,
`rviz_rendering`, `rviz_default_plugins` and `rviz2` all at version **15.2.6**.
Only `rviz_common` is rebuilt. The other RViz packages remain in the ROS underlay.

- Source: [RViz 15.2.6](https://github.com/ros2/rviz/tree/baab61a68bc089217dfaa4f270276dc7a30268b1).
- Commit: `baab61a68bc089217dfaa4f270276dc7a30268b1`.
- Archive: `https://codeload.github.com/ros2/rviz/tar.gz/baab61a68bc089217dfaa4f270276dc7a30268b1`.
- Archive SHA-256: `ca0ed4dc39bf1652db788baa2ba24ac7d3ff23b1b925653a25806de39b2708cf`.

The builder verifies the archive checksum and checks that both patches apply.
It retains the upstream source and license notices in the overlay workspace,
along with `source.tar.gz` and `PROVENANCE.txt`.

## Build and select the overlay

Start from an existing ROS 2 Lyrical development installation. Install any missing
build and private-display test dependencies through your package manager:

```bash
sudo apt-get install build-essential cmake curl git \
  python3-colcon-common-extensions python3-colcon-override-check \
  ros-lyrical-rviz2 xvfb xauth
```

The helper uses `tar`, `sha256sum` and `timeout` from the standard system tools.
It does not run sudo. From a POSIM checkout, choose a new writable destination:

```bash
bash extras/rviz/build_overlay.sh /path/to/new-rviz-overlay

# In the terminal that will launch POSIM:
source /opt/ros/lyrical/setup.bash
source /path/to/posim_ws/install/setup.bash
source /path/to/new-rviz-overlay/install/local_setup.bash
ros2 pkg prefix rviz_common
# Expected: /path/to/new-rviz-overlay/install
```

Keep the existing ArduPilot and MAVROS environment settings. Building an overlay
does not select it in other terminals. For a running RViz process, inspect
`/proc/<rviz_pid>/maps`: `librviz_common.so` must come from the selected overlay.

## Regression tests

The builder runs the tests before completing. To check an already selected
environment, including a downstream POSIM workspace's recorded underlays, run:

```bash
bash extras/rviz/test_overlay.sh /path/to/new-rviz-overlay/install /tmp/rviz-close-tests
```

Tests use a private Xvfb display and settings directory. They exercise Discard,
Cancel followed by Discard, Save and unchanged close with live Image displays.
They check the loaded library, timer behavior, continued image reception after
Cancel, destruction of the view manager and ROS nodes, and release of DDS
participant threads. FastDDS's process-global shared-memory watchdog is allowed
to remain until library finalization. Logs and live library maps are retained in
the test directory. Full tutorial and graphics-platform qualification is separate.

Automatic ROS discovery is restricted to the local host. Tests use domain `87`,
independently of the caller's `ROS_DOMAIN_ID`. Set `POSIM_RVIZ_TEST_DOMAIN_ID` to
another domain from `0` to `232` if needed. Each process uses a UUID-based node
name and image topic to keep concurrent fixture streams separate.

## Update or remove the corrections

The builder rejects an existing destination or an unreviewed RViz version.
Before changing the pin, compare both corrections with the official source and
ROS binary packages, update the checksum, and rerun the regression and tutorial
checks on each supported platform. Remove a patch when its correction is present
in the official packages and the corresponding tests pass without the overlay.

To return to the system RViz package, remove this overlay's source lines from the
installation environment and rebuild dependent workspaces in a fresh terminal
that does not load it. Existing `setup.bash` files retain their recorded parent
workspaces. Keep other required POSIM overlays enabled.
