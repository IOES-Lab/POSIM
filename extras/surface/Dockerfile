# Optional ARM64 overlay; AMD64 has a different user/workspace layout.
ARG POSIM_BASE_IMAGE=posim:dev-arm64-rdp
FROM ${POSIM_BASE_IMAGE}
USER root
SHELL ["/bin/bash", "-c"]
RUN apt-get update && apt-get install -y --no-install-recommends libcgal-dev libfftw3-dev libboost-iostreams-dev libboost-system-dev python3-scipy ros-lyrical-mavros-extras && rm -rf /var/lib/apt/lists/*
COPY extras/install-waves.sh /tmp/posim-extras/install-waves.sh
COPY extras/surface/dependency.json extras/surface/patch_external_waves.py /tmp/posim-extras/surface/
RUN bash /tmp/posim-extras/install-waves.sh
RUN su docker -c 'cd /home/docker/ardupilot && python3 modules/waf/waf-light build --target bin/ardurover -j2' && cp /home/docker/ardupilot/build/sitl/bin/ardurover /usr/local/bin/ardurover
# Retain upstream GPL source and notices. No code is copied into POSIM.
ENV LD_LIBRARY_PATH=/opt/waves/lib
ENV GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/waves/lib:/home/docker/ardupilot_gazebo/build
ENV GZ_SIM_RESOURCE_PATH=/opt/asv_wave_sim/gz-waves-models/models:/opt/asv_wave_sim/gz-waves-models/world_models:/home/docker/ardupilot_gazebo/models:/home/docker/ardupilot_gazebo/worlds
COPY extras/surface /opt/posim-surface
RUN mkdir /tmp/posim-surface && chown docker:docker /tmp/posim-surface
USER docker

LABEL org.opencontainers.image.licenses="Apache-2.0 AND GPL-3.0-or-later"
