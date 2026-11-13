#!/usr/bin/env bash
# Build upstream Wave Sim as a pinned external dependency of POSIM.
# Run as root after ROS Lyrical / Gazebo Jetty are installed.
set -euo pipefail

EXTRAS_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WAVE_SOURCE="${POSIM_WAVE_SOURCE:-/opt/asv_wave_sim}"
WAVE_PREFIX="${POSIM_WAVE_PREFIX:-/opt/waves}"
WAVE_COMMIT="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["commit"])' "$EXTRAS_DIR/surface/dependency.json")"
PATCH_HASH="$(sha256sum "$EXTRAS_DIR/surface/patch_external_waves.py" | cut -d ' ' -f 1)"
INSTALL_HASH="$(sha256sum "$EXTRAS_DIR/install-waves.sh" | cut -d ' ' -f 1)"
RENDERING_HASH="$(cat "$EXTRAS_DIR/rendering_compat/"{manifest.json,apply.py,build.sh,test.sh,test/CMakeLists.txt,test/dynamic_material.cc} | sha256sum | cut -d ' ' -f 1)"
UNDERLAY_HASH="$(sha256sum /opt/ros/lyrical/opt/gz_rendering_vendor/lib/libgz-rendering.so.10.0.2 \
    /opt/ros/lyrical/opt/gz_ogre_next_vendor/lib/libOgreNextMain.so.2.3.3 | sha256sum | cut -d ' ' -f 1)"
RECIPE="${WAVE_COMMIT}:${PATCH_HASH}:${INSTALL_HASH}:${RENDERING_HASH}:${UNDERLAY_HASH}:lyrical:jetty"

if [[ -f "$WAVE_PREFIX/.posim-build" ]] && [[ "$(cat "$WAVE_PREFIX/.posim-build")" == "$RECIPE" ]] &&
   [[ -f "$WAVE_SOURCE/LICENSE" ]] && [[ -d "$WAVE_PREFIX/lib" ]] &&
   sha256sum -c /opt/posim/rendering-compat/library.sha256 >/dev/null 2>&1 &&
   sha256sum -c "$WAVE_PREFIX/share/posim-waves/library.sha256" >/dev/null 2>&1; then
    echo "Wave Sim already built at the pinned POSIM revision."
    exit 0
fi

apt-get update
apt-get install -y --no-install-recommends \
    ca-certificates curl git build-essential cmake pkg-config python3 libgtest-dev \
    libcgal-dev libfftw3-dev libboost-iostreams-dev libboost-system-dev
rm -rf /var/lib/apt/lists/*

if [[ ! -d "$WAVE_SOURCE/.git" ]]; then
    if [[ -e "$WAVE_SOURCE" ]]; then
        echo "Refusing to replace an existing non-Git Wave Sim directory: $WAVE_SOURCE" >&2
        exit 1
    fi
    git init "$WAVE_SOURCE"
    git -C "$WAVE_SOURCE" remote add origin https://github.com/srmainwaring/asv_wave_sim.git
    git -C "$WAVE_SOURCE" fetch --depth 1 origin "$WAVE_COMMIT"
    git -C "$WAVE_SOURCE" checkout --detach FETCH_HEAD
fi
if [[ "$(git -C "$WAVE_SOURCE" rev-parse HEAD)" != "$WAVE_COMMIT" ]]; then
    echo "Wave Sim checkout does not match dependency.json; use a fresh dependency directory." >&2
    exit 1
fi
if [[ ! -f "$WAVE_SOURCE/.posim-build-adaptation" ]]; then
    python3 "$EXTRAS_DIR/surface/patch_external_waves.py" "$WAVE_SOURCE"
    printf '%s\n' "$PATCH_HASH" > "$WAVE_SOURCE/.posim-build-adaptation"
elif [[ "$(cat "$WAVE_SOURCE/.posim-build-adaptation")" != "$PATCH_HASH" ]]; then
    echo "The Wave Sim compatibility patch changed; use a fresh dependency directory." >&2
    exit 1
fi

# Apply both guarded source corrections and build the separate rendering engine.
bash "$EXTRAS_DIR/rendering_compat/build.sh" "$WAVE_SOURCE"

# ROS setup files do not promise nounset compatibility.
set +u
# shellcheck disable=SC1091
source /opt/ros/lyrical/setup.bash
set -u
export PKG_CONFIG_PATH="/opt/ros/lyrical/opt/gz_ogre_next_vendor/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
export GZ_VERSION=jetty
cmake -S "$WAVE_SOURCE/gz-waves" -B "$WAVE_SOURCE/build" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX="$WAVE_PREFIX"
cmake --build "$WAVE_SOURCE/build" -j "${POSIM_WAVE_JOBS:-2}"
cmake --install "$WAVE_SOURCE/build"
cmake -S "$EXTRAS_DIR/rendering_compat/test" -B /opt/posim/rendering-compat/test-build \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_RPATH=ON \
    -DWAVE_SOURCE="$WAVE_SOURCE" -DWAVE_PREFIX="$WAVE_PREFIX"
cmake --build /opt/posim/rendering-compat/test-build -j "${POSIM_WAVE_JOBS:-2}"

# Preserve corresponding source, compatibility edits, original notices and the
# recipe alongside binaries. Distribution must also preserve dependency notices.
mkdir -p "$WAVE_PREFIX/share/posim-waves/surface"
cp "$EXTRAS_DIR/surface/dependency.json" "$EXTRAS_DIR/surface/patch_external_waves.py" \
    "$WAVE_PREFIX/share/posim-waves/surface/"
cp "$EXTRAS_DIR/install-waves.sh" "$WAVE_PREFIX/share/posim-waves/"
cp -r "$EXTRAS_DIR/rendering_compat" "$WAVE_PREFIX/share/posim-waves/"
cp "$WAVE_SOURCE/LICENSE" "$WAVE_PREFIX/share/posim-waves/LICENSE"
sha256sum "$WAVE_PREFIX/lib/libgz-waves1-rendering-ogre2.so.1.0.0" \
    > "$WAVE_PREFIX/share/posim-waves/library.sha256"
printf '%s\n' "POSIM build compatibility edits applied on $(date -u +%Y-%m-%d)." \
    "Original source revision: $WAVE_COMMIT; patched source retained at $WAVE_SOURCE." \
    > "$WAVE_SOURCE/POSIM_BUILD_CHANGES.txt"
printf '%s\n' "$RECIPE" > "$WAVE_PREFIX/.posim-build"
