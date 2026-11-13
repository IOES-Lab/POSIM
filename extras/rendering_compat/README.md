# Dynamic-mesh source compatibility patches

The Ubuntu/WSL installer and both Docker builds apply these pinned corrections
through [`../install-waves.sh`](../install-waves.sh). The rebuilt gz-rendering
core and Ogre2 engine live in `/opt/posim/rendering-compat`; ROS-owned files
remain unchanged. Wave Sim is rebuilt from its retained source in
`/opt/asv_wave_sim` and installed under `/opt/waves`.

The build requires gz-rendering **10.0.2** and Ogre-Next **2.3.3**. It rejects
other versions and unknown source bytes. The renderer archive is checked by
SHA-256, and its source and build recipe remain beside the installed libraries.
The POSIM environment selects this renderer after sourcing ROS underlays.
The RDP desktop restores the same paths before starting XFCE.

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

On the supported Ubuntu 26.04 / ROS Lyrical stack, run the normal installer or
build either POSIM Dockerfile. The helper builds the renderer against the
installed dependencies and builds the upstream Ogre2 `lidar_visual`, `gpu_rays`
and `load_unload` tests. It also builds material regressions for gz-rendering
and Wave Sim covering a PBS/legacy name collision, buffer growth and shrinkage,
legacy material restoration, return to PBS, and destruction.

After sourcing the installed POSIM workspace, run:

```bash
bash extras/rendering_compat/test.sh /tmp/posim-rendering-results
```

The checks verify the loaded rendering library paths and hashes, reject failed
or skipped upstream tests, and preserve logs and XML results. Both architecture
image tests run this command. The helper does not repair the caller's environment
to make a missing overlay pass.

Run the surface and underwater tutorials separately with their GUIs enabled.
Check scene rendering, camera and lidar output, clock progression, vehicle
response, QGC video and shutdown. Inspect both server and GUI library mappings;
a successful build alone does not prove that a running process uses it.
Keep validation evidence outside the product repository.

These corrections do not remove every Ogre material, UV/tangent or shutdown
warning. Software-rendered Docker tests do not qualify native macOS/Metal,
Windows graphics drivers or video latency/FPS. Apple Silicon uses the documented
Linux ARM64 Docker/RDP installation; Windows uses Ubuntu under WSL 2.

## Offline helper checks

```bash
python3 extras/rendering_compat/test_apply.py
```

These tests exercise source-file safety and idempotence, not a GPU or simulator.
The source-safety CI job makes no runtime-rendering claim; image tests provide
the separate runtime checks.

## Retirement and rollback

When the pinned dependencies contain an equivalent upstream correction, compare
the source and runtime behaviour before advancing the pin and removing its
patch. Unknown source hashes intentionally stop the helper.

Use the previous qualified image digest to roll back the complete stack. Keep
its source and test evidence. Do not copy individual libraries between ROS,
Ogre or architecture versions. Existing containers are not migrated by a new
image build.
