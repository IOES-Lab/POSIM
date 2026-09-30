#!/usr/bin/env bash
# Fresh containers from the registry-resolved image, not the source checkout.
set -euo pipefail
REF="${1:?repository@digest required}"
ARCH="${2:?architecture required}"
IMAGE_ID="${3:?prevalidated image ID required}"
RESULTS="${4:?results directory required}"
CONFIG_ID="${5:?registry-verified configuration digest required}"
[[ "$REF" =~ ^ioeslab/posim@sha256:[0-9a-f]{64}$ ]]
[[ "$IMAGE_ID" =~ ^sha256:[0-9a-f]{64}$ ]]
[[ "$CONFIG_ID" =~ ^sha256:[0-9a-f]{64}$ ]]
[[ "$ARCH" == arm64 || "$ARCH" == amd64 ]]
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CHECKS="$ROOT/extras/ci"
mkdir -p "$RESULTS"
RESULTS="$(cd "$RESULTS" && pwd)"
chmod 777 "$RESULTS"
docker image inspect "$REF" > "$RESULTS/image-inspect.json"
test "$(docker image inspect --format '{{.Id}}' "$REF")" = "$IMAGE_ID"
test "$(docker image inspect --format '{{.Architecture}}' "$REF")" = "$ARCH"
printf '%s\n' "$REF" > "$RESULTS/pulled-reference.txt"
printf 'case,status\n' > "$RESULTS/summary.csv"
CURRENT=""
cleanup() { if [[ -n "$CURRENT" ]]; then docker rm -f "$CURRENT" >/dev/null 2>&1 || true; fi; }
trap cleanup EXIT
trap 'exit 130' INT TERM
FAILED=0
CASES="inventory $(awk -F '\t' '!/^#/ && NF {print $1}' "$CHECKS/quickstarts.tsv") camera-offline"
for RECORD in $CASES; do
  CASE="$RECORD"
  NETWORK=bridge
  if [[ "$RECORD" == camera-offline ]]; then CASE=camera; NETWORK=none; fi
  mkdir -p "$RESULTS/$RECORD"
  chmod 777 "$RESULTS/$RECORD"
  CURRENT="posim-pull-${GITHUB_RUN_ID:-$$}-$ARCH-$RECORD"
  docker create --init --name "$CURRENT" --platform "linux/$ARCH" --network "$NETWORK" \
    --shm-size=1g --entrypoint bash -e ROS_DOMAIN_ID=124 -e GZ_IP=127.0.0.1 \
    -e LIBGL_ALWAYS_SOFTWARE=1 -e QT_QPA_PLATFORM=offscreen -e "CASE=$CASE" -e "RECORD=$RECORD" \
    -v "$CHECKS:/checks:ro" -v "$CHECKS/../patches:/patches:ro" \
    -v "$CHECKS/../fuel:/fuel:ro" -v "$RESULTS:/results" "$REF" -c '
      set -eo pipefail
      source /opt/ros/lyrical/setup.bash
      export POSIM_WORKSPACE="${DAVE_WS:-$DAVE_UNDERLAY}"
      source "$POSIM_WORKSPACE/install/setup.bash"
      cd /tmp
      exec python3 /checks/image_smoke.py "$CASE" --record "$RECORD"
    ' > "$RESULTS/$RECORD/container-id.txt"
  docker inspect --format '{{json .HostConfig.NetworkMode}}' "$CURRENT" \
    > "$RESULTS/$RECORD/docker-network-mode.json"
  container_image="$(docker inspect --format '{{.Image}}' "$CURRENT")"
  printf '%s\n' "$container_image" > "$RESULTS/$RECORD/container-image-id.txt"
  requested_image="$(docker inspect --format '{{.Config.Image}}' "$CURRENT")"
  printf '%s\n' "$requested_image" > "$RESULTS/$RECORD/container-requested-image.txt"
  test "$requested_image" = "$REF"
  # Both digests were verified against the same registry manifest before create.
  [[ "$container_image" == "$IMAGE_ID" || "$container_image" == "$CONFIG_ID" ]]
  test "$(docker inspect --format '{{.HostConfig.NetworkMode}}' "$CURRENT")" = "$NETWORK"
  docker start -a "$CURRENT" || true
  CODE="$(docker inspect --format '{{.State.ExitCode}}' "$CURRENT")"
  STATE="$(docker inspect --format '{{.State.Status}}' "$CURRENT")"
  printf '%s\n' "$CODE" > "$RESULTS/$RECORD/container-exit-code.txt"
  printf '%s\n' "$STATE" > "$RESULTS/$RECORD/container-state.txt"
  if [[ "$STATE" == exited && "$CODE" == 0 ]] && \
      python3 -c 'import json,sys; sys.exit(json.load(open(sys.argv[1]))["status"] != "PASS")' \
        "$RESULTS/$RECORD/result.json"; then
    printf '%s,PASS\n' "$RECORD" >> "$RESULTS/summary.csv"
  else
    printf '%s,FAIL\n' "$RECORD" >> "$RESULTS/summary.csv"
    FAILED=1
  fi
  cleanup
  CURRENT=""
done
cat "$RESULTS/summary.csv"
exit "$FAILED"
