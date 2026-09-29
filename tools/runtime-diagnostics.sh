#!/usr/bin/env bash
# Diagnostic only: same installed image, with GDB and an instrumented launch.
# A green workflow means evidence was collected, not that runtime tests passed.
set -euo pipefail
IMAGE_ID="${1:?image ID required}"
ARCH="${2:?architecture required}"
RESULTS="${3:?results directory required}"
[[ "$IMAGE_ID" =~ ^sha256:[0-9a-f]{64}$ ]]
[[ "$ARCH" == arm64 || "$ARCH" == amd64 ]]
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$RESULTS"
RESULTS="$(cd "$RESULTS" && pwd)"
chmod 777 "$RESULTS"
docker image inspect "$IMAGE_ID" > "$RESULTS/base-image-inspect.json"
test "$(docker image inspect --format '{{.Architecture}}' "$IMAGE_ID")" = "$ARCH"
if [[ "${POSIM_DIAGNOSTIC_MODE:-}" == gazebo-fresh ]]; then
  for n in $(seq 1 20); do
    POSIM_DIAGNOSTIC_MODE=gazebo-shutdown POSIM_DIAGNOSTIC_TRIALS=1 \
      bash "$0" "$IMAGE_ID" "$ARCH" "$RESULTS/fresh-$n"
    if grep -q '"pid"' "$RESULTS/fresh-$n/gazebo-stack-captures.json"; then
      printf 'Slow-shutdown evidence captured in fresh container %s; diagnostic only.\n' "$n" \
        > "$RESULTS/diagnostic-outcome.txt"
      exit 0
    fi
  done
  printf 'No slow-shutdown stack in 20 fresh containers; earlier failures remain unresolved.\n' \
    > "$RESULTS/diagnostic-outcome.txt"
  exit 0
fi
CONTAINER="posim-diagnose-${GITHUB_RUN_ID:-$$}"
trap 'docker rm -f "$CONTAINER" >/dev/null 2>&1 || true' EXIT
docker run --rm --init --name "$CONTAINER" --platform "linux/$ARCH" \
  --user root --shm-size=1g --cap-add SYS_PTRACE --security-opt seccomp=unconfined \
  --entrypoint bash -e ROS_DOMAIN_ID=123 -e GZ_IP=127.0.0.1 \
  -e POSIM_DIAGNOSTIC_TRIALS="${POSIM_DIAGNOSTIC_TRIALS:-20}" \
  -e POSIM_DIAGNOSTIC_MODE="${POSIM_DIAGNOSTIC_MODE:-gdb}" \
  -v "$ROOT:/candidate:ro" \
  -e LIBGL_ALWAYS_SOFTWARE=1 -e QT_QPA_PLATFORM=offscreen \
  -v "$ROOT/extras/ci:/checks:ro" -v "$ROOT/tools:/diagnostics:ro" -v "$RESULTS:/results" "$IMAGE_ID" -c '
    set -eo pipefail
    dpkg-query -W > /results/packages-before.tsv
    apt-get update > /results/debugger-install.log 2>&1
    apt-get install -y --no-install-recommends gdb >> /results/debugger-install.log 2>&1
    dpkg-query -W > /results/packages-after.tsv
    source /opt/ros/lyrical/setup.bash
    source "${DAVE_WS:-$DAVE_UNDERLAY}/install/setup.bash"
    if [[ "$POSIM_DIAGNOSTIC_MODE" == router-lifetime ]]; then
      exec bash /diagnostics/diagnose-router-lifetime.sh
    fi
    if [[ "$POSIM_DIAGNOSTIC_MODE" == gazebo-shutdown ]]; then
      exec python3 /diagnostics/diagnose-gazebo-shutdown.py
    fi
    python3 /diagnostics/instrument-mavros-gdb.py
    cd /tmp
    for n in $(seq 1 20); do
      record="mavros-gdb-$n"
      python3 /checks/image_smoke.py bluerov2_heavy --record "$record" || true
      if grep -Eq "received signal SIGSEGV|Program terminated with signal SIGSEGV" \
        "/results/$record/launch.log"; then
        printf "Captured SIGSEGV in %s; diagnostic run, not acceptance.\n" "$record" \
          > /results/diagnostic-outcome.txt
        exit 0
      fi
    done
    printf "No GDB-captured SIGSEGV in 20 trials; this does not establish absence.\n" \
      > /results/diagnostic-outcome.txt
  '
