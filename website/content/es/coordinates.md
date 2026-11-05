# Coordenadas esféricas

El plugin ROS `SphericalCoords` gestiona el origen geográfico del mundo y convierte posiciones cartesianas locales en latitud, longitud y altitud, y viceversa.

## Iniciar un mundo configurado

En la [terminal A preparada para tu instalación](terminals.md):

{{CODE:coordinates:0}}

En la terminal B preparada:

{{CODE:coordinates:1}}

El SDF de ejemplo fija latitud **35.074823°** y longitud **129.084798°**. Consulta la respuesta para conocer el origen del mundo en ejecución.

## Integración SDF y parámetros

Coloca ambos bloques directamente dentro de `<world>`:

{{CODE:coordinates:2}}

El plugin lee las coordenadas del mundo y no añade parámetros SDF propios. Sus cuatro servicios tienen rutas fijas bajo `/gz/`. Si se coloca fuera del mundo o falta su configuración geográfica, informa del error y deja los servicios desactivados.

| Campo del mundo | Significado | Ejemplo |
|---|---|---|
| `surface_model` | Elipsoide de referencia | `EARTH_WGS84` |
| `world_frame_orientation` | Convención de ejes | `ENU` |
| `latitude_deg`, `longitude_deg` | Origen, grados | `35.074823`, `129.084798` |
| `elevation` | Altitud del origen, m | `0` |
| `heading_deg` | Desplazamiento de rumbo, grados | `0` |

Define un origen explícito. Consulta el [plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc) y el [mundo de ejemplo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world).

## Servicios

Los tipos pertenecen a `posim_interfaces/srv`. Los ángulos se expresan en grados; las coordenadas locales y la altitud, en metros.

| Sufijo de servicio bajo `/gz/` | Tipo | Solicitud |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Vacía |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Los valores deben ser finitos. La latitud admite −90° a 90° y la longitud −180° a 180°. Un cambio de origen inválido devuelve `success: false` y conserva el origen. Una transformación fallida devuelve coordenadas `NaN`; comprueba el resultado antes de usarlo en un controlador.

## Convertir una posición local

{{CODE:coordinates:3}}

La respuesta contiene latitud, longitud y altitud. Pasa esos valores al servicio inverso y compara `output` con el vector inicial para comprobar origen y convención de ejes.

## Cambiar el origen

{{CODE:coordinates:4}}

El cambio modifica la correspondencia entre posiciones locales y geográficas. Mantén coherentes terreno, orientación del mundo, referencia de altitud y controladores. Las conversiones usan Gazebo; ten en cuenta el rumbo y la orientación configurados en el SDF.
