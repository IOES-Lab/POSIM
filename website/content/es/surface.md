# Vehículos de superficie y oleaje

Simula un WAM-V con oleaje de Wave Sim y control ArduRover en modo embarcación. El instalador POSIM y sus recetas Docker compilan las bibliotecas externas de oleaje.

## Tutorial estándar

La instalación estándar incluye un recorrido de boyas con WAM-V, cámara, lidar y una vista de RViz configurada. No necesita la imagen opcional de abajo. En la [terminal de POSIM preparada](terminals.md), ejecuta:

{{CODE:quickstart:4}}

Sigue [Primera simulación](quickstart.md) para la escena, la visualización del lidar y el vídeo y control con QGroundControl.

## Preparar el WAM-V

Esta imagen opcional usa la distribución de escritorio ARM64. En un Mac con Apple Silicon, prepara [Docker y RDP](install-macos.md). Desde una copia del repositorio POSIM, ejecuta en la Terminal de macOS:

{{CODE:surface:0}}

Conecta por RDP a **127.0.0.1:3390** con la cuenta `docker`. Abre una terminal dentro del escritorio Linux:

{{CODE:surface:1}}

La imagen incluye binarios separados de ArduSub y ArduRover. El ejemplo selecciona ArduRover con `FRAME_CLASS=2`. Ejecuta **una instancia por entorno**: nombre del vehículo, archivos generados y puertos del controlador son fijos. Para varios vehículos, separa archivos, nombres, identidades y puertos, o utiliza entornos aislados.

El controlador espera a que aparezca el vehículo y publique los datos de su IMU. Si la creación falla o supera 180 s, o no llega una medida IMU durante los 180 s siguientes, el lanzamiento termina con error e indica la etapa. Revisa los mensajes Gazebo y del sensor antes de repetirlo.

## Modelo, control y navegación

`extras/surface/wamv.py` prepara el modelo externo. Las hélices de popa usan las salidas 1 y 3; el vehículo empieza en la superficie.

El adaptador de navegación externa envía **posición, orientación y velocidad simuladas de Gazebo** a MAVROS. Comprueba estado del controlador, navegación válida y sentido de propulsión antes de enviar una misión.

Para estudiar ruido, contacto con el agua o readquisición del receptor, integra el [GNSS marino](gnss.md), su antena y un adaptador de navegación adecuado. El adaptador de pose externa de este ejemplo es una entrada distinta.

## Ajustar las olas

Los controles usan `gz.msgs.Param` en `/world/<world_name>/waves`. El nombre procede de `<world name="…">`. Lista los tópicos activos:

{{CODE:surface:2}}

El [lanzador WAM-V](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py) usa `posim_surface` por defecto: `/world/posim_surface/waves`. Puedes cambiar el nombre dentro del contenedor preparado:

{{CODE:surface:3}}

Así, el tópico es `/world/harbour/waves`; la IMU ROS permanece en `/model/wamv/imu`. `world_name` nombra el mundo generado: no selecciona un archivo de terreno ni aísla otra instancia.

El fondo es plano y está a z = −10 m. Para modificarlo, consulta [Terrenos](heightmaps.md).

| Parámetro (`double`) | Unidad | Función |
|---|---|---|
| `wind_speed` | m/s | Velocidad del viento del modelo de oleaje |
| `wind_angle` | grados | Dirección del viento |
| `steepness` | sin unidad | Pendiente de la ola |

Mantén coherentes los ajustes visuales e hidrodinámicos. Las [corrientes](currents.md) se configuran por separado. El mundo debe incluir y configurar los sistemas de oleaje, además de tener sus bibliotecas instaladas.

## Límites y dependencias

El Dockerfile opcional admite la estructura ARM64. Calibra casco, propulsión y resistencia para la embarcación y sus condiciones. Al cerrar, MAVROS o Gazebo pueden requerir que el supervisor los termine; confirma que ambos han salido antes de reiniciar.

`extras/surface/dependency.json` fija la revisión externa. El instalador compila en `/opt/waves` y conserva el código en `/opt/asv_wave_sim`; es una dependencia de compilación, no un submódulo Git. Consulta la [guía de integración](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) y las [licencias](citation.md).
