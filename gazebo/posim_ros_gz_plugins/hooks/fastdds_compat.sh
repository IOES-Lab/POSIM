# shellcheck shell=sh
# ROS setup can prepend its unpatched library ahead of an inherited Docker path.
if [ -d /opt/posim/fastdds/lib ]; then
    _posim_fastdds_first() {
        _posim_fastdds_path=/opt/posim/fastdds/lib
        while [ -n "$1" ]; do
            _posim_fastdds_entry=${1%%:*}
            if [ -n "$_posim_fastdds_entry" ] &&
                [ "$_posim_fastdds_entry" != /opt/posim/fastdds/lib ]; then
                _posim_fastdds_path="$_posim_fastdds_path:$_posim_fastdds_entry"
            fi
            case "$1" in
                *:*) set -- "${1#*:}" ;;
                *) break ;;
            esac
        done
        printf '%s' "$_posim_fastdds_path"
    }
    LD_LIBRARY_PATH=$(_posim_fastdds_first "${LD_LIBRARY_PATH:-}")
    export LD_LIBRARY_PATH
    unset -f _posim_fastdds_first
fi
