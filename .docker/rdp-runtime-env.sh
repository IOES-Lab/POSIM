# SPDX-License-Identifier: Apache-2.0
# shellcheck shell=bash
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
    # shellcheck disable=SC2163 # Export the named path variable, not its name holder.
    export "$variable"
}
_posim_prepend_existing GZ_SIM_RESOURCE_PATH /opt/asv_wave_sim/gz-waves-models/world_models
_posim_prepend_existing GZ_SIM_RESOURCE_PATH /opt/asv_wave_sim/gz-waves-models/models
_posim_prepend_existing GZ_SIM_SYSTEM_PLUGIN_PATH /opt/waves/lib
_posim_prepend_existing LD_LIBRARY_PATH /opt/waves/lib
_posim_prepend_existing LD_LIBRARY_PATH /opt/posim/fastdds/lib
_posim_prepend_existing LD_LIBRARY_PATH /opt/posim/rendering-compat/lib
_posim_prepend_existing GZ_RENDERING_PLUGIN_PATH /opt/posim/rendering-compat/lib
# Restore the image's already-installed pre-merge terrain shader overlay too.
# Ogre2 expects a single directory here, not a colon-separated search path.
# Leave an explicit user overlay intact; only restore the image default if empty.
if [ -z "${GZ_RENDERING_RESOURCE_PATH:-}" ] && [ -d /opt/posim/terra ]; then
    export GZ_RENDERING_RESOURCE_PATH=/opt/posim/terra
fi
unset -f _posim_prepend_existing
