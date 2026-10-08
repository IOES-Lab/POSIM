# Heightmap terrain

A heightmap represents terrain as a regular grid of elevations. Configure visual and collision geometry together so the seabed seen by a camera agrees with the surface used for contact.

## Example terrain

Use [Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) for a geographic scene and [graded seabed](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world) for a seabed scene. Santorini uses the Fuel model `Santorini Scaled`.

These examples show world composition. For heightmap geometry itself, follow Gazebo's [heightmap tutorial](https://gazebosim.org/api/sim/10/heightmap_dem.html) and the [SDFormat geometry reference](http://sdformat.org/spec?ver=1.12&elem=geometry).

## Keep terrain assets inside the package

Place a new world in `models/posim_worlds/worlds` and its data under `models/posim_worlds/media`. Inspect the package's CMake install rules and resource hook. Use installed, portable URIs rather than an absolute developer path.

The geometry inside a visual or collision takes this general form:

```xml
<geometry>
  <heightmap>
    <uri>model://media/meshes/my_heightmap.png</uri>
    <size>1000 1000 80</size>
    <pos>0 0 -80</pos>
  </heightmap>
</geometry>
```

This fragment is an illustration for your own asset. Select a raster format and sampling supported by the renderer and physics backend. Check the elevation encoding before choosing the vertical size and offset.

## Preserve coordinates and scale

`size` gives horizontal extent and elevation scale in meters; `pos` supplies an offset. A normalized image is not automatically a georeferenced elevation raster. Record source extent, elevation range, sea-level reference and any reprojection or resampling.

Set the world origin with `<spherical_coordinates>` when geographic positions matter. See [Spherical coordinates](coordinates.md) to transform between latitude/longitude and local coordinates.

## Build and verify

Rebuild and source the workspace after adding resources. Launch the new world from outside the checkout. Check recognizable elevation landmarks, the visual/collision alignment, sensor depth and a controlled contact probe. Confirm the intended sea level and robot start height before using the terrain in navigation experiments.
