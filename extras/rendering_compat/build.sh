#!/usr/bin/env bash
# Build the pinned Ogre2 engine separately from ROS-owned libraries.
set -eo pipefail
if [[ $# != 1 ]]; then echo "Usage: $0 WAVE_SOURCE" >&2; exit 2; fi
helpers=$(cd -- "$(dirname -- "$0")" && pwd)
waves=$1
prefix=/opt/posim/rendering-compat
vendor=/opt/ros/lyrical/opt/gz_rendering_vendor
# shellcheck source=/dev/null
source /opt/ros/lyrical/setup.bash
export PKG_CONFIG_PATH="/opt/ros/lyrical/opt/gz_ogre_next_vendor/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
if ! grep -q '^set(PACKAGE_VERSION "10.0.2")' \
    "$vendor/lib/cmake/gz-rendering/gz-rendering-config-version.cmake" ||
   [[ $(pkg-config --modversion OGRE-Next) != 2.3.3 ]]; then
    echo 'Rendering compatibility requires gz-rendering 10.0.2 and Ogre-Next 2.3.3.' >&2
    exit 1
fi
revision=574445fd7f614fcb70b6ea847ca8189485fdd09d
mkdir -p "$prefix/src" "$prefix/lib"
sha256sum "$vendor/lib/libgz-rendering.so.10.0.2" \
    "$vendor/lib/libgz-rendering-ogre2.so.10.0.2" > "$prefix/underlay.sha256"
source_dir="$prefix/src/gz-rendering-$revision"
if [[ ! -d "$source_dir" ]]; then
    curl --fail --location --connect-timeout 20 --max-time 300 --retry 3 \
        "https://codeload.github.com/gazebosim/gz-rendering/tar.gz/$revision" \
        -o "$prefix/source.tar.gz"
    echo "870f8420d364517cb251298de745e2297b51bbdf4251bd2a65cd9dafb72fe26f  $prefix/source.tar.gz" | sha256sum -c -
    tar -xzf "$prefix/source.tar.gz" -C "$prefix/src"
fi
python3 "$helpers/apply.py" --rendering-source "$source_dir" --waves-source "$waves" --apply
cmake -S "$source_dir" -B "$prefix/build" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCMAKE_SKIP_RPATH=ON \
    -DGZ_OGRE2_PROJECT_NAME=OGRE-Next -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$prefix/build" -j "${POSIM_WAVE_JOBS:-2}" --target \
    gz-rendering-ogre2 INTEGRATION_lidar_visual INTEGRATION_gpu_rays INTEGRATION_load_unload
# The core's compiled plugin path must select this engine before the ROS engine.
# Keep both rebuilt libraries together; leave every ROS-owned file unchanged.
for library in gz-rendering gz-rendering-ogre2; do
    install -m 755 "$prefix/build/lib/lib$library.so.10.0.2" "$prefix/lib/"
    ln -sfn "lib$library.so.10.0.2" "$prefix/lib/lib$library.so.10"
    ln -sfn "lib$library.so.10" "$prefix/lib/lib$library.so"
done
mkdir -p "$prefix/lib/gz-rendering-10/engine-plugins" "$prefix/share/gz"
ln -sfn ../../libgz-rendering-ogre2.so \
    "$prefix/lib/gz-rendering-10/engine-plugins/libgz-rendering-ogre2.so"
cp -r "$vendor/share/gz/gz-rendering" "$prefix/share/gz/"
if ldd "$prefix/lib/libgz-rendering-ogre2.so.10.0.2" | grep -q 'not found'; then
    echo 'The rebuilt renderer has unresolved dependencies.' >&2
    exit 1
fi
mkdir -p "$prefix/recipe"
cp "$helpers/"{apply.py,manifest.json,gz-rendering.patch,waves.patch,build.sh} "$prefix/recipe/"
sha256sum "$prefix/lib/"*.so.10.0.2 > "$prefix/library.sha256"
