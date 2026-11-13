# Dynamic-mesh source compatibility patches

This directory is an **opt-in source candidate**, not an enabled POSIM runtime
feature. The normal installers and Dockerfiles do not apply it. The helper
changes only the two source files in the external checkouts supplied by the
caller; it does not build, install, load or replace any library.

Use separate dependency checkouts and a disposable test image. Do not run this
procedure against a working installation or assume that applying the source
patches changes a running Gazebo process. Build and loader integration must be
reviewed before this candidate can become a default image dependency.

## What changes

Both patches address dynamic geometry in the Ogre2 rendering path:

- Read and restore a legacy material only when the active datablock is
  `HLMS_LOW_LEVEL`. A legacy material with the same name must not override an
  active PBS material.
- Deinitialise and reinitialise the Ogre item, then restore the stored material,
  rather than asking forced reinitialisation to find a material by name.
- Unmap a persistent vertex buffer before destroying its backing buffers.

The patches do not change the Terra shader, camera streaming controls, QGC
settings, physics or vehicle control. They do not establish that every material
warning or teardown problem has been resolved.

## Source pins and licences

| Dependency | Revision | Target file | Licence |
| --- | --- | --- | --- |
| [gz-rendering](https://github.com/gazebosim/gz-rendering/tree/574445fd7f614fcb70b6ea847ca8189485fdd09d) (`gz-rendering10_10.0.2`) | `574445fd7f614fcb70b6ea847ca8189485fdd09d` | `ogre2/src/Ogre2DynamicRenderable.cc` | Apache-2.0 |
| [Wave Sim](https://github.com/srmainwaring/asv_wave_sim/tree/ee85b48bdcac5a2737709d6ea8170babd067f015) | `ee85b48bdcac5a2737709d6ea8170babd067f015` | `gz-waves/src/systems/waves/Ogre2DynamicMesh.cc` | GPL-3.0-or-later |

`manifest.json` records the before/after source hashes and each patch hash.
The Wave Sim patch retains the upstream file's licence and attribution; it is
not relicensed under POSIM's Apache licence. `COPYING.WaveSim` contains the GPL
text. Keep the upstream source, notices and licence with any separate Wave Sim
build. This directory contains patch hunks, not a vendored Wave Sim checkout.

## Check and apply

Requirements: Python 3.9 or newer, Git, and fresh external source checkouts at
the pins above. Set these paths to the checkouts you intend to rebuild:

```bash
export RENDERING_SOURCE=/absolute/path/to/separate/gz-rendering
export WAVES_SOURCE=/absolute/path/to/separate/asv_wave_sim

# From the POSIM repository root: validate both components without writing.
python3 extras/rendering_compat/apply.py \
  --rendering-source "$RENDERING_SOURCE" \
  --waves-source "$WAVES_SOURCE"

# Explicitly apply only after both components pass the source checks.
python3 extras/rendering_compat/apply.py \
  --rendering-source "$RENDERING_SOURCE" \
  --waves-source "$WAVES_SOURCE" --apply
```

The helper rejects unknown source contents and mismatched patch/output hashes.
It recognises already-patched files and leaves them untouched. Both components
are checked before either is written. Each replacement is atomic, but this is
not a two-file transaction across power loss. Re-running after interruption is
safe for either of the recognised before/after contents. Preserve any unrelated
local work before preparing the external checkouts.

The existing Wave Sim Jetty build adaptation in
[`../surface/patch_external_waves.py`](../surface/patch_external_waves.py) is
separate. These two C++ diffs neither replace that build adaptation nor qualify
arbitrary Gazebo/Ogre versions. Do not weaken the hash guard to accommodate a
new dependency revision; review the upstream changes first.

## Build and runtime acceptance

Source application alone is not runtime acceptance. In a separate test image:

1. Build both patched sources against the image's installed ROS/Gazebo/Ogre
   dependency versions. Use isolated build directories and staging prefixes;
   do not overwrite the host or an existing development container's ROS files.
2. Configure the candidate image's loader paths explicitly. Verify the actual
   engine and Wave libraries in both Gazebo server and GUI process maps and
   compare their hashes with the rebuilt outputs. Do not rely on build success
   or `LD_LIBRARY_PATH` alone as proof that the correction is loaded.
3. Run the upstream Ogre2 `lidar_visual`, `gpu_rays` and `load_unload`
   integration tests against those same libraries. Inspect test XML, not just
   the test launcher exit code.
4. Open a new desktop session with the required Wave model and library paths.
   Run the documented demos separately, keeping the GUI enabled:
   ```bash
   ros2 launch posim_demos surface_tutorial.launch.py
   # Stop this run before starting the other demo.
   ros2 launch posim_demos underwater_tutorial.launch.py
   ```
5. Check scene rendering, `/clock` progression, sensor output, simulated vehicle
   response and shutdown. Record child-process failures separately from the ROS
   launcher's exit code. Retain periodic Ogre logs: a shared GUI/server log may
   be overwritten, so a clean final file is not proof of a warning-free run.

Existing general integration tests do not cover every material transition.
A targeted material regression and a clean image build/loader recipe remain
necessary before default activation. Software-rendered ARM64 Docker results do
not establish native macOS/Metal, NVIDIA, Windows or AMD64 compatibility, or
video latency/FPS. Residual material and UV/tangent errors must be tracked
separately rather than hidden or counted as resolved by this patch.

## Offline helper checks

```bash
python3 extras/rendering_compat/test_apply.py
```

These tests exercise source-file safety and idempotence, not a GPU or simulator.
The matching CI job makes no runtime-rendering claim.

## Retirement and rollback

When the pinned dependencies contain an equivalent upstream correction, compare
the source and runtime behaviour before advancing the pin and removing its
patch. Unknown source hashes intentionally stop the helper. Roll back a test
by using a fresh unpatched checkout/build or the previous image digest, not by
editing installed package files. Existing environments are not migrated.
