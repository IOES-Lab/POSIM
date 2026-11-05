# Sensor de velocidad Doppler (DVL)

Un DVL estima la velocidad respecto al fondo o al agua circundante. POSIM combina el sistema DVL de Gazebo con un puente ROS que genera `posim_interfaces/msg/DVL`.

## Ejecutar el sensor

En la [terminal A preparada para tu instalación](terminals.md):

{{CODE:dvl:0}}

En la terminal B preparada, consulta Gazebo, detén esa consulta con **Ctrl+C** y después consulta ROS:

{{CODE:dvl:1}}

Si creas otro sensor, usa el topic definido en su modelo.

## Integración SDF

Añade estos sistemas directamente en `<world>`:

{{CODE:dvl:2}}

El puente POSIM va dentro del `<model>` del sensor. Su topic debe coincidir con el `<topic>` del DVL situado en `<link>`:

{{CODE:dvl:3}}

El [SDF completo del DVL500-300](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf) define un sensor `type="custom" gz:type="dvl"`, cuatro haces y los modos de seguimiento. El [mundo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world) aporta la geometría del fondo.

## Parámetros y unidades

`DVLBridge` acepta un único parámetro: `topic`, predeterminado `/dvl/velocity`. Recibe Gazebo y publica ROS en esa ruta. Los demás campos pertenecen al sensor Gazebo; la tabla muestra el ejemplo DVL500-300.

| Campo SDF del sensor | Significado | Ejemplo |
|---|---|---|
| `topic`, `update_rate` | Ruta y frecuencia solicitada, Hz | `/dvl/velocity`, 8 |
| `gz:dvl/type` | Disposición de transductores | `phased_array` |
| `arrangement`: `aperture`, `rotation`, `tilt` | Ángulos de haz; `degrees="true"` usa grados | Apertura 3°, giros −135/135/45/−45°, inclinación 25° |
| `minimum_range`, `maximum_range` | Alcance acústico, m | 0.3, 200 |
| `reference_frame` | Posición en m y orientación en rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | Selección de seguimiento | Ambos `best` |
| Seguimiento: `noise/stddev` | Desviación estándar de velocidad, m/s | Fondo 0.005; agua 0.015 |
| Masa de agua: `water_velocity` | Componentes x/y/z, m/s | 0/0/0 |
| Masa de agua: `boundaries/near`, `boundaries/far`, `bins` | Límites de muestreo en m y número de celdas | 10, 100, 10 |
| Seguimiento: `visualize` | Mostrar geometría de muestreo | true |

La [implementación del puente](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc) describe la conversión. El sensor de profundidad necesita el renderizado de Gazebo: completa la [configuración del entorno](requirements.md).

## Modelos disponibles

| Descripción | Familia |
|---|---|
| `nortek_dvl500_300`, `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300`, `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000`, `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL con [sonar ](sonar-tuning.md) |

Revisa el muestreo y la geometría de cada SDF.

## Referencia y covarianza

El ejemplo transforma ENU a su referencia configurada de avance/estribor/abajo mediante `<reference_frame>`. El puente conserva el nombre de referencia Gazebo, cambiando `::` por `/`. Usa la misma referencia en el estimador y distingue velocidad respecto al fondo y al agua.

La velocidad conjunta y cada haz incluyen una covarianza ROS de movimiento 6×6. El bloque superior izquierdo 3×3 contiene la covarianza de velocidad lineal, en m²/s². Las demás entradas son cero porque no hay estimación de velocidad angular; no representan incertidumbre medida nula.

## Examinar observaciones

Consulta `ros2 interface show posim_interfaces/msg/DVL`. Comprueba velocidad, referencia y estado con el robot quieto y después en movimiento controlado. El fondo, el alcance y las intersecciones de los haces influyen en la medición. Activa `visualize` en el modelo para inspeccionar los haces.

## Haces y salida DVL

{{MEDIA:dvl}}

Imágenes: wiki Notion de POSIM; atribución DAVE en [Citas y licencias](citation.md). Usa los comandos y topics de esta página.
