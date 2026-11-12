#!/usr/bin/env bash
# Test the already selected environment, including dynamic library precedence.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "Usage: $0 EXPECTED_PREFIX TEST_DIRECTORY" >&2; exit 2; fi
root=$(cd -- "$(dirname -- "$0")" && pwd)
prefix=$(realpath "$1")
output=$2
test_domain="${POSIM_RVIZ_TEST_DOMAIN_ID:-87}"
if [[ ! "$test_domain" =~ ^[0-9]{1,3}$ ]] || (( 10#$test_domain > 232 )); then
  echo "POSIM_RVIZ_TEST_DOMAIN_ID must be an integer from 0 to 232" >&2
  exit 2
fi
test_domain=$((10#$test_domain))
mkdir -p "$output"
output=$(cd -- "$output" && pwd)
cmake -S "$root/test" -B "$output/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$output/build" --parallel 1
mkdir -p "$output/home"
# Use a private display and home, not the user's RViz settings or running desktop.
attempt=0
for scenario in discard discard discard cancel save unchanged; do
  attempt=$((attempt + 1))
  name="$attempt-$scenario"
  HOME="$output/home" XDG_CONFIG_HOME="$output/home/config" \
    POSIM_RVIZ_TEST_MAPS="$output/$name.maps" \
    LIBGL_ALWAYS_SOFTWARE=1 QT_QPA_PLATFORM=xcb \
    RMW_IMPLEMENTATION=rmw_fastrtps_cpp \
    ROS_DOMAIN_ID="$test_domain" ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST \
    timeout 45s xvfb-run -a -s '-screen 0 1280x800x24 -nolisten tcp' \
    "$output/build/rviz_close_test" "$prefix" "$scenario" 2>&1 | tee "$output/$name.log"
done
