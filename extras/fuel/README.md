# Docker Quickstart Fuel cache

Docker builds prepare the external assets used by the 14 headless Quickstart
paths in `extras/ci/quickstarts.tsv`. Previously every fresh container downloaded
these models while Gazebo started. An AMD64 camera trial exceeded its unchanged
90-second entity-readiness budget. A same-image GDB reproduction waited in Fuel's
HTTP downloader while loading a material. In particular, Sunken Vase Distorted
references the **version-3** Sunken Vase textures even when the scene includes
version 4 of Sunken Vase. Downloading only the top-level models is insufficient.

`quickstart-assets.lock.json` pins the 12 required model versions and SHA-256 of
all 105 prepared files. `prepare-image-assets.py` stages bounded downloads during
the image build, verifies every file, and checks nested SDF HTTP dependencies.
Existing corrupt/incomplete caches fail verification. Runtime inventory verifies
the installed lock against the validation source and rehashes the cache without
network access. The original runtime acceptance budgets are unchanged.

The Docker environment uses `GZ_FUEL_CACHE_PATH=/opt/posim_fuel/cache`. Both root
and the ARM64 desktop user can read these assets; the desktop user owns this
cache on ARM64 so other worlds can still download additional models. A caller
who overrides the cache path or masks it with an empty volume must prepare the
assets there. This is not an offline bundle of all 18 worlds, nor a CUDA/WGPU
sonar validation. Native installations are not modified by this Docker change.

The installed lock and this notice are under `/opt/posim_fuel`. Normal Gazebo Fuel
Tools 11 normalization of `model://` references is preserved. No geometry,
textures, scene parameters or physical models were edited. The cache path format
and download behavior follow the [Fuel tools documentation](https://gazebosim.org/api/fuel_tools/11/cmdline.html).

## Third-party asset attribution

These assets retain their own licenses, not POSIM's Apache-2.0 license. The lock
records the exact source URLs, uploaders, model-config author credits, versions,
and license URLs; the original `model.config` files remain beside each model.

| Models | Fuel uploader | License |
| --- | --- | --- |
| North East Down frame; Sand Heightmap; Sunken Vase Distorted; Sunken Vase with Inertia; mossy_cinder_block | hmoyen | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) |
| Coast Water; Waves | OpenRobotics | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) |
| Sunken Vase (versions 3 and 4); Coral01 | Cole | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) |
| Ground Plane; Sun | OpenRobotics | [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) |

To change a version, download into an isolated cache, check the source's license,
inspect all nested dependencies and review the new file hashes. Do not regenerate
hashes silently in a release build. Rebuild and rerun both architectures afterward.
The CI also runs five fresh camera containers with `--network none`, retains their
Docker network configuration and actual image payloads, and requires normal
shutdown. These extra offline trials are reported separately from the 77-trial
connected matrix; no failing observation is replaced by a later pass.
