#!/usr/bin/env bash
# Run each scene in a fresh container; never mount a host source/install overlay.
set -euo pipefail
IMAGE="${1:?image tag or ID required}"
PLATFORM="${2:?linux/arm64 or linux/amd64 required}"
RESULTS="${3:?results directory required}"
CHECKS="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$RESULTS"
RESULTS="$(cd "$RESULTS" && pwd)"
chmod 777 "$RESULTS"
docker image inspect "$IMAGE" > "$RESULTS/image-inspect.json"
IMAGE_ID="$(docker image inspect --format '{{.Id}}' "$IMAGE")"
EXPECTED_ARCH="${PLATFORM#linux/}"
EXPECTED_ARCH="${EXPECTED_ARCH%%/*}"
test "$(docker image inspect --format '{{.Architecture}}' "$IMAGE_ID")" = "$EXPECTED_ARCH"
printf '%s\n' "$IMAGE_ID" > "$RESULTS/tested-image-id.txt"
printf 'case,status\n' > "$RESULTS/summary.csv"
FAILED=0
CURRENT_CONTAINER=""
cleanup() {
  if [[ -n "$CURRENT_CONTAINER" ]]; then
    docker rm -f "$CURRENT_CONTAINER" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT
trap 'exit 130' INT TERM
CASES="inventory $(awk -F '\t' '!/^#/ && NF {print $1}' "$CHECKS/quickstarts.tsv")"
# Ten trials per previously failing lifecycle case, counting the matrix run.
# Every trial is retained; a later pass never cancels an earlier failure.
for REPEAT in $(seq 2 10); do
  for CASE in spherical_world camera rexrov_waves ocean_current sea_pressure bluerov2 bluerov2_heavy; do
    CASES="$CASES ${CASE}:r${REPEAT}"
  done
done
for RECORD in $CASES; do
  CASE="${RECORD%%:*}"
  echo "=== POSIM image validation: $CASE ==="
  RECORD="${RECORD//:/-}"
  CURRENT_CONTAINER="posim-smoke-${GITHUB_RUN_ID:-$$}-${GITHUB_RUN_ATTEMPT:-1}-$RECORD"
  if docker run --rm --init --name "$CURRENT_CONTAINER" --platform "$PLATFORM" --shm-size=1g \
    --entrypoint bash -e ROS_DOMAIN_ID=121 -e GZ_IP=127.0.0.1 \
    -e LIBGL_ALWAYS_SOFTWARE=1 -e QT_QPA_PLATFORM=offscreen \
    -e "CASE=$CASE" -e "RECORD=$RECORD" \
    -v "$CHECKS:/checks:ro" -v "$CHECKS/../patches:/patches:ro" -v "$RESULTS:/results" \
    "$IMAGE_ID" -c '
      set -eo pipefail
      source /opt/ros/lyrical/setup.bash
      if [[ -n "${DAVE_WS:-}" ]]; then
        export POSIM_WORKSPACE="$DAVE_WS"
      else
        export POSIM_WORKSPACE="${DAVE_UNDERLAY:?installed workspace missing}"
      fi
      source "$POSIM_WORKSPACE/install/setup.bash"
      cd /tmp
      exec python3 /checks/image_smoke.py "$CASE" --record "$RECORD"
    '; then
    printf '%s,PASS\n' "$RECORD" >> "$RESULTS/summary.csv"
  else
    printf '%s,FAIL\n' "$RECORD" >> "$RESULTS/summary.csv"
    FAILED=1
  fi
done
cat "$RESULTS/summary.csv"
cleanup
trap - EXIT
exit "$FAILED"
