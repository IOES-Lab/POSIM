# shellcheck shell=sh
# Select the matching Linux engine after ROS underlays have populated their paths.
if [ -d /opt/posim/rendering-compat/lib ]; then
    # Docker and RDP may already export this path before ROS prepends its vendor
    # libraries. ament_prepend_unique_value leaves that existing entry in place.
    _posim_rendering_first() {
        _posim_rendering_path=/opt/posim/rendering-compat/lib
        while [ -n "$1" ]; do
            _posim_rendering_entry=${1%%:*}
            if [ -n "$_posim_rendering_entry" ] &&
                [ "$_posim_rendering_entry" != /opt/posim/rendering-compat/lib ]; then
                _posim_rendering_path="$_posim_rendering_path:$_posim_rendering_entry"
            fi
            case "$1" in
                *:*) set -- "${1#*:}" ;;
                *) break ;;
            esac
        done
        printf '%s' "$_posim_rendering_path"
    }
    LD_LIBRARY_PATH=$(_posim_rendering_first "${LD_LIBRARY_PATH:-}")
    GZ_RENDERING_PLUGIN_PATH=$(_posim_rendering_first "${GZ_RENDERING_PLUGIN_PATH:-}")
    export LD_LIBRARY_PATH GZ_RENDERING_PLUGIN_PATH
    unset -f _posim_rendering_first
fi
