#!/usr/bin/env bash
set -euo pipefail

# QGC bundles GStreamer, while Gazebo uses Ubuntu's GStreamer. Keep both the
# plugin search and registry process-local; never delete the user's shared cache.
app_dir="${HOME}/QGC"
plugins="${app_dir}/usr/lib/gstreamer-1.0"
scanner="${app_dir}/usr/lib/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner"
if [[ ! -x "${app_dir}/AppRun" || ! -d "$plugins" || ! -x "$scanner" ]]; then
  printf 'QGroundControl bundle is incomplete: %s\n' "$app_dir" >&2
  exit 1
fi

# XDG_CACHE_HOME must be absolute. Ignore invalid relative values.
case "${XDG_CACHE_HOME:-}" in
  /*) cache_root="$XDG_CACHE_HOME" ;;
  *) cache_root="${HOME}/.cache" ;;
esac
cache_dir="${cache_root}/posim-qgc/gstreamer-1.0"
# Only the final QGC registry directory needs private permissions.
# shellcheck disable=SC2174
mkdir -p -m 0700 "$cache_dir"

export APPDIR="$app_dir"
export GST_PLUGIN_PATH="$plugins" GST_PLUGIN_PATH_1_0="$plugins"
export GST_PLUGIN_SYSTEM_PATH="" GST_PLUGIN_SYSTEM_PATH_1_0=""
export GST_PLUGIN_SCANNER="$scanner" GST_PLUGIN_SCANNER_1_0="$scanner"
GST_REGISTRY="${cache_dir}/registry.$(uname -m).bin"
export GST_REGISTRY
export GST_REGISTRY_1_0="$GST_REGISTRY"

exec "${app_dir}/AppRun" "$@"
