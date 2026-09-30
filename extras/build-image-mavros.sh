#!/usr/bin/env bash
# Keep MAVROS 2.15.1, backport upstream I/O self-close, and break the Router cycle.
set -eo pipefail
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
WS="${POSIM_MAVROS_UNDERLAY:-/opt/posim_mavros_ws}"
REVISION=22ae5b7cc7cdb4cb9c2070a8213c72dae445a23e
FIX=3a1f39f1a033d39d9e7c34d9ba7cb28cd3dbcd5f
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PATCH="$SCRIPT_DIR/patches/mavconn-self-close-lifetime.patch"
ROUTER_PATCH="$SCRIPT_DIR/patches/mavros-router-parent-lifetime.patch"
mkdir -p "$WS/src"
git init "$WS/src/mavros"
git -C "$WS/src/mavros" remote add origin https://github.com/mavlink/mavros.git
git -C "$WS/src/mavros" -c http.version=HTTP/1.1 fetch --depth 1 origin "$REVISION" "$FIX"
git -C "$WS/src/mavros" checkout --detach "$REVISION"
test "$(git -C "$WS/src/mavros" rev-parse HEAD)" = "$REVISION"
git -C "$WS/src/mavros" apply --check "$PATCH"
git -C "$WS/src/mavros" apply "$PATCH"
# Ensure the backported header is byte-identical to the upstream fixed header.
git -C "$WS/src/mavros" show "$FIX:libmavconn/include/mavconn/io_context_runner.hpp" \
  > "$WS/upstream-fixed-header.hpp"
cmp "$WS/upstream-fixed-header.hpp" \
  "$WS/src/mavros/libmavconn/include/mavconn/io_context_runner.hpp"
# Local fix, distinct from the upstream I/O patch: endpoints must not own Router.
git -C "$WS/src/mavros" apply --check "$ROUTER_PATCH"
git -C "$WS/src/mavros" apply "$ROUTER_PATCH"
# shellcheck disable=SC1090
source "/opt/ros/$ROS_DISTRO/setup.bash"
# Keep the matching Gazebo Transport overlay in the workspace setup chain.
# shellcheck disable=SC1090,SC1091
source "${POSIM_TRANSPORT_UNDERLAY:?Transport underlay required}/install/setup.bash"
cd "$WS"
export CMAKE_BUILD_PARALLEL_LEVEL=2 MAKEFLAGS=-j2
# Router also includes IoContextRunner; rebuild MAVROS, not only libmavconn.
colcon build --merge-install --packages-select libmavconn mavros \
  --executor sequential --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
printf '%s\n' "$REVISION" > "$WS/upstream-revision.txt"
printf '%s\n' "$FIX" > "$WS/upstream-fix.txt"
sha256sum "$PATCH" | cut -d ' ' -f 1 > "$WS/self-close-patch.sha256"
sha256sum "$ROUTER_PATCH" | cut -d ' ' -f 1 > "$WS/router-parent-patch.sha256"
mkdir -p "$WS/probe/bin"
c++ -std=c++20 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
  -pthread -I"$WS/install/include" "$SCRIPT_DIR/ci/mavconn_self_close/main.cc" \
  -o "$WS/probe/bin/posim_mavconn_self_close_check"
"$WS/probe/bin/posim_mavconn_self_close_check"
# shellcheck disable=SC1090,SC1091
source "$WS/install/setup.bash"
cmake -S "$SCRIPT_DIR/ci/mavros_ownership" -B "$WS/ownership-probe-build" \
  -DCMAKE_INSTALL_PREFIX="$WS/probe"
cmake --build "$WS/ownership-probe-build" --parallel 2
cmake --install "$WS/ownership-probe-build"
"$WS/probe/bin/posim_mavros_ownership_check"
