# Spherical coordinates

The `SphericalCoords` ROS plugin provides services for the world's geographic origin and transformations between local Cartesian positions and latitude/longitude/altitude.

## Start a configured world

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_bimanual_example headless:=true
```

In a second sourced terminal:

```bash
ros2 service list -t
ros2 service call /gz/get_origin_spherical_coordinates \
  posim_interfaces/srv/GetOriginSphericalCoord '{}'
```

The example SDF sets latitude 35.074823° and longitude 129.084798°. Read the response for the actual running origin.

## Service reference

| Service suffix under `/gz/` | POSIM service type | Request |
| --- | --- | --- |
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Empty |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Types live in `posim_interfaces/srv`. Angles are degrees; local coordinates and altitude are meters.

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
