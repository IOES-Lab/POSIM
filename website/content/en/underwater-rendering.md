# Underwater scene effects

POSIM's `posim_gz_rendering` applies water-path haze, RGB absorption, depth-dependent sunlight, seabed caustics and suspended specks to Ogre2 PBR surfaces. The specks resemble marine snow: sparse, animated bright particles seen through water. They are available in the public POSIM source and start **off**.

The Gazebo scene, RGB cameras and RGBD color share this rendering path. Numeric depth and collision geometry remain unchanged. Use [underwater camera](camera.md) for the separate depth-based image attenuation plugin.

## Effects and defaults

| Setting | Appearance | Default |
| --- | --- | --- |
| Water optics | Master switch for the optical effects below | On |
| Haze | Distance-dependent blending along the underwater light path | On |
| RGB absorption | Wavelength-dependent color loss | On |
| Sunlight fading | Weaker directional illumination with depth | On |
| Caustics | Animated patterns of sunlight on submerged surfaces | On |
| Suspended specks / marine snow | Sparse procedural bright specks along the viewing ray | Off |
| Seabed texture | Detail from the seabed's supplied PBR maps | On |
| Seabed grid | Measurement lines, 10 m spacing; adjustable 1–500 m | Off |
| Depth contours | Depth lines, 5 m spacing; adjustable 1–50 m | Off |

Turning water optics off retains the texture, grid and contour selections. Seabed overlays recognize visuals named `seabed` or `shallow`, including Gazebo scoped names. [POSIM-Terrain](libraries.md) supplies sand maps and terrain exports; the renderer does not download textures.

## Build and attach the system

Use a configured Gazebo Jetty/Ogre2 development environment matching the server. From the POSIM checkout, build the system and its rendering helper together:

```bash
cmake -S gazebo/posim_gz_rendering -B build/underwater \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/install/underwater"
cmake --build build/underwater --parallel 2
cmake --install build/underwater
```

CMake needs the Gazebo development packages and the `OGRE-Next` and `OGRE-Next-Hlms` pkg-config entries. On the Ubuntu ROS vendor layout, include `/opt/ros/lyrical/opt/gz_ogre_next_vendor/lib/pkgconfig` in `PKG_CONFIG_PATH`. Keep libraries, headers and shaders from the same build.

Add this plugin inside your SDF `<world>`. Replace `/absolute/underwater` with the installation prefix above. Load one instance per Gazebo process and give each running session its own `session_nonce`.

```xml
<plugin filename="/absolute/underwater/lib/posim_underwater.so"
        name="posim::Underwater">
  <renderer_library>/absolute/underwater/lib/posim_underwater_render.so</renderer_library>
  <shader_directory>/absolute/underwater/share/posim/underwater</shader_directory>
  <session_nonce>appearance_example</session_nonce>
</plugin>
```

macOS libraries use `.dylib`. A rendering scene and active camera are needed, including for headless image output. When also using `UnderwaterCamera`, set `POSIM_UNDERWATER_PLUGIN` to the loaded system's path in the server environment. That flag makes the compatibility camera output pass through the native-colored pixels, avoiding a second attenuation stage. Set it only with a successfully installed native system.

## Turn on marine snow

Start the world, then read the applied settings:

```bash
gz service -s /posim/appearance/appearance_example/state \
  --reqtype gz.msgs.Empty --reptype gz.msgs.Double_V --timeout 3000 --req ''
```

The first 13 values use this order. Indices are zero-based.

| Index | Value |
| --- | --- |
| 0–1 | Schema `1`, revision |
| 2–5 | Water optics, seabed texture, grid, contours |
| 6–10 | Haze, absorption, sunlight fading, caustics, specks |
| 11–12 | Grid spacing and contour spacing in meters |

For a freshly started system at revision `0`, this request enables specks while preserving the other defaults:

```bash
gz service -s /posim/appearance/appearance_example/set \
  --reqtype gz.msgs.Double_V --reptype gz.msgs.Double_V --timeout 3000 \
  --req 'data: [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 10, 5]'
```

For subsequent changes, retain the desired settings and advance revision by one. Boolean fields accept `0` or `1`. Repeating the identical request is allowed. Read `/state` again after rendering: it appends `ready`, camera count and `renderer_failed` at indices 13–15. Confirm the requested revision, `ready = 1` and `renderer_failed = 0`; the `/set` reply alone confirms only receipt.

To start with specks enabled, add this optional SDF tag inside the plugin instead of sending the first request:

```xml
<initial_settings>1 0 1 1 0 0 1 1 1 1 1 10 5</initial_settings>
```

WWW-POSIM uses this public renderer and adds its own appearance panel, session controls and preference storage.

## Model limits and verification

Water-path length uses a flat mean-sea-level plane at **z = 0 m**. It does not follow each wave crest. Caustics and specks are visual approximations; they do not model calibrated particle concentrations or optical sensor noise. Local lamps retain direct illumination. Sky rays receive underwater attenuation, while unlit materials and custom ocean-surface shaders retain their own rendering. The underside of a wave surface is therefore a separate surface-material concern.

Compare above-water and submerged views, RGB and RGBD color, and each toggle. Keep pose and lighting fixed for comparisons. Check that depth bytes remain identical, time-driven effects pause with simulation time, and the system reports renderer failures. The [public renderer package](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering) includes settings and RGB/RGBD render fixtures, shader source and build details.
