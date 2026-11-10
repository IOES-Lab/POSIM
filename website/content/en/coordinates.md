# Spherical coordinates

The `SphericalCoords` ROS plugin provides services for the world's geographic origin and transformations between local Cartesian positions and latitude/longitude/altitude.

## Start a configured world

Run in [terminal A prepared for your installation](terminals.md):

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_bimanual_example headless:=false
```

In the prepared terminal B:

```bash
ros2 service list -t
ros2 service call /gz/get_origin_spherical_coordinates \
  posim_interfaces/srv/GetOriginSphericalCoord '{}'
```

The example SDF sets latitude 35.074823° and longitude 129.084798°. Read the response for the actual running origin.

## SDF integration and parameters

Place both blocks directly inside `<world>`. The plugin reads the world's spherical-coordinate configuration and has no plugin-specific SDF parameters. Its four ROS service paths are fixed under `/gz/`. If the plugin is attached outside a world or the world has no spherical-coordinate configuration, it logs an error and leaves its services disabled.

```xml
<spherical_coordinates>
  <surface_model>EARTH_WGS84</surface_model>
  <world_frame_orientation>ENU</world_frame_orientation>
  <latitude_deg>35.074823</latitude_deg>
  <longitude_deg>129.084798</longitude_deg>
  <elevation>0</elevation>
  <heading_deg>0</heading_deg>
</spherical_coordinates>
<plugin filename="SphericalCoords"
        name="posim_ros_gz_plugins::SphericalCoords"/>
```

| World SDF field | Meaning | Example |
| --- | --- | --- |
| `surface_model` | Reference ellipsoid | `EARTH_WGS84` |
| `world_frame_orientation` | World axis convention | `ENU` |
| `latitude_deg`, `longitude_deg` | Origin, degrees | 35.074823, 129.084798 |
| `elevation` | Origin altitude, m | 0 |
| `heading_deg` | Heading offset, degrees | 0 |

Keep an explicit origin in your world. See [the source](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc) and [the example world](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world) for the service integration.

## Service reference

| Service suffix under `/gz/` | POSIM service type | Request |
| --- | --- | --- |
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Empty |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Types live in `posim_interfaces/srv`. Angles are degrees; local coordinates and altitude are meters.

Requests must contain finite numbers. Geographic latitude must be within −90° to 90° and longitude within −180° to 180°. An invalid origin update returns `success: false` and preserves the origin. Failed transformations return `NaN` coordinates; check the result before using it in a controller.

## Convert a local position

```bash
ros2 service call /gz/transform_to_spherical_coordinates \
  posim_interfaces/srv/TransformToSphericalCoord \
  '{input: {x: 100.0, y: 200.0, z: 3.0}}'
```

The response contains latitude, longitude and altitude. Use those returned values with the inverse service and compare the resulting `output` to the original vector. This checks your configured origin and frame convention.

## Change the origin

```bash
ros2 service call /gz/set_origin_spherical_coordinates \
  posim_interfaces/srv/SetOriginSphericalCoord \
  '{latitude_deg: 35.074823, longitude_deg: 129.084798, altitude: 0.0}'
```

Changing the origin updates how local positions map to latitude/longitude. Align terrain, world orientation, altitude reference and controllers with that coordinate system.

Gazebo's spherical-coordinate transformations are used internally. Keep the SDF's heading/orientation in mind rather than assuming every world has the same local axes.
