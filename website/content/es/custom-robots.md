# Añadir un robot

Parte de un robot POSIM con una distribución parecida. El SDF describe la mecánica y los sistemas; la configuración ROS conecta medidas y comandos.

## Entorno de trabajo

Usa las [terminales Linux preparadas](terminals.md) en Ubuntu nativo, WSL o Docker. En macOS, trabaja en la terminal del escritorio RDP. Edita y compila allí; `POSIM_WS` es la ruta de ese espacio de trabajo.

## 1. Copiar un modelo de referencia

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) usa propulsores Gazebo. [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) integra ArduSub/MAVROS. [Slocum](gliders.md) muestra otra disposición.

Desde `$POSIM_WS`, crea una copia completa de REXROV para conservar mallas, configuración del puente y nombres de tópicos:

{{CODE:custom-robots:0}}

Guarda el siguiente programa como `copy_robot.py` y ejecuta `python3 copy_robot.py`. Se detiene si ya existe `my_robot`.

{{CODE:custom-robots:1}}

La estructura resultante incluye:

{{CODE:custom-robots:2}}

`my_robot` es el identificador elegido. El lanzamiento busca `description/<namespace>/model.sdf` y su configuración correspondiente.

## 2. Definir mecánica y recursos

Conserva la mecánica copiada para el primer lanzamiento. Al modificar el casco, revisa en conjunto:

| Ruta SDF | Unidad | Comprobación |
|---|---|---|
| `model/@name`, nombres de eslabones y articulaciones | identificador | Coincidir con plugins y configuración ROS |
| `link/inertial/mass` | kg | Masa positiva de cada eslabón móvil |
| `link/inertial/pose` | m, rad | Centro de masa y orientación del sistema de inercia |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | Momentos respecto a sus ejes |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | Términos cruzados; tensor completo físicamente válido |
| `link/visual`, `link/collision` | m, rad | Escala y alineación de geometrías |
| `joint/axis`, ajustes de propulsores | según el plugin | Dirección del actuador y distribución de fuerzas |

STL aporta geometría. Define masa y dinámica en SDF y comprueba unidades y escala de la malla. Sigue las URI instaladas de los ejemplos existentes. Usa una colisión sencilla si el detalle visual encarece innecesariamente el contacto.

## 3. Conectar sistemas y ROS

Adapta hidrodinámica, flotabilidad, ejes de propulsión y poses de sensores al vehículo. Los coeficientes deben representar sus propiedades físicas.

Consulta [Corrientes](currents.md), [Añadir sensores](custom-sensors.md) y las referencias de cada plugin. Si necesitas latitud y longitud, configura el origen con [Coordenadas esféricas](coordinates.md).

Adapta `config/<namespace>/robot_config.py`: cambia de forma coherente nombres de entidades y articulaciones, tópicos, sistemas de referencia y puertos. El paquete ya instala `description`, `meshes`, `config` y los ajustes de búsqueda de recursos. Edita los fuentes, no los archivos generados en `install/`.

## 4. Compilar y probar

Desde el espacio de trabajo con su entorno cargado:

{{CODE:custom-robots:3}}

Prueba cada eje con comandos pequeños. Comprueba también el lanzamiento desde fuera del repositorio:

- Rutas de recursos instalados.
- Masa, inercia y pose de equilibrio.
- Geometría de colisión y respuesta al contacto.
- Sistemas de referencia y contenido de mensajes.

En otra terminal preparada, consulta el modelo renombrado:

{{CODE:custom-robots:4}}

Debes recibir posición y velocidad. Primero haz funcionar la copia, después cambia la mecánica y finalmente sensores o propulsión. Compara ejecuciones con [Datos de sensores](visualization.md) y consulta [Solución de problemas](troubleshooting.md) si faltan recursos.
