#!/usr/bin/env bash
# Controlled diagnostic in a disposable existing-image container, not a release.
set -eo pipefail
WS="${POSIM_MAVROS_UNDERLAY:?}"
CHECK=/candidate/extras/ci/mavros_ownership
PATCH=/candidate/extras/patches/mavros-router-parent-lifetime.patch
test "$(cat "$WS/upstream-revision.txt")" = 22ae5b7cc7cdb4cb9c2070a8213c72dae445a23e
printf '%s\n' "$(sha256sum "$PATCH")" > /results/router-patch.sha256
for phase in before after; do
  if [[ "$phase" == after ]]; then
    git -C "$WS/src/mavros" apply --check "$PATCH"
    git -C "$WS/src/mavros" apply "$PATCH"
    (
      cd "$WS"
      export CMAKE_BUILD_PARALLEL_LEVEL=2 MAKEFLAGS=-j2
      colcon build --merge-install --packages-select mavros --executor sequential \
        --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
    ) > /results/router-rebuild.log 2>&1
  fi
  # shellcheck disable=SC1090,SC1091
  source "$WS/install/setup.bash"
  cmake -S "$CHECK" -B "/tmp/probe-$phase" > "/results/$phase-build.log" 2>&1
  cmake --build "/tmp/probe-$phase" --parallel 2 >> "/results/$phase-build.log" 2>&1
  status=0
  timeout 30 "/tmp/probe-$phase/posim_mavros_ownership_check" \
    > "/results/$phase-ownership.log" 2>&1 || status=$?
  printf '%s\n' "$status" > "/results/$phase-ownership.exit"
  if [[ "$phase" == before ]]; then
    # A failing baseline must specifically exhibit both endpoint cycles.
    test "$status" != 0
    grep -q 'mode=0 router_expired=1' /results/before-ownership.log
    grep -q 'mode=1 router_expired=0' /results/before-ownership.log
    grep -q 'mode=2 router_expired=0' /results/before-ownership.log
  else
    test "$status" = 0
    test "$(grep -c 'router_expired=1 all_endpoints_expired=1' /results/after-ownership.log)" = 3
  fi
done
# Same payload/connection/shutdown classifier; no debugger in these trials.
failures=0
cd /tmp
for n in $(seq 1 5); do
  for model in bluerov2 bluerov2_heavy; do
    python3 /checks/image_smoke.py "$model" --record "router-patch-$model-$n" \
      || failures=$((failures + 1))
  done
done
printf 'Candidate-patch diagnostic only: 10 runtime trials, %s failures.\n' "$failures" \
  > /results/diagnostic-outcome.txt
test "$failures" = 0
