# SPDX-License-Identifier: Apache-2.0
# xrdp-sesman starts a clean session rather than inheriting Docker ENV.
# Source before starting XFCE. Preserve user paths and avoid duplicate entries.
_posim_prepend_existing() {
    local variable="$1" directory="$2" value
    [ -d "$directory" ] || return 0
    value="${!variable-}"
    case ":$value:" in
        *":$directory:"*) ;;
        *) printf -v "$variable" '%s' "$directory${value:+:$value}" ;;
    esac
    export "$variable"
}
_posim_prepend_existing GZ_SIM_RESOURCE_PATH /opt/asv_wave_sim/gz-waves-models/world_models
_posim_prepend_existing GZ_SIM_RESOURCE_PATH /opt/asv_wave_sim/gz-waves-models/models
_posim_prepend_existing GZ_SIM_SYSTEM_PLUGIN_PATH /opt/waves/lib
_posim_prepend_existing LD_LIBRARY_PATH /opt/waves/lib
# Restore the image's already-installed pre-merge terrain shader overlay too.
_posim_prepend_existing GZ_RENDERING_RESOURCE_PATH /opt/posim/terra
unset -f _posim_prepend_existing
