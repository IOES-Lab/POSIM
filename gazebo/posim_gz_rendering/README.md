# Underwater surface rendering

[한국어](README.ko.md)

This Gazebo Jetty / Ogre2 system applies water-path haze, RGB absorption, depth-dependent sunlight, animated seabed caustics and optional suspended specks while PBR surfaces are shaded. RGB cameras and RGBD color use the same material path. Numeric depth and collision geometry retain their measured values. Gazebo's clipping, solid-color sensor output and terrain-shadow hooks are retained.

The water surface is approximated by the plane z = 0 m. Caustics and specks are inexpensive procedural approximations. Sky rays use water-path attenuation without changing depth measurements. Unlit objects and custom water shaders retain their own appearance. Local lamps keep their direct illumination; water between a surface and a camera still attenuates that surface's light.

## Suspended particles

Specks have positions in world-space 0.25 m cells. A seeded cell determines a
particle's centre and radius. Uniform visual drift moves the field by
`(0.004, 0.008, 0)` metres per second. At the same simulation time, cameras
observe the same particle positions. The fragment shader walks crossed cells
and evaluates each particle's perpendicular distance from the viewing ray.
Scene distance clips the contribution at opaque surfaces.
Grid traversal uses standard 3D DDA; see
[Amanatides and Woo, 1987](https://www.cse.yorku.ca/~amana/research/grid.pdf).

- Cell occupancy: 3% by default.
- Particle radius: 1.5–4 mm.
- Visible ray interval: 0.05–3 m, clipped by the scene and mean water level.
- Work limit: 24 crossed cells per fragment.
- Footprint smoothing uses the viewing-ray pixel derivatives.

The specks are a lightweight optical approximation. Their uniform drift is
separate from fluid simulation and does not change acoustic rays, pressure or
numeric depth. A particle is not a fixed seabed landmark; perception systems
still need to distinguish suspended matter from static surfaces.

Build with CMake against the same Gazebo/Ogre SDK as the server. The system library delays loading its rendering helper until Gazebo creates a rendering scene. Install libraries to `lib` and shader pieces to `share/posim/underwater`.

```xml
<plugin filename="/absolute/lib/posim_underwater.so" name="posim::Underwater">
  <renderer_library>/absolute/lib/posim_underwater_render.so</renderer_library>
  <shader_directory>/absolute/share/posim/underwater</shader_directory>
  <session_nonce>unique_session_id</session_nonce>
</plugin>
```

On macOS the libraries have the `.dylib` suffix. Use one instance per Gazebo process. The renderer extends the engine's PBS listener and restores it during render teardown.

`/posim/appearance/<session_nonce>/set` accepts `gz.msgs.Double_V`. Its 13 values are schema (1), revision, water optics, seabed texture, grid, contours, haze, absorption, sunlight fading, caustics, specks, grid spacing and contour spacing. Boolean values are 0 or 1. Revision advances by exactly one; an identical retry is idempotent. `/state` accepts `gz.msgs.Empty` and returns those 13 values plus ready, camera count and renderer failed. Applied state is acknowledged after rendering.

- Defaults on: optics, texture, haze, absorption, sunlight fading, caustics.
- Defaults off: grid, contours, suspended specks.
- Grid: 10 m, adjustable from 1 to 500 m.
- Contours: 5 m, adjustable from 1 to 50 m.
- Turning water optics off preserves texture and measurement-overlay selections.

Seabed visuals are named `seabed` or `shallow` (including Gazebo scoped names). Terrain exports supply their PBR maps. Texture detail and measurement overlays affect these visuals below sea level. The legacy `UnderwaterCamera` compatibility output passes native-colored pixels through when `POSIM_UNDERWATER_PLUGIN` is set. Hosts should enable this flag only when they install the native system, and surface renderer failures to users.

`underwater-settings-test` covers configuration and water-path boundaries. `underwater-render-test SHADER_DIRECTORY OUTPUT_DIRECTORY SAND_MAP_DIRECTORY` renders RGB and RGBD comparisons and requires byte-identical depth across settings. Run it in a configured Gazebo environment with the sand maps distributed by POSIM-Terrain.

## Native checks

Build the `underwater-settings-test`, `underwater-render-test` and
`underwater-particle-render-test` targets with a matching Gazebo/Ogre SDK.
`ctest` runs the settings check. The rendering programs require a functioning
Metal or headless Ogre2 context and run separately.

```sh
python tests/verify_particles.py \
  --binary build/underwater-particle-render-test \
  --sdk /absolute/matching-sdk \
  --renderer-dir build \
  --shaders shaders \
  --output /absolute/new-evidence-directory
```

The script fixes only cell selection and sub-cell jitter in a diagnostic shader
copy. It retains the production ray traversal, coverage, drift and clipping.
Four known particles are checked from four camera positions and after a time
advance. Their measured image centroids must agree with independent 3D
projection. A nearer opaque surface must hide them. The native fixture also
requires byte-identical numeric depth when specks are switched on and off.
This is a renderer check, separate from SLAM or calibrated water optics.

[Projection controls and retained native-map failure](https://github.com/IOES-Lab/POSIM-SLAM/blob/main/docs/validation/particle-static-correspondence-20261028/README.md)
record measured execution and its acceptance limits.
