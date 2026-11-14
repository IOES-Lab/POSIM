#!/usr/bin/env bash
# Backport the TypeLookup shutdown ordering fix without replacing ROS files.
set -eo pipefail
root=$(cd -- "$(dirname -- "$0")" && pwd)
prefix=/opt/posim/fastdds
if [[ -e "$prefix" ]]; then
    echo "Refusing an existing Fast DDS overlay: $prefix" >&2
    exit 1
fi
# shellcheck source=/dev/null
source /opt/ros/lyrical/setup.bash
version=$(dpkg-query -W -f='${Version}' ros-lyrical-fastdds)
if [[ "$version" != 3.6.2-* ]]; then
    echo "Fast DDS backport requires 3.6.2; found $version. Review upstream first." >&2
    exit 1
fi
mkdir -p "$prefix/src" "$prefix/lib" "$prefix/recipe"
curl --fail --location --connect-timeout 20 --max-time 300 --retry 3 \
    https://codeload.github.com/eProsima/Fast-DDS/tar.gz/refs/tags/v3.6.2 \
    -o "$prefix/source.tar.gz"
echo "7ae3e55067a5337da98bc7997ec476fe04218c655457c10d4fbf74576d83f221  $prefix/source.tar.gz" | sha256sum -c -
tar -xzf "$prefix/source.tar.gz" -C "$prefix/src"
source_dir="$prefix/src/Fast-DDS-3.6.2"
git -C "$source_dir" apply --check "$root/stop-request-before-history.patch"
git -C "$source_dir" apply "$root/stop-request-before-history.patch"
cmake -S "$source_dir" -B "$prefix/build" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix" \
    -DBUILD_TESTING=OFF -DCOMPILE_EXAMPLES=OFF -DSECURITY=ON
cmake --build "$prefix/build" --target fastdds --parallel 2
# Only the ABI-compatible runtime library is selected; ROS owns the headers/tools.
install -m 755 "$prefix/build/src/cpp/libfastdds.so.3.6.2.0" "$prefix/lib/"
ln -s libfastdds.so.3.6.2.0 "$prefix/lib/libfastdds.so.3.6"
ln -s libfastdds.so.3.6 "$prefix/lib/libfastdds.so"
cp "$root/"{build.sh,stop-request-before-history.patch,README.md} "$prefix/recipe/"
cp "$source_dir/LICENSE" "$prefix/recipe/LICENSE.Fast-DDS"
sha256sum /opt/ros/lyrical/lib/libfastdds.so.3.6.2.0 > "$prefix/underlay.sha256"
sha256sum "$prefix/lib/libfastdds.so.3.6.2.0" > "$prefix/library.sha256"
