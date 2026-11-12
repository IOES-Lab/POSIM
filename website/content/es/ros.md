# Terminales y tópicos ROS

El simulador se ejecuta en A; B sirve para consultar datos o enviar órdenes. Ambas terminales deben cargar la misma distribución ROS y el mismo espacio de trabajo POSIM.

## Entender el recorrido de los datos

<div class="docs-flow"><div><strong>Programas y comandos ROS</strong><span>Suscripciones, servicios y órdenes</span></div><div><strong>Interfaz ROS</strong><span>Puente ros_gz o publicador del plugin</span></div><div><strong>Gazebo y plugins POSIM</strong><span>Física, estado y mediciones</span></div></div>

Las mediciones llegan al programa ROS; las órdenes van al simulador. Un tópico Gazebo aparece en ROS cuando lo publica un puente o un plugin con interfaz ROS. El lanzamiento del robot carga su configuración de puentes.

## Preparar A y B

Sigue la [preparación de terminales para tu instalación](terminals.md). Mantén la [primera simulación](quickstart.md) en A y ejecuta lo siguiente en B.

Estos tópicos corresponden a BlueROV2 sin sonar. Con sonar, sustituye `bluerov2` por `bluerov2_heavy_multibeam_sonar`; para el ejemplo de superficie, usa `wamv`.

## Inspeccionar las conexiones

`--no-daemon` consulta ROS directamente, sin depender de la caché del demonio de la CLI.

{{CODE:ros:0}}

`ros2 topic list` enumera tópicos ROS; `gz topic -l`, tópicos Gazebo Transport. Revisa tipos de mensaje, publicadores, suscriptores y QoS antes de conectar un programa.

## Observar una medición

{{CODE:ros:1}}

Aunque la posición cambie poco, deben seguir llegando muestras con nuevas marcas de tiempo. **Ctrl+C en B** detiene el visor; A sigue simulando. Usa `--once` para recibir un solo mensaje. Los nodos que usan tiempo simulado necesitan `use_sim_time:=true` y `/clock`.

## Conectar otro equipo ROS

Utiliza paquetes ROS y mensajes compatibles, el mismo dominio y una red que permita el descubrimiento y los datos DDS. Instala `posim_interfaces` para mensajes específicos de POSIM. Si hay publicadores pero no llegan muestras, consulta [Solución de problemas](troubleshooting.md).

Continúa con [Python](python-control.md), [propulsores y joystick](robot-control.md) o [visualización y grabación](visualization.md).
