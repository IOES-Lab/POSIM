#!/usr/bin/env bash
set -euo pipefail

# Keep the simulator's environment while running desktop applications with
# the image's existing unprivileged application account.
if [[ "$EUID" -eq 0 ]]; then
  if [[ -n "${XAUTHORITY:-}" ]]; then
    if [[ ! -r "$XAUTHORITY" ]]; then
      printf 'Cannot read the X11 authorization file: %s\n' "$XAUTHORITY" >&2
      exit 1
    fi
    # The host mount stays read-only and private. Only this application account
    # can read the copy; it cannot replace files in the root-owned directory.
    install -d -m 0750 -o root -g posim-qgc /run/posim-qgc
    install -m 0640 -o root -g posim-qgc "$XAUTHORITY" /run/posim-qgc/Xauthority
    export XAUTHORITY=/run/posim-qgc/Xauthority
  fi
  exec runuser -u posim-qgc -- env HOME=/home/posim-qgc "$@"
fi

exec "$@"
