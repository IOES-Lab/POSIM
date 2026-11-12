#!/usr/bin/env bash
# Build an isolated RViz 15.2.6 overlay; never overwrite /opt/ros.
set -eo pipefail
if [ "$#" -ne 1 ]; then echo "Usage: $0 NEW_WORKSPACE_DIRECTORY" >&2; exit 2; fi
root=$(cd -- "$(dirname -- "$0")" && pwd)
workspace=$1
if [ -e "$workspace" ]; then
  echo "Refusing an existing path: $workspace (choose a new empty location)" >&2; exit 2
fi
# shellcheck source=/dev/null
source /opt/ros/lyrical/setup.bash
for tool in curl tar git colcon sha256sum cmake xvfb-run timeout; do
  command -v "$tool" >/dev/null
done
python3 - <<'CHECK'
from pathlib import Path
import xml.etree.ElementTree as ET
for package in ('rviz_common', 'rviz_rendering', 'rviz_default_plugins', 'rviz2'):
    path = Path('/opt/ros/lyrical/share') / package / 'package.xml'
    version = ET.parse(path).findtext('version')
    if version != '15.2.6':
        raise SystemExit(
            f'This close correction requires {package} 15.2.6; found {version}. '
            'Review the official RViz release before updating or retiring the overlay.'
        )
CHECK
mkdir -p "$workspace/src"
workspace=$(cd -- "$workspace" && pwd)
cd "$workspace"
revision=baab61a68bc089217dfaa4f270276dc7a30268b1
curl --fail --location --connect-timeout 20 --max-time 300 --retry 2 \
  "https://codeload.github.com/ros2/rviz/tar.gz/$revision" -o source.tar.gz
printf '%s  %s\n' ca0ed4dc39bf1652db788baa2ba24ac7d3ff23b1b925653a25806de39b2708cf source.tar.gz | sha256sum -c -
tar -xzf source.tar.gz -C src
cd "src/rviz-$revision"
git apply --check "$root/stop-update-on-close.patch"
git apply "$root/stop-update-on-close.patch"
git apply --check "$root/release-view-manager.patch"
git apply "$root/release-view-manager.patch"
cd "$workspace"
CMAKE_BUILD_PARALLEL_LEVEL=1 MAKEFLAGS=-j1 colcon build --merge-install \
  --packages-select rviz_common --allow-overriding rviz_common --executor sequential \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  '-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=-O1 -g0 -DNDEBUG' -DBUILD_TESTING=OFF
# shellcheck source=/dev/null
source install/local_setup.bash
bash "$root/test_overlay.sh" "$workspace/install" "$workspace/regression"
cat > PROVENANCE.txt <<'RECEIPT'
RViz tag: 15.2.6
Base commit: baab61a68bc089217dfaa4f270276dc7a30268b1
Correction: stop VisualizationManager updates only after accepting frame close.
Correction: release the owned ViewManager and its view-controller ROS services.
Only rviz_common is rebuilt; rviz2, rendering and default plugins use the ROS underlay.
Original source and license notices are retained in src/ and source.tar.gz.
RECEIPT
printf 'Overlay built. Explicitly select it with: source %q/install/local_setup.bash\n' "$workspace"
