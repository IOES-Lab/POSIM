#!/usr/bin/env bash
# Keep the ROS vendor ABI and install the poll fix in a separate ament overlay.
# Never overwrite /opt/ros or treat the churn probe as full-image acceptance.
set -eo pipefail
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
WS="${POSIM_TRANSPORT_UNDERLAY:-/opt/posim_transport_ws}"
VENDOR_REVISION=bc048aec33d25d73e651b86e2858339885dfab87
REVISION=82b10bdff114f77655c7f0cc856835179a674d6d
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PATCH="$SCRIPT_DIR/patches/gz-transport-poll-serialization.patch"
VENDOR_PATCH="$SCRIPT_DIR/patches/gz-transport-vendor-poll.patch"
apt-get update
apt-get install -y --no-install-recommends "ros-$ROS_DISTRO-ament-cmake-vendor-package"
mkdir -p "$WS/src"
git init "$WS/src/gz_transport_vendor"
git -C "$WS/src/gz_transport_vendor" remote add origin \
  https://github.com/gazebo-release/gz_transport_vendor.git
git -C "$WS/src/gz_transport_vendor" -c http.version=HTTP/1.1 fetch --depth 1 origin "$VENDOR_REVISION"
git -C "$WS/src/gz_transport_vendor" checkout --detach FETCH_HEAD
test "$(git -C "$WS/src/gz_transport_vendor" rev-parse HEAD)" = "$VENDOR_REVISION"
git -C "$WS/src/gz_transport_vendor" apply --check "$VENDOR_PATCH"
git -C "$WS/src/gz_transport_vendor" apply "$VENDOR_PATCH"
cp "$PATCH" "$WS/src/gz_transport_vendor/poll-serialization.patch"
# shellcheck disable=SC1090
source "/opt/ros/$ROS_DISTRO/setup.bash"
cd "$WS"
export CMAKE_BUILD_PARALLEL_LEVEL=2 MAKEFLAGS=-j2
colcon build --merge-install --packages-select gz_transport_vendor \
  --executor sequential --cmake-args \
  -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DFORCE_BUILD_VENDOR_PKG=ON \
  -DVENDOR_FROM_LIB_VCS_REF=ON "-DLIB_VCS_REF=$REVISION"
# Match the installed vendor's public configuration (including Zenoh disabled).
PREFIX="$WS/install/opt/gz_transport_vendor"
cmp "/opt/ros/$ROS_DISTRO/opt/gz_transport_vendor/include/gz/transport15/gz/transport/config.hh" \
  "$PREFIX/include/gz/transport15/gz/transport/config.hh"
test "$(git -C "$WS/build/gz_transport_vendor/gz_transport_vendor-prefix/src/gz_transport_vendor" rev-parse HEAD)" = "$REVISION"
printf '%s\n' "$VENDOR_REVISION" > "$WS/vendor-revision.txt"
printf '%s\n' "$REVISION" > "$WS/upstream-revision.txt"
sha256sum "$PATCH" | cut -d ' ' -f 1 > "$WS/poll-patch.sha256"
sha256sum "$VENDOR_PATCH" | cut -d ' ' -f 1 > "$WS/vendor-patch.sha256"
sha256sum "$PREFIX/lib/libgz-transport.so.15" | cut -d ' ' -f 1 > "$WS/library.sha256"
# shellcheck disable=SC1091
source "$WS/install/setup.bash"
cmake -S "$SCRIPT_DIR/ci/transport_shutdown" -B "$WS/probe-build" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$WS/probe-build" --parallel 2
mkdir -p "$WS/probe/bin"
cp "$WS/probe-build/posim_transport_churn" "$WS/probe/bin/"
ldd "$WS/probe/bin/posim_transport_churn" > "$WS/probe-linkage.txt"
grep -F "libgz-transport.so.15 => $PREFIX/lib/libgz-transport.so.15" "$WS/probe-linkage.txt"
python3 "$SCRIPT_DIR/ci/transport_shutdown/run_pairs.py" \
  --executable "$WS/probe/bin/posim_transport_churn" --variant patched: \
  --trials 5 --seconds 20 --output "$WS/build-probe"
