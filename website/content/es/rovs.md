# ROV y BlueROV2

`namespace` selecciona el directorio de un modelo instalado. Sus tópicos ROS siguen la configuración de ese modelo.

## Modelos disponibles

| Modelo | Uso |
|---|---|
| `rexrov` | Propulsores Gazebo, hidrodinámica y odometría |
| `bluerov2` | BlueROV2 con ArduSub SITL y MAVROS |
| `bluerov2_heavy` | Configuración Heavy |
| `bluerov2_heavy_multibeam_sonar` | Heavy con sonar |

Utiliza la [terminal preparada para tu instalación](terminals.md). Para REXROV, usa el [ejemplo siguiente](#start-rexrov).

## Iniciar BlueROV2 {#start-bluerov2}

{{CODE:rovs:0}}

Selecciona la variante Heavy con `namespace:=bluerov2_heavy`. Comprueba la conexión del autopiloto en la salida de inicio y `/mavros/state`. La variante con sonar necesita la [configuración ](sonar-tuning.md).

## Controlar con el teclado

En una terminal interactiva del escritorio, utiliza `gui:=true headless:=false use_teleop:=true`.

| Tecla | Acción |
|---|---|
| `c` / `x` | Armar / desarmar |
| `w` / `s` | Avanzar / retroceder |
| `a` / `d` | Girar a izquierda / derecha |
| `r` / `f` | Ascender / descender |
| `h` / `j` | ALT_HOLD / STABILIZE |
| Espacio | Orden neutra |

## Usar el joystick del navegador

Sirve la página instalada desde otra terminal preparada:

{{CODE:rovs:1}}

Inicia BlueROV2 con la teleoperación activada:

{{CODE:rovs:2}}

Mantén `use_teleop:=true`, ya que el puente WebSocket pertenece a ese lanzamiento. Si el navegador no se abre automáticamente, abre la URL en el mismo escritorio Linux. Las imágenes Docker incluyen Firefox y QGC. [QGroundControl](qgroundcontrol.md) explica su instalación, los joysticks virtuales y el vídeo en directo.

Para suscripciones y órdenes desde tus programas, continúa con [ROS](ros.md).

## Ver el robot en movimiento

{{MEDIA:rovs}}

Imágenes y vídeos: POSIM Notion Wiki. La atribución a DAVE está en [Citas y licencias](citation.md). Utiliza los comandos y tópicos de esta página.

## Iniciar REXROV {#start-rexrov}

En la [terminal A preparada](terminals.md), inicia este ejemplo separado para las guías de Python y propulsores. Abre el mundo oceánico y los puentes ROS, y coloca REXROV en z = −5 m. Mantén esta sesión activa mientras sigues las guías en la terminal B.

{{CODE:rovs:3}}
