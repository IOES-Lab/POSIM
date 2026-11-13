#!/usr/bin/env bash
# Validate the caller's final runtime environment, without repairing its paths.
set -eo pipefail
prefix=/opt/posim/rendering-compat
results=${1:?Usage: test.sh RESULTS_DIRECTORY}
mkdir -p "$results"
results=$(cd "$results" && pwd)
sha256sum -c "$prefix/library.sha256"
sha256sum -c "$prefix/underlay.sha256"
sha256sum -c "${POSIM_WAVE_PREFIX:-/opt/waves}/share/posim-waves/library.sha256"
export POSIM_EXPECT_RENDERING_PREFIX="$prefix" LIBGL_ALWAYS_SOFTWARE=1
export GZ_ENGINE_TO_TEST=ogre2 GZ_ENGINE_BACKEND=gl3plus
export GZ_RENDERING_INSTALL_PREFIX="$prefix"
for name in dynamic waves; do
    timeout 90 xvfb-run -a "$prefix/test-build/$name-material-test" \
        > "$results/$name-material.log" 2>&1
    cat "$results/$name-material.log"
done
for name in lidar_visual gpu_rays load_unload; do
    timeout 240 xvfb-run -a "$prefix/build/bin/INTEGRATION_$name" \
        --gtest_output="xml:$results/$name.xml" > "$results/$name.log" 2>&1
done
python3 - "$results" <<'PY'
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
for name in ('lidar_visual', 'gpu_rays', 'load_unload'):
    root = ET.parse(Path(sys.argv[1]) / (name + '.xml')).getroot()
    assert int(root.get('tests', '0')) > 0, name
    for field in ('failures', 'errors', 'disabled'):
        assert int(root.get(field, '0')) == 0, (name, field)
    assert not root.findall('.//skipped'), name
    print(name, root.attrib)
PY
