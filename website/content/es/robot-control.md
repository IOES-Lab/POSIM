# Propulsores y control con joystick

Elige la interfaz correspondiente al robot. Comprueba primero los sensores y activa un solo controlador a la vez.

| Vehículo | Ruta de control | Guía |
|---|---|---|
| REXROV | Fuerzas ROS → propulsores Gazebo | [Python](python-control.md) |
| BlueROV2 | ROS/joystick → MAVROS → ArduSub | [QGC y vídeo](qgroundcontrol.md) · [ROS/joystick](rovs.md) |
| WAM-V | Integración de superficie y controlador del vehículo | [Robots de superficie](surface.md) |

## Propulsores de REXROV

En las [terminales preparadas](terminals.md), mantén REXROV activo en A y envía las órdenes desde B. Su configuración conecta ocho tópicos `cmd_thrust` con el tipo `std_msgs/msg/Float64`. Los valores son fuerzas en newtons. Para dejar neutro el primer propulsor:

{{CODE:robot-control:0}}

Distribuye las fuerzas según la posición y el eje de cada propulsor. Controlar la velocidad del vehículo exige coordinarlos. Al terminar, envía cero a todos los propulsores utilizados.

## BlueROV2 con ArduSub

Inicia el [ejemplo BlueROV2](rovs.md#start-bluerov2), espera a que se conecte el autopiloto y consulta sus interfaces:

{{CODE:robot-control:1}}

La durabilidad `volatile` selecciona muestras nuevas; el filtro espera a `connected: true` después de establecer la conexión. Si no termina, revisa los registros de inicio y la configuración de MAVROS/ArduSub.

El adaptador convierte `/joy` y `/keyboard/joy` en órdenes MAVROS:

- Publicación: **20 Hz**.
- Tiempo de espera de la entrada: **0,3 s**.

Utiliza una sola fuente de órdenes y comprueba el estado de armado y navegación. [ROV y BlueROV2](rovs.md) explica el teclado y el puente WebSocket.
