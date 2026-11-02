# Pre-merge Ogre lifetime overlay (macOS arm64)

This **opt-in** helper applies the material/datablock lifetime correction proposed
in [Ogre-Next #598](https://github.com/OGRECave/ogre-next/pull/598) before upstream
merges it. It builds a separate Ogre core and selects it for one child process.
It does not overwrite a Homebrew library, conda package, desktop engine, shell
profile, or dependency manifest, and it never runs a global install command.

## Supported profile

- macOS arm64; a conda-based POSIM engine containing Ogre-Next **2.3.3**.
- Upstream source: `8d4daeaf46d7d8f85f1833f17daedd7dac05daec` (`v2.3.3`).
- Installed `OgreBuildSettings.h` SHA256:
  `b20e7576609f3743d0109e41db3c905b2864baf41174266d061a8493e6a1a0a8`.
- The generated build settings must match the installed file byte-for-byte.
  This is a compatibility guard, not a proof of universal ABI compatibility.
- The build retains the installed core name `libOgreNextMain.2.3.3.dylib`,
  preserves the installed rendering plugins, and builds only `OgreNextMain`.
- The helper also carries the existing [Xcode compatibility correction](https://github.com/scpeters/ogre-next/commit/b7439ae047489aa104a6775a99a9e93294c3d5b5)
  for current Apple SDKs. It disables legacy Apple UTFString support as that
  upstream correction does. It is not presented as a bit-identical rebuild of
  the distributed conda binary. No renderer performance claim is made.

This is **not** the older Homebrew 2.3.1 overlay, a generic Linux/Windows
installer, or a replacement with Ogre master. Unsupported configurations fail
closed. Ubuntu's vendor package and desktop packages need their own matching
profiles and validation; do not copy this dylib into those environments.

## Build outside the existing installation

Use Python 3.11+, Git, CMake, the Apple compiler/toolchain, and the dependencies
already present in the engine. Select an existing engine explicitly; the helper
does not install missing dependencies or run `sudo`.

```bash
export ENGINE_PREFIX="/absolute/path/to/existing/engine"
export WORK="/absolute/path/to/new/ogre-review-work"
mkdir -p "$WORK"
git clone --filter=blob:none --no-checkout --depth 1 --branch v2.3.3 \
  https://github.com/OGRECave/ogre-next.git "$WORK/ogre-source"
git -C "$WORK/ogre-source" sparse-checkout init --cone
git -C "$WORK/ogre-source" sparse-checkout set OgreMain CMake
git -C "$WORK/ogre-source" checkout --detach \
  8d4daeaf46d7d8f85f1833f17daedd7dac05daec
python3 extras/ogre/build_overlay.py \
  --engine-prefix "$ENGINE_PREFIX" \
  --source "$WORK/ogre-source" --output "$WORK/overlay" --jobs 2
```

The source checkout must be clean. The output directory must not exist and must
be outside both the source and installed engine. The helper retains the copied
source, upstream notices, compiler adaptation, build logs, library hash and
installed-library hash in the output. It refuses mismatched settings, stops on
build errors/time limits, and bounds parallelism. Do not treat a partial output
without `manifest.json` as a completed build.

## Run a separate process and inspect loading

```bash
# Configure POSIM resource paths / ROS environment for this same engine first.
python3 extras/ogre/run_overlay.py --overlay "$WORK/overlay" --trace-loads -- \
  "$ENGINE_PREFIX/libexec/gz/sim10/gz-sim-server" \
  -r --render-engine-server-api-backend metal /absolute/path/to/world.sdf
```

The launch wrapper checks both installed and overlay hashes before executing.
It preserves caller-supplied component overlay paths **ahead of the installed
engine**, while selecting the patched Ogre first. Empty path entries are
removed and `DYLD_INSERT_LIBRARIES` is not used. It does not source shell
profiles or start a background service. Exiting the process removes the effect.

For combined pre-merge validation, independently build the bridge, Wave Sim and
sonar into a fresh prefix. Put their library paths in `DYLD_LIBRARY_PATH` and
plugin paths in `GZ_SIM_SYSTEM_PLUGIN_PATH` / `GZ_RENDERING_PLUGIN_PATH` before
using the wrapper. Verify **every relevant loaded library path** in the dyld
trace; a successful launch alone does not prove that a patched dependency is
loaded. In particular, an old Wave Sim installation must not take precedence.
The tested packaged renderer also logged a NUL-padded built-in Metal plugin
path error before successfully loading Metal via the explicit resource path.
This also occurred without the overlay; it remains a packaging diagnostic,
not a claim of an error-free log.

System-protected launchers may strip `DYLD_*`; verify the final simulator process,
not just the wrapper's environment.

The retained Mac integration used ROS-Gazebo `c38a35deaaba29380665949872023a5998f24222`
and Wave Sim `ee85b48bdcac5a2737709d6ea8170babd067f015` (the pins proposed in
[POSIM #44](https://github.com/IOES-Lab/POSIM/pull/44)), plus the sonar candidate
from [POSIM #46](https://github.com/IOES-Lab/POSIM/pull/46). Component sources were
built against the same packaged engine. The POSIM Jetty Wave Sim adapter was
retained. A private pkg-config copy with a space-free prefix alias and explicit
Eigen / Apple-compatible Ogre includes were needed for this Mac build; no
installed `.pc` or header file was edited.

## Acceptance and limits

Before relying on the overlay, check depth-camera recreation, live data after
sonar/wave recreation, matched raw/image timestamps, and normal server/bridge
shutdown, with the loaded-library trace retained. A failed check remains a
failure. This does not validate sonar distance accuracy or eliminate the
separate strongest-peak criterion issue.

The retained integration is a bounded Mac/Metal fixture (CPU and WGPU), not a
full desktop/web/ArduPilot acceptance run, a long-duration qualification, or
Linux/Windows support. Upstream merge, packaging approval and automatic adoption
in POSIM's standard installers remain separate decisions.

To stop using the overlay, run the original engine without this wrapper. Keep
working source and research data; the helper provides no automatic cleanup.

```bash
python3 -m unittest discover -s extras/ogre/tests -v
```
