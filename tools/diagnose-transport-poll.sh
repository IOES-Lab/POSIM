#!/usr/bin/env bash
# Disposable-container diagnostic. Never installs into the release image.
set -eo pipefail
REVISION=82b10bdff114f77655c7f0cc856835179a674d6d
WORK=/tmp/posim-transport-comparison
PATCH=/candidate/extras/patches/gz-transport-poll-serialization.patch
mkdir -p "$WORK" /results/transport-poll
RESULTS=/results/transport-poll
printf '%s\n' "$REVISION" > "$RESULTS/upstream-revision.txt"
sha256sum "$PATCH" > "$RESULTS/patch.sha256"
dpkg-query -W libzmq5 ros-lyrical-gz-transport-vendor > "$RESULTS/packages.txt"
# shellcheck disable=SC1091
source /opt/ros/lyrical/setup.bash
cmake -S /candidate/extras/ci/transport_shutdown -B "$WORK/probe" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo > "$RESULTS/probe-build.log" 2>&1
cmake --build "$WORK/probe" --parallel 2 >> "$RESULTS/probe-build.log" 2>&1
sha256sum "$WORK/probe/posim_transport_churn" > "$RESULTS/probe.sha256"
git init "$WORK/source" > "$RESULTS/source-fetch.log" 2>&1
git -C "$WORK/source" remote add origin https://github.com/gazebosim/gz-transport.git
git -C "$WORK/source" -c http.version=HTTP/1.1 fetch --depth 1 origin "$REVISION" \
  >> "$RESULTS/source-fetch.log" 2>&1
git -C "$WORK/source" checkout --detach FETCH_HEAD >> "$RESULTS/source-fetch.log" 2>&1
test "$(git -C "$WORK/source" rev-parse HEAD)" = "$REVISION"
for VARIANT in baseline patched; do
  cp -a "$WORK/source" "$WORK/$VARIANT-src"
  if [[ "$VARIANT" == patched ]]; then
    git -C "$WORK/$VARIANT-src" apply --check "$PATCH"
    git -C "$WORK/$VARIANT-src" apply "$PATCH"
  fi
  cmake -S "$WORK/$VARIANT-src" -B "$WORK/$VARIANT-build" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DSKIP_PYBIND11=ON \
    > "$RESULTS/$VARIANT-build.log" 2>&1
  cmake --build "$WORK/$VARIANT-build" --target gz-transport --parallel 2 \
    >> "$RESULTS/$VARIANT-build.log" 2>&1
done
python3 /checks/transport_shutdown/run_pairs.py \
  --executable "$WORK/probe/posim_transport_churn" \
  --variant "baseline:$WORK/baseline-build/lib" \
  --variant "patched:$WORK/patched-build/lib" \
  --trials 5 --seconds 20 --output "$RESULTS/pairs"
# The driver retains all outcomes. A successful collection is not release acceptance.
printf 'Transport diagnostic completed; inspect transport-poll/pairs/summary.json. Not release acceptance.\n' \
  > /results/diagnostic-outcome.txt
