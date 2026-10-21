# Underwater surface rendering

[한국어](README.ko.md)

This Gazebo Jetty / Ogre2 system applies water-path haze, RGB absorption, depth-dependent sunlight, animated seabed caustics and optional suspended specks while PBR surfaces are shaded. RGB cameras and RGBD color use the same material path. Numeric depth and collision geometry retain their measured values. Gazebo's clipping, solid-color sensor output and terrain-shadow hooks are retained.

The water surface is approximated by the plane z = 0 m. Caustics and specks are inexpensive procedural approximations. Unlit objects, custom water shaders and the sky are outside the PBR effect. Local lamps keep their direct illumination; water between a surface and a camera still attenuates that surface's light.

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
- Grid: 50 m, adjustable from 1 to 500 m.
- Contours: 5 m, adjustable from 1 to 50 m.
- Turning water optics off preserves texture and measurement-overlay selections.

Seabed visuals are named `seabed` or `shallow` (including Gazebo scoped names). Terrain exports supply their PBR maps. Texture detail and measurement overlays affect these visuals below sea level. The legacy `UnderwaterCamera` compatibility output passes native-colored pixels through when `POSIM_UNDERWATER_PLUGIN` is set. Hosts should enable this flag only when they install the native system, and surface renderer failures to users.

`underwater-settings-test` covers configuration and water-path boundaries. `underwater-render-test SHADER_DIRECTORY OUTPUT_DIRECTORY SAND_MAP_DIRECTORY` renders RGB and RGBD comparisons and requires byte-identical depth across settings. Run it in a configured Gazebo environment with the sand maps distributed by POSIM-Terrain.
