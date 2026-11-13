# shellcheck shell=sh
# Select the matching Linux engine after ROS underlays have populated their paths.
if [ -d /opt/posim/rendering-compat/lib ]; then
    ament_prepend_unique_value LD_LIBRARY_PATH /opt/posim/rendering-compat/lib
    ament_prepend_unique_value GZ_RENDERING_PLUGIN_PATH /opt/posim/rendering-compat/lib
fi
