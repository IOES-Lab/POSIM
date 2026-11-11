# Funciones y ejemplos

POSIM ofrece sistemas Gazebo, modelos de robots e interfaces ROS. El SDF del mundo y los archivos de lanzamiento seleccionan los sistemas de cada simulación.

## Entorno y representación visual

| Función | Implementación | Guía |
|---|---|---|
| Turbidez, absorción RGB y atenuación de la luz solar | `gazebo/posim_gz_rendering` | [Efectos submarinos](underwater-rendering.md) |
| Cáusticas del fondo y partículas en suspensión | `gazebo/posim_gz_rendering` | [Activación de efectos](underwater-rendering.md) |
| Textura del fondo, cuadrícula y curvas de profundidad | `gazebo/posim_gz_rendering`; mapas del terreno | [Materiales y espaciado](underwater-rendering.md) |
| Oleaje espectral e hidrodinámica del WAM-V | Wave Sim externo; integración `extras/surface` | [Vehículos de superficie](surface.md) |
| Corrientes uniformes, por profundidad y de marea | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [Corrientes](currents.md) |
| Origen geográfico y conversión de coordenadas | `posim_ros_gz_plugins` | [Coordenadas esféricas](coordinates.md) |
| Fondos, objetos y escenarios de tareas | `models/posim_worlds`, `posim_object_models` | [Mundos](worlds.md), [objetos](objects.md), [terrenos](heightmaps.md) |

Los efectos ópticos submarinos modifican las superficies PBR y conservan la profundidad numérica. La superficie oceánica usa su propio material. Las partículas marinas son una aproximación visual procedural y están desactivadas inicialmente. La [guía de representación](underwater-rendering.md) explica el nivel del mar y el comportamiento de las cámaras.

## Sensores, robots y control

| Función | Implementación | Guía |
|---|---|---|
| GNSS marino, pérdida por inmersión, readquisición y latencia | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| Atenuación de imágenes submarinas con RGBD | `posim_gz_sensor_plugins/UnderwaterCamera` | [Cámara](camera.md) |
| Mediciones DVL y conversión a ROS | DVL de Gazebo; `posim_ros_gz_plugins` | [DVL](dvl.md) |
| Presión hidrostática y profundidad calculada | `posim_gz_sensor_plugins` | [Presión](pressure.md) |
| Interrogación acústica y posicionamiento USBL | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| Sonar multihaz con CUDA | `gazebo/posim_gz_multibeam_sonar` | [Sonar](sonar.md), [configuración y rendimiento](sonar-tuning.md) |
| REXROV, variantes de BlueROV2 y planeador Slocum | `models/posim_robot_models` | [ROV](rovs.md), [planeador](gliders.md), [añadir un robot](custom-robots.md) |
| WAM-V con ArduRover y control ROS | `extras/surface`, configuración de lanzamiento, `posim_interfaces` | [Vehículo de superficie](surface.md), [ROS 2](ros.md) |

Los sensores disponibles dependen de los plugins instalados y del modelo. Los perfiles GNSS describen el montaje: hay que incorporar el eslabón y el receptor. El sonar CUDA requiere GPU y herramientas compatibles. Usa el inicio rápido de cada referencia para comprobar las mediciones durante la simulación.

## Directorios del código fuente

Las rutas son relativas al repositorio POSIM.

| Directorio | Contenido |
|---|---|
| `models/posim_worlds` | Mundos y recursos del entorno |
| `models/posim_robot_models` | Mecánica, sensores y configuración ROS de cada robot |
| `models/posim_sensor_models` | Descripciones y ejemplos de sensores independientes |
| `gazebo/` | Plugins del entorno marino y sensores |
| `posim_interfaces` | Definiciones de mensajes y servicios POSIM |
| `examples/posim_demos` | Lanzamientos que conectan mundos, modelos y ROS |

Ejecuta estos componentes con [Primera simulación](quickstart.md). Para modificarlos, consulta [Añadir un robot](custom-robots.md) y [Crear un mundo](custom-worlds.md).

## Biblioteca Ogre alternativa para macOS

La [herramienta de compilación de Ogre](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) crea un núcleo Ogre aislado para macOS arm64 con Ogre-Next 2.3.3. Sigue el perfil SDK compatible y comprueba las bibliotecas cargadas según esa guía.

Consulta las [licencias de los componentes](citation.md).
