ARG ROS_DISTRO="lyrical"
FROM osrf/ros:${ROS_DISTRO}-desktop-full

ARG ROS_DISTRO="lyrical"
ENV ROS_DISTRO=${ROS_DISTRO}
ENV DEBIAN_FRONTEND=noninteractive

# The installer expects sudo even when the image build runs as root.
RUN apt-get update && \
    apt-get install -y --no-install-recommends sudo ca-certificates xz-utils && \
    rm -rf /var/lib/apt/lists/*

# Install ROS 2 Lyrical, Gazebo Jetty, ArduSub, and MAVROS from this checkout.
COPY extras /tmp/dave-extras
RUN DAVE_EXTRAS_DIR=/tmp/dave-extras \
    bash /tmp/dave-extras/ros-lyrical-gz-jetty-install.sh

# docker run/exec do not necessarily start an interactive shell. Keep these
# paths in the image environment, not just the installer's ~/.bashrc hook.
ENV PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:/opt/ardusub_ws/ardupilot/Tools/autotest:${PATH}
ENV GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build
ENV GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds
ENV GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids
ENV POSIM_BRIDGE_UNDERLAY=/opt/posim_bridge_ws
RUN bash /tmp/dave-extras/build-image-bridge.sh

# Install QGroundControl.
RUN mkdir -p /opt/QGC && cd /opt/QGC && \
    wget -O QGroundControl-x86_64.AppImage \
      "https://d176tv9ibo4jno.cloudfront.net/latest/QGroundControl-x86_64.AppImage" && \
    chmod +x QGroundControl-x86_64.AppImage && \
    ./QGroundControl-x86_64.AppImage --appimage-extract && \
    mv squashfs-root/* /opt/QGC/ && \
    rm QGroundControl-x86_64.AppImage && \
    ln -sf /opt/QGC/AppRun /usr/local/bin/qgroundcontrol

# Install Firefox from Mozilla.
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64&lang=en-US" \
      -o /tmp/firefox.tar.xz && \
    tar -xJf /tmp/firefox.tar.xz -C /opt && \
    ln -sf /opt/firefox/firefox /usr/local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

# Build the exact POSIM revision supplied as the Docker build context. Import the
# companion repositories, but never replace the checked-out POSIM source.
ENV DAVE_WS=/opt/dave_ws
WORKDIR $DAVE_WS/src
COPY . dave
RUN vcs import --shallow --skip-existing \
      --input dave/extras/repos/posim.lyrical.repos

RUN apt-get update && \
    rosdep update --rosdistro "$ROS_DISTRO" && \
    rosdep install --rosdistro "$ROS_DISTRO" -iy --from-paths . && \
    rm -rf /var/lib/apt/lists/*

WORKDIR $DAVE_WS
RUN . "$POSIM_BRIDGE_UNDERLAY/install/setup.sh" && \
    colcon build --merge-install --executor sequential --symlink-install

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /root/.bashrc && \
    echo "source $DAVE_WS/install/setup.bash" >> /root/.bashrc && \
    echo "export PS1='\[\e[1;36m\]\u@POSIM_docker\[\e[0m\]\[\e[1;34m\](\$(hostname | cut -c1-12))\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >> /root/.bashrc

RUN touch /root/.dave_entrypoint && \
    printf '\033[1;36mPOSIM - Platform for Ocean Simulation\033[0m\n' >> /root/.dave_entrypoint && \
    printf '\033[1;33mROS 2 Lyrical · Gazebo Jetty · ArduSub · MAVROS\033[0m\n\n' >> /root/.dave_entrypoint && \
    echo 'cat /root/.dave_entrypoint' >> /root/.bashrc

WORKDIR /root

LABEL org.opencontainers.image.title="POSIM" \
      org.opencontainers.image.description="Platform for Ocean Simulation" \
      org.opencontainers.image.source="https://github.com/IOES-Lab/POSIM" \
      org.opencontainers.image.licenses="Apache-2.0"
