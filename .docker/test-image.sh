#!/usr/bin/env bash
# Run inside a freshly built POSIM image. No host display or GPU is required.
set -eo pipefail

workspace="${POSIM_WS:-${POSIM_UNDERLAY:?POSIM workspace is not configured}}"
results=/tmp/posim-image-tests
mkdir -p "$results"
# shellcheck disable=SC1091
source "$workspace/install/setup.bash"
cd "$workspace"

# Validate the final workspace setup selects the corrected RViz library and that
# Cancel keeps real render updates alive, accepted close stops them, and teardown
# releases the view manager and DDS participant resources.
bash "$workspace/src/posim/extras/rviz/test_overlay.sh" \
  "${POSIM_RVIZ_OVERLAY:?RViz overlay is not configured}/install" "$results/rviz-close"

export ROS_DOMAIN_ID=137 ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST
export GZ_PARTITION="posim_image_test_${HOSTNAME}"
export LIBGL_ALWAYS_SOFTWARE=1 QT_QPA_PLATFORM=offscreen
export AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1 CTEST_PARALLEL_LEVEL=2
# The image test's processes share one container. Use the installed local DDS
# profile so large sonar images do not depend on the runner's UDP buffer sizes.
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
export FASTDDS_DEFAULT_PROFILES_FILE="$workspace/install/share/multibeam_sonar/config/fastdds_macos_local.xml"

status=0
colcon --log-base "$results/colcon" test --merge-install --base-paths src/posim \
  --return-code-on-test-failure || status=1
colcon test-result --verbose | tee "$results/test-result.log" || status=1
if (( status != 0 )); then
  exit "$status"
fi

sonar="$workspace/src/posim/gazebo/posim_gz_multibeam_sonar"
python3 "$sonar/multibeam_sonar/test/test_empty_return_display.py" \
  --backend cpu --render-backend headless --output "$results/sonar-display"
python3 "$sonar/multibeam_sonar_system/test/test_sensor_creation.py" \
  --backend cpu --render-backend headless --output "$results/sonar-creation"
