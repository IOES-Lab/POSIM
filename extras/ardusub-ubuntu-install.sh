#!/usr/bin/env bash
set -euo pipefail
POSIM_INSTALL_EXTRAS=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

# Keep the existing SPS/PPS fix and apply only reviewed camera backports.
# Resolve before changing working directory during the full installer.
POSIM_PREMERGE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/premerge"
patch_gst_camera() {
    python3 "$POSIM_PREMERGE_DIR/apply.py" gst --source "$1"
}

if [[ "${1:-}" == "--patch-gst-camera" ]]; then
    if [[ "$#" != 2 ]]; then
        echo "Usage: $0 --patch-gst-camera ARDUPILOT_GAZEBO_SOURCE" >&2
        exit 2
    fi
    patch_gst_camera "$2"
    exit 0
fi

# Source Ros and Gazebo
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
set +u
# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO}/setup.bash"
set -u
export GZ_VERSION="${GZ_VERSION:-jetty}"

# ArduPilot 4.7.2 stable for ArduSub and ArduRover.
ARDUPILOT_COMMIT="${ARDUPILOT_COMMIT:-fb773567896014da007bca58358704d4a0d8516f}"
ARDUPILOT_GAZEBO_COMMIT="${ARDUPILOT_GAZEBO_COMMIT:-082a0fe231f6e63bc8d1598f1cba461d9e2ea7f5}"
# Fetch pinned revisions over HTTP/1.1 and retry interrupted GitHub transfers.
retry() {
    local attempt=1
    until "$@"; do
        if [ "$attempt" -ge 3 ]; then return 1; fi
        echo "Git transfer failed; retrying in $((attempt * 5)) seconds" >&2
        sleep "$((attempt * 5))"
        attempt=$((attempt + 1))
    done
}

mkdir -p "/opt/ardusub_ws" && cd "/opt/ardusub_ws" || exit
git init ardupilot
cd "/opt/ardusub_ws/ardupilot" || exit
git remote get-url origin >/dev/null 2>&1 || \
    git remote add origin https://github.com/ArduPilot/ardupilot.git
retry git -c http.version=HTTP/1.1 -c http.lowSpeedLimit=1 -c http.lowSpeedTime=60 \
    fetch --depth 1 --no-tags origin "$ARDUPILOT_COMMIT"
git checkout --detach "$ARDUPILOT_COMMIT"
retry git -c http.version=HTTP/1.1 -c http.lowSpeedLimit=1 -c http.lowSpeedTime=60 \
    submodule update --init --recursive --depth 1 --jobs 2

# ArduPilot's waf extras still import Python modules removed in 3.12/3.13.
# These minimal compatibility shims were used in the verified Ubuntu 26.04 build.
mkdir -p /opt/ardusub_ws/python_compat
printf 'import types\ndef new_module(name):\n    return types.ModuleType(name)\n' \
  > /opt/ardusub_ws/python_compat/imp.py
printf 'import shlex\ndef quote(value):\n    return shlex.quote(value)\n' \
  > /opt/ardusub_ws/python_compat/pipes.py
export PYTHONPATH="/opt/ardusub_ws/python_compat:/opt/ardusub_ws/ardupilot/modules/waf/waflib/extras${PYTHONPATH:+:$PYTHONPATH}"
export PIP_BREAK_SYSTEM_PACKAGES=1

# Install ArduSub dependencies
export SKIP_AP_EXT_ENV=1 SKIP_AP_GRAPHIC_ENV=1 SKIP_AP_COV_ENV=1 SKIP_AP_GIT_CHECK=1
# Do not install the STM development tools
export DO_AP_STM_ENV=0
# Do not activate the Ardupilot venv by default
export DO_PYTHON_VENV_ENV=0
# argparse is included in Python 3; keep upstream package discovery intact.
sed -i -E '/SITL_PKGS.*argparse/s/python3?-argparse/libpython3-stdlib/g' \
  Tools/environment_install/install-prereqs-ubuntu.sh
# Keep the ROS 2 Lyrical colcon requirement (setuptools < 80) intact.
sed -i 's/^SETUPTOOLS="setuptools"$/SETUPTOOLS="setuptools<80"/' \
  Tools/environment_install/install-prereqs-ubuntu.sh
# This system-wide helper is invoked by the root-owned Docker build. The pinned
# ArduPilot prerequisite script rejects EUID 0 before using sudo for the same
# package operations, so remove only that guard in this container installer.
# shellcheck disable=SC2016
sed -i '/^if \[ \$EUID == 0 \]; then$/,/^fi$/d' \
  Tools/environment_install/install-prereqs-ubuntu.sh
Tools/environment_install/install-prereqs-ubuntu.sh -y

# Build both vehicles from the shared stable revision.
python3 modules/waf/waf-light configure --board sitl
python3 modules/waf/waf-light build --target bin/ardusub,bin/ardurover

# Fetch the pinned ardupilot_gazebo revision without cloning the default branch.
cd "/opt/ardusub_ws" || exit
git init ardupilot_gazebo
cd "/opt/ardusub_ws/ardupilot_gazebo" || exit
git remote get-url origin >/dev/null 2>&1 || \
    git remote add origin https://github.com/ArduPilot/ardupilot_gazebo.git
retry git -c http.version=HTTP/1.1 -c http.lowSpeedLimit=1 -c http.lowSpeedTime=60 \
    fetch --depth 1 --no-tags origin "$ARDUPILOT_GAZEBO_COMMIT"
git checkout --detach "$ARDUPILOT_GAZEBO_COMMIT"
patch_gst_camera "$PWD"
python3 "$POSIM_INSTALL_EXTRAS/ardupilot_gazebo/patch_resampling.py" "$PWD"

# Install ardupilot_gazebo plugin
# Check if the directory creation was successful
mkdir -p "/opt/ardusub_ws/ardupilot_gazebo/build" \
  && cd "/opt/ardusub_ws/ardupilot_gazebo/build" || exit
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -j2

# Add results of ArduSub build
export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:$PATH
# Optional: add autotest to the PATH, helpful for running sim_vehicle.py
export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:$PATH
# Add ardupilot_gazebo plugin
export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build:${GZ_SIM_SYSTEM_PLUGIN_PATH:-}
# Add ardupilot_gazebo models and worlds
export GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:${GZ_SIM_RESOURCE_PATH:-}
