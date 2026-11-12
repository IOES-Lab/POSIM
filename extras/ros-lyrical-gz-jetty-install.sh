#!/usr/bin/env bash
# Install the ROS 2 Lyrical / Gazebo Jetty development stack used by POSIM (formerly DAVE).
set -euo pipefail

# A local checkout supplies the companion installers from the same revision.
LOCAL_EXTRAS_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]:-.}")" && pwd)"
POSIM_EXTRAS_DIR="${POSIM_EXTRAS_DIR:-$LOCAL_EXTRAS_DIR}"

DIST="${ROS_DISTRO:-lyrical}"
echo
echo -e "\033[94m============================================================\033[0m"
echo -e "\033[94m== One-liner Installation Script for ROS-Gazebo Framework ==\033[0m"
echo -e "\033[94m============================================================\033[0m"
echo -e "Target stack: ROS 2 Lyrical / Gazebo Jetty"
echo -e "Requirements: Ubuntu 26.04 LTS Resolute"
echo -e "\033[94m============================================================\033[0m"

echo
echo -e "\033[96m(1/5) -------------    Updating the System  ----------------\033[0m"
echo "Performing full system upgrade (this might take a while)..."
sudo apt-get update
sudo apt-get full-upgrade -y

echo
echo -e "\033[96m(2/5) ------------    Install Dependencies   ---------------\033[0m"
echo -e "\033[34mInstalling essential tools and libraries...\033[0m"
sudo apt install -y \
    build-essential \
    cmake \
    cppcheck \
    curl \
    git \
    gnupg \
    libeigen3-dev \
    libgles2-mesa-dev \
    lsb-release \
    pkg-config \
    protobuf-compiler \
    python3-dbg \
    python3-pip \
    python3-venv \
    qtbase5-dev \
    xvfb xauth \
    ruby \
    software-properties-common \
    sudo \
    cppzmq-dev \
    wget

echo
echo -e "\033[96m(3/5) ------------    Install Package Keys   ---------------\033[0m"
echo -e "\033[34mInstalling the ROS apt source...\033[0m"
# Remove keyring if exists to avoid conflicts
sudo rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg && \
    sudo rm -rf /etc/apt/sources.list.d/ros2-latest.list
# Get Keys
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
sudo apt update && sudo apt install -y jq
# shellcheck disable=SC1091
. /etc/os-release
UBUNTU_CODENAME="${UBUNTU_CODENAME:-${VERSION_CODENAME}}"
# Use a known release instead of a rate-limited unauthenticated API lookup.
ROS_APT_SOURCE_VERSION="${ROS_APT_SOURCE_VERSION:-1.3.0}"
curl --fail --show-error --location --retry 5 --retry-delay 2 \
    -o /tmp/ros2-apt-source.deb \
    "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${UBUNTU_CODENAME}_all.deb" && \
    sudo apt-get install -y /tmp/ros2-apt-source.deb && \
    rm -f /tmp/ros2-apt-source.deb

echo
echo -e "\033[96m(4/5) ------------     Install ROS-Gazebo    ---------------\033[0m"

echo -e "\033[34mInstalling ROS 2 ${DIST}; ros-${DIST}-ros-gz supplies Gazebo Jetty vendor packages...\033[0m"
sudo apt update && sudo apt install -y \
    python3-rosdep \
    python3-catkin-pkg \
    python3-rosinstall-generator \
    python3-colcon-core \
    python3-colcon-common-extensions \
    python3-colcon-override-check \
    python3-vcstool \
    "ros-${DIST}-desktop-full" \
    "ros-${DIST}-ros-gz" \
    "ros-${DIST}-gz-ros2-control" \
    "ros-${DIST}-marine-acoustic-msgs" \
    ros-dev-tools

sudo rosdep init 2>/dev/null || true
rosdep update --rosdistro "$DIST"

echo
echo -e "\033[96m(5/5) ------------     Install ArduSub       ---------------\033[0m"
sudo apt update && sudo apt install -y \
    rapidjson-dev libopencv-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-libav gstreamer1.0-gl \
    ffmpeg python3-venv python3-websockets \
    "ros-${DIST}-joy-linux" gstreamer1.0-tools gstreamer1.0-plugins-good gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-ugly python3-gi python3-gst-1.0 \
    libfuse2 libxcb-xinerama0 libxkbcommon-x11-0 libxcb-cursor-dev \
    cython3 python3-dev python3-lxml

# Install ArduSub. The helper sources the Lyrical environment so the
# ardupilot_gazebo build resolves Jetty's vendored CMake packages.
sudo mkdir -p /opt/ardusub_ws && cd /opt/ardusub_ws || exit
if [[ -n "${POSIM_EXTRAS_DIR:-}" && -f "${POSIM_EXTRAS_DIR}/ardusub-ubuntu-install.sh" ]]; then
    ARDUPILOT_HELPERS="$POSIM_EXTRAS_DIR"
else
    ARDUPILOT_HELPERS="$(mktemp -d)"
    trap 'rm -rf -- "$ARDUPILOT_HELPERS"' EXIT
    mkdir -p "$ARDUPILOT_HELPERS/ardupilot_gazebo" "$ARDUPILOT_HELPERS/premerge"
    for ARDUPILOT_HELPER in ardusub-ubuntu-install.sh \
        ardupilot_gazebo/patch_resampling.py ardupilot_gazebo/SensorResampler.hh \
        premerge/apply.py premerge/manifest.json premerge/gst-sps.patch \
        premerge/gst-camera.patch premerge/terra.patch; do
        curl -fsSL "https://raw.githubusercontent.com/IOES-Lab/POSIM/main/extras/$ARDUPILOT_HELPER" \
            -o "$ARDUPILOT_HELPERS/$ARDUPILOT_HELPER"
    done
fi
sudo bash "$ARDUPILOT_HELPERS/ardusub-ubuntu-install.sh"


# Mavros install
sudo apt-get -y install "ros-${DIST}-mavros" "ros-${DIST}-mavros-msgs" \
    "ros-${DIST}-mavros-extras" python3-scipy \
    "ros-${DIST}-ament-cmake-google-benchmark" "ros-${DIST}-angles" \
    "ros-${DIST}-ament-cmake-gmock" "ros-${DIST}-ament-cmake-pytest" \
    "ros-${DIST}-ament-lint-auto" "ros-${DIST}-ament-lint-common"
sudo mkdir -p /opt/mavros_ws && cd /opt/mavros_ws || exit
sudo wget https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh
sudo chmod +x install_geographiclib_datasets.sh && sudo bash ./install_geographiclib_datasets.sh

# Build and test the upstream Router lifetime corrections outside the ROS packages.
MAVROS_OVERLAY=/opt/posim/mavros-router
if [[ -f "${POSIM_EXTRAS_DIR}/mavros/build_overlay.sh" ]]; then
    sudo bash "${POSIM_EXTRAS_DIR}/mavros/build_overlay.sh" "$MAVROS_OVERLAY"
else
    MAVROS_HELPERS="$(mktemp -d)"
    for MAVROS_HELPER in build_overlay.sh \
        a974025818cd6101a633c8c059c1a0155132d588.patch \
        2b788a04e69e0aaaef78b9175f816dfcc565b5a0.patch; do
        curl -fsSL "https://raw.githubusercontent.com/IOES-Lab/POSIM/main/extras/mavros/$MAVROS_HELPER" \
            -o "$MAVROS_HELPERS/$MAVROS_HELPER"
    done
    sudo bash "$MAVROS_HELPERS/build_overlay.sh" "$MAVROS_OVERLAY"
    rm -rf "$MAVROS_HELPERS"
fi

# Stop RViz updates on accepted close and release view-owned ROS resources during
# teardown, without changing files owned by the ROS packages.
RVIZ_OVERLAY=/opt/posim/rviz-close
if [[ -f "${POSIM_EXTRAS_DIR}/rviz/build_overlay.sh" ]]; then
    sudo bash "${POSIM_EXTRAS_DIR}/rviz/build_overlay.sh" "$RVIZ_OVERLAY"
else
    RVIZ_HELPERS="$(mktemp -d)"
    mkdir -p "$RVIZ_HELPERS/test"
    for RVIZ_HELPER in build_overlay.sh stop-update-on-close.patch release-view-manager.patch test_overlay.sh \
        test/CMakeLists.txt test/close_test.cpp; do
        curl -fsSL "https://raw.githubusercontent.com/IOES-Lab/POSIM/main/extras/rviz/$RVIZ_HELPER" \
            -o "$RVIZ_HELPERS/$RVIZ_HELPER"
    done
    sudo bash "$RVIZ_HELPERS/build_overlay.sh" "$RVIZ_OVERLAY"
    rm -rf "$RVIZ_HELPERS"
fi

# Fetch and build the pinned external waves during the same POSIM install.
if [[ -n "${POSIM_EXTRAS_DIR:-}" && -f "${POSIM_EXTRAS_DIR}/install-waves.sh" ]]; then
    sudo bash "${POSIM_EXTRAS_DIR}/install-waves.sh"
else
    # The one-liner installer may run without a local POSIM checkout.
    WAVE_HELPERS="$(mktemp -d)"
    mkdir -p "$WAVE_HELPERS/surface"
    for WAVE_HELPER in install-waves.sh surface/dependency.json surface/patch_external_waves.py; do
        curl -fsSL "https://raw.githubusercontent.com/IOES-Lab/POSIM/main/extras/$WAVE_HELPER" \
            -o "$WAVE_HELPERS/$WAVE_HELPER"
    done
    sudo bash "$WAVE_HELPERS/install-waves.sh"
    rm -rf "$WAVE_HELPERS"
fi

# Separate rendering resources: never modify files owned by the ROS packages.
sudo python3 "$ARDUPILOT_HELPERS/premerge/apply.py" terrain \
    --source "/opt/ros/${DIST}/opt/gz_rendering_vendor/share/gz/gz-rendering" \
    --output /opt/posim/terra

# Environment variables setup (write to ~/.posim/env and source from shell rc)
TARGET_USER="${SUDO_USER:-${USER:-}}"
if [[ -z "$TARGET_USER" ]]; then
    TARGET_USER="$(id -un)"
fi
TARGET_HOME="$(getent passwd "$TARGET_USER" | cut -d: -f6)"
TARGET_SHELL="$(getent passwd "$TARGET_USER" | cut -d: -f7)"

RC_FILE="$TARGET_HOME/.bashrc"
case "$TARGET_SHELL" in
    */zsh) RC_FILE="$TARGET_HOME/.zshrc" ;;
esac

ENV_DIR="$TARGET_HOME/.ros_ardusub_env"
ENV_FILE="$ENV_DIR/env"
mkdir -p "$ENV_DIR"

cat > "$ENV_FILE" <<EOF
if [ -n "\${ZSH_VERSION:-}" ]; then
    source /opt/ros/${DIST}/setup.zsh
    source ${MAVROS_OVERLAY}/install/local_setup.zsh
    source ${RVIZ_OVERLAY}/install/local_setup.zsh
else
    source /opt/ros/${DIST}/setup.bash
    source ${MAVROS_OVERLAY}/install/local_setup.bash
    source ${RVIZ_OVERLAY}/install/local_setup.bash
fi
export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:\$PATH
export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:\$PATH
export GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids
export GZ_VERSION=jetty
export GZ_RENDERING_RESOURCE_PATH=/opt/posim/terra
export LD_LIBRARY_PATH=/opt/waves/lib:\${LD_LIBRARY_PATH:-}
export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/waves/lib:/opt/ardusub_ws/ardupilot_gazebo/build:\${GZ_SIM_SYSTEM_PLUGIN_PATH:-}
export GZ_SIM_RESOURCE_PATH=/opt/asv_wave_sim/gz-waves-models/models:/opt/asv_wave_sim/gz-waves-models/world_models:/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:\${GZ_SIM_RESOURCE_PATH:-}
EOF

if ! grep -q "^source \$HOME/.ros_ardusub_env/env$" "$RC_FILE" 2>/dev/null; then
    echo "source \$HOME/.ros_ardusub_env/env" >> "$RC_FILE"
fi

chown -R "$TARGET_USER:$TARGET_USER" "$ENV_DIR"
chown "$TARGET_USER:$TARGET_USER" "$RC_FILE" 2>/dev/null || true

echo
echo -e "\033[32m============================================================\033[0m"
echo -e "\033[32mROS-Gazebo Framework (w mavros and Ardusub) Installation completed. Awesome! 🤘🚀 \033[0m"
echo -e "Check the installed stack with \033[33mprintenv ROS_DISTRO\033[0m and \033[33mgz sim --versions\033[0m"
echo
