#!/usr/bin/env bash
# Build the Lyrical bridge with the upstream ownership-cycle fix. The published
# 3.0.10 Debian package predates this one-commit backport; do not patch /opt/ros.
set -eo pipefail
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
WS="${POSIM_BRIDGE_UNDERLAY:-/opt/posim_bridge_ws}"
REVISION=54a2e78a41c623173608cdd8eef2e049ee3ee3b0
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PATCH="$SCRIPT_DIR/patches/ros-gz-bridge-callback-lifetime.patch"
mkdir -p "$WS/src"
git init "$WS/src/ros_gz"
git -C "$WS/src/ros_gz" remote add origin https://github.com/gazebosim/ros_gz.git
git -C "$WS/src/ros_gz" -c http.version=HTTP/1.1 fetch --depth 1 origin "$REVISION"
git -C "$WS/src/ros_gz" checkout --detach FETCH_HEAD
test "$(git -C "$WS/src/ros_gz" rev-parse HEAD)" = "$REVISION"
# The upstream handle fix does not remove the ROS subscription callback's
# shared node capture. Keep the additional patch explicit and hash-recorded.
git -C "$WS/src/ros_gz" apply --check "$PATCH"
git -C "$WS/src/ros_gz" apply "$PATCH"
# shellcheck disable=SC1090
source "/opt/ros/$ROS_DISTRO/setup.bash"
cd "$WS"
# All build dependencies are supplied by the installed ros_gz packages.
export CMAKE_BUILD_PARALLEL_LEVEL=2 MAKEFLAGS=-j2
colcon build --merge-install --packages-select ros_gz_bridge \
  --executor sequential --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
printf '%s\n' "$REVISION" > "$WS/upstream-revision.txt"
sha256sum "$PATCH" | cut -d ' ' -f 1 > "$WS/callback-patch.sha256"
# shellcheck disable=SC1091
source "$WS/install/setup.bash"
cmake -S "$SCRIPT_DIR/ci/bridge_ownership" -B "$WS/probe-build" \
  -DCMAKE_INSTALL_PREFIX="$WS/probe"
cmake --build "$WS/probe-build" --parallel 2
cmake --install "$WS/probe-build"
"$WS/probe/bin/posim_bridge_ownership_check"
