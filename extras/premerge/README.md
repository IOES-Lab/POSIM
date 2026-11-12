# Pre-merge camera and Terra backports

This bundle makes two upstream corrections available in **new POSIM builds**
without waiting for upstream release packages. It does not merge the upstream
PRs, update running containers, or change an already-published Docker image.

## Scope and provenance

| Dependency | Reviewed change | POSIM application |
| --- | --- | --- |
| ArduPilot Gazebo | [PR 192](https://github.com/ArduPilot/ardupilot_gazebo/pull/192), head `b3303144c947ec2c06afe63bcf36a38b8d59d652`: repeated video ON requests must not consume active frames; optional x264 keyframe interval | Backport onto POSIM's existing `082a0fe231f6e63bc8d1598f1cba461d9e2ea7f5` pin, not a wholesale dependency update |
| ArduPilot Gazebo | Already-merged [PR 188](https://github.com/ArduPilot/ardupilot_gazebo/pull/188), merge `ad27a8344c2b57489211a58bcef6644e35f167e3`: single streaming-thread guard | Prerequisite adapted for the same older POSIM pin |
| Gazebo Rendering | [PR 1419](https://github.com/gazebosim/gz-rendering/pull/1419), head `92a22902ecf1bc6a7806727bbd56bddf7b4221e1`: guard `detailCol0` when a heightmap has no detail texture | Copy installed rendering resources to `/opt/posim/terra`, patch one shader piece, select the copy with `GZ_RENDERING_RESOURCE_PATH` |

The existing H.264 SPS/PPS `config-interval=-1` correction remains in place.
The QGC executable, launcher cache isolation, bitrate, preset, camera update
rate, and default x264 keyframe interval (**10 frames**) are unchanged.
No unrelated VA-API, RTMP, parser, flight-control, CUDA, or WGPU changes are
imported from newer ArduPilot Gazebo revisions.

The optional camera setting is `<keyframe_interval>2</keyframe_interval>` under
`GstCameraPlugin`, for x264 only (valid range 1–600; invalid values retain 10).
More frequent keyframes can help a receiver join a low-FPS stream, at a
bandwidth/quality cost. This is not a guarantee of a particular startup time
or FPS. POSIM does **not** enable it globally, or change QGC's latency setting.

Third-party code retains its upstream copyright and license terms. The small
bundled diffs are adaptations, not new POSIM implementations of those plugins.

## Build routes

- ARM64 Docker: the existing `--patch-gst-camera` step applies the reviewed
  source changes before compiling `GstCameraPlugin`; a new resource-copy step
  installs the Terra overlay. The image exports its path.
- Ubuntu installer and AMD64 Docker: `ros-lyrical-gz-jetty-install.sh` copies
  the companion bundle (or downloads it for the standalone installer), calls
  the same camera helper, and creates the same Terra overlay. The Ubuntu shell
  environment and AMD64 Docker image export the overlay path.
- Standalone use of `ardusub-ubuntu-install.sh` requires its adjacent
  `premerge/` directory from the **same POSIM revision**. Do not download just
  that helper script. The documented top-level installer fetches its companions.
- Native macOS/Windows installs and the legacy user-local ArduSub helper are
  not automatically altered by this bundle. macOS Docker uses the ARM64 route.

Existing ROS-managed shader files are never edited. The camera patch is applied
only in the dependency **source checkout being built**, not to a preinstalled
library. To preserve an existing environment, build a separately tagged image
or use a new source checkout and build directory.

## Safety and reproducibility

`manifest.json` records the dependency revision, upstream PR heads, patch
SHA256s, and accepted before/after file hashes. The helper checks these before
writing, tests patches in staging, and refuses unknown source content. It
accepts pristine pinned camera source, that source with the existing SPS/PPS
fix, or the fully patched result. Repeating the operation is a no-op.

The Terra overlay contains the full installed rendering resource tree. Its
`POSIM_BACKPORT.json` inventories the source and copied files; only the reviewed
shader may differ. A modified/unmanaged existing output is refused rather
than overwritten. Source and output directories may not overlap.

```bash
python3 extras/premerge/test_apply.py
bash -n extras/ardusub-ubuntu-install.sh
bash -n extras/ros-lyrical-gz-jetty-install.sh

# Run only in a separate dependency checkout that you intend to rebuild.
bash extras/ardusub-ubuntu-install.sh --patch-gst-camera /path/to/ardupilot_gazebo
# Then build that checkout using the normal ROS/Gazebo environment.

# Example isolated resource copy, without modifying the installed ROS resources.
python3 extras/premerge/apply.py terrain \
  --source /opt/ros/lyrical/opt/gz_rendering_vendor/share/gz/gz-rendering \
  --output /absolute/new/terra-overlay
export GZ_RENDERING_RESOURCE_PATH=/absolute/new/terra-overlay
```

Unknown hashes are a deliberate build failure: inspect the new upstream
revision and update/remove the backport rather than weakening the guard.

Gazebo Ogre2 reads `GZ_RENDERING_RESOURCE_PATH` as one directory. Select the
complete Terra resource copy for this session.

## Runtime acceptance checks

1. Run the documented BlueROV2 example and enable QGC video. Confirm the loaded
   `libGstCameraPlugin.so` is the rebuilt library, not a stale overlay.
2. With video active, repeat absolute ON commands. Compare camera frames and
   RTP frame markers over the same interval; ensure no extra pipeline starts.
3. Leave the GUI open, disable/re-enable streaming and check recovery. Record
   default interval 10 separately from any opt-in interval-2 test.
4. Open untextured, single-texture, and blended heightmaps, then an existing
   POSIM textured scene. Check the Ogre log for the `/opt/posim/terra` resource
   path and shader errors, simulation clock progress, and normal GUI shutdown.
   A launcher exit code alone can hide a crashed GUI child.
5. Verify the original ROS shader hash and QGC launcher remain unchanged.

Software-rendered ARM64 Docker tests do not establish native Metal, NVIDIA,
Windows, AMD64 or physical-sensor performance. See the integration PR for the
exact tested image and measured results; a derived-image check is not a full
clean multi-architecture image build.

## Upstream retirement and rollback

When a released/pinned dependency contains the fixes, verify equivalence,
advance the relevant dependency pin, then remove that dependency's patch chain
and build hook together. Do not stack a stale patch onto a newer file.
The helper recognizes the exact corrected Terra shader already in the manifest
and copies it without reapplying the diff; other revisions still need review.

Rollback uses the previous image tag/digest or a fresh unpatched dependency
build. To bypass only the Terra overlay for diagnosis, start a process without
`GZ_RENDERING_RESOURCE_PATH` or select a complete alternate resource directory.
No package-file restoration is needed. Existing environments are not migrated.
