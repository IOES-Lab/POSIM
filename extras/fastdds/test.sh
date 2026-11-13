#!/usr/bin/env bash
# Exercise participant teardown while other processes request type information.
set -eo pipefail
results=${1:?Usage: test.sh RESULTS_DIRECTORY}
workspace=${POSIM_WS:-${POSIM_UNDERLAY:?POSIM workspace is not configured}}
prefix=/opt/posim/fastdds
sha256sum -c "$prefix/underlay.sha256"
sha256sum -c "$prefix/library.sha256"
python3 - <<'PY'
import ctypes
from pathlib import Path

ctypes.CDLL('librmw_fastrtps_cpp.so')
libraries = {
    line.split()[-1] for line in Path('/proc/self/maps').read_text().splitlines()
    if '/libfastdds.so.' in line
}
expected = '/opt/posim/fastdds/lib/libfastdds.so.3.6.2.0'
if libraries != {expected}:
    raise SystemExit(f'Expected {expected}; loaded {sorted(libraries)}')
print(f'Loaded {expected}')
PY
mkdir -p "$results"
results=$(cd -- "$results" && pwd)
export ROS_DOMAIN_ID=137 ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
export FASTDDS_DEFAULT_PROFILES_FILE="$workspace/install/share/multibeam_sonar/config/fastdds_macos_local.xml"
for batch in $(seq 1 10); do
    pids=()
    for worker in $(seq 1 12); do
        timeout 30 "$workspace/build/posim_ros_gz_plugins/test_spherical_coords" \
            --gtest_output="xml:$results/$batch-$worker.xml" \
            > "$results/$batch-$worker.log" 2>&1 &
        pids+=("$!")
    done
    status=0
    for pid in "${pids[@]}"; do
        wait "$pid" || status=1
    done
    if (( status != 0 )); then
        cat "$results/$batch-"*.log
        exit 1
    fi
done
python3 - "$results" <<'PY'
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

files = list(Path(sys.argv[1]).glob('*.xml'))
assert len(files) == 120, f'Expected 120 process results; found {len(files)}'
for path in files:
    root = ET.parse(path).getroot()
    assert int(root.get('tests', '0')) == 5, path
    for field in ('failures', 'errors', 'disabled'):
        assert int(root.get(field, '0')) == 0, (path, field)
    assert not root.findall('.//skipped'), path
print('120 concurrent process runs: 600 tests passed, none skipped')
PY
