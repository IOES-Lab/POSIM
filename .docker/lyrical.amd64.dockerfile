ARG ROS_DISTRO="lyrical"
FROM osrf/ros:${ROS_DISTRO}-desktop-full

ARG ROS_DISTRO="lyrical"
ENV ROS_DISTRO=${ROS_DISTRO}
ENV DEBIAN_FRONTEND=noninteractive
ENV POSIM_MAVROS_OVERLAY=/opt/posim/mavros-router

# The installer expects sudo even when the image build runs as root.
RUN apt-get update && \
    apt-get install -y --no-install-recommends sudo ca-certificates xz-utils && \
    rm -rf /var/lib/apt/lists/*

# Install ROS 2 Lyrical, Gazebo Jetty, ArduSub, and MAVROS from this checkout.
COPY extras /tmp/posim-extras
RUN POSIM_EXTRAS_DIR=/tmp/posim-extras \
    bash /tmp/posim-extras/ros-lyrical-gz-jetty-install.sh
ENV GZ_RENDERING_RESOURCE_PATH=/opt/posim/terra
ENV LD_LIBRARY_PATH=/opt/waves/lib
ENV GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/waves/lib:/opt/ardusub_ws/ardupilot_gazebo/build
ENV GZ_SIM_RESOURCE_PATH=/opt/asv_wave_sim/gz-waves-models/models:/opt/asv_wave_sim/gz-waves-models/world_models:/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds

# Install QGroundControl.
RUN mkdir -p /opt/QGC && cd /opt/QGC && \
    wget -O QGroundControl-x86_64.AppImage \
      "https://d176tv9ibo4jno.cloudfront.net/latest/QGroundControl-x86_64.AppImage" && \
    chmod +x QGroundControl-x86_64.AppImage && \
    ./QGroundControl-x86_64.AppImage --appimage-extract && \
    mv squashfs-root/* /opt/QGC/ && \
    chmod -R a+rX /opt/QGC && \
    rm QGroundControl-x86_64.AppImage

COPY .docker/qgroundcontrol.sh /usr/local/bin/qgroundcontrol
RUN useradd --create-home --user-group --groups dialout --shell /bin/bash posim-qgc && \
    chmod 0755 /usr/local/bin/qgroundcontrol

# Install Firefox from Mozilla.
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64&lang=en-US" \
      -o /tmp/firefox.tar.xz && \
    tar -xJf /tmp/firefox.tar.xz -C /opt && \
    ln -sf /opt/firefox/firefox /usr/local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

# Build the exact POSIM revision supplied as the Docker build context. Import the
# companion repositories, but never replace the checked-out POSIM source.
ENV POSIM_WS=/opt/posim_ws
WORKDIR $POSIM_WS/src
COPY . posim
RUN vcs import --shallow --skip-existing \
      --input posim/extras/repos/posim.lyrical.repos

RUN apt-get update && \
    rosdep update --rosdistro "$ROS_DISTRO" && \
    rosdep install --rosdistro "$ROS_DISTRO" -iy --from-paths . && \
    rm -rf /var/lib/apt/lists/*

WORKDIR $POSIM_WS
RUN . "/opt/ros/${ROS_DISTRO}/setup.sh" && \
    . "$POSIM_MAVROS_OVERLAY/install/setup.sh" && \
    colcon build --merge-install --executor sequential --symlink-install

RUN . "$POSIM_WS/install/setup.sh" && \
    test "$(ros2 pkg prefix mavros)" = "$POSIM_MAVROS_OVERLAY/install" && \
    python3 "$POSIM_WS/src/posim/extras/mavros/test_library_selection.py" \
      "$POSIM_MAVROS_OVERLAY/install"

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /root/.bashrc && \
    echo "source $POSIM_WS/install/setup.bash" >> /root/.bashrc && \
    echo "export PS1='\[\e[1;36m\]\u@POSIM_docker\[\e[0m\]\[\e[1;34m\](\$(hostname | cut -c1-12))\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >> /root/.bashrc

RUN touch /root/.posim_entrypoint && \
    printf '\033[1;36mPOSIM - Platform for Ocean Simulation\033[0m\n' >> /root/.posim_entrypoint && \
    printf '\033[1;33mROS 2 Lyrical · Gazebo Jetty · ArduSub · MAVROS\033[0m\n\n' >> /root/.posim_entrypoint && \
    echo 'cat /root/.posim_entrypoint' >> /root/.bashrc

# Direct docker exec / entrypoint commands also need the installed autopilot.
ENV PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:/opt/ardusub_ws/ardupilot/Tools/autotest:${PATH}
ENV GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids

WORKDIR /root

LABEL org.opencontainers.image.title="POSIM" \
      org.opencontainers.image.description="Platform for Ocean Simulation" \
      org.opencontainers.image.source="https://github.com/IOES-Lab/POSIM" \
      org.opencontainers.image.licenses="Apache-2.0 AND GPL-3.0-or-later"
