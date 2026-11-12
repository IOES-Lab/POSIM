# Controlar con QGroundControl

Conecta QGroundControl (QGC) al BlueROV2 simulado, usa los joysticks virtuales y observa la cámara frontal. Completa la [instalación](requirements.md) y prepara las [terminales A y B](terminals.md).

## 1. Abrir QGC junto a POSIM

**Windows/WSL:** utiliza la versión de Linux de QGC en estos tutoriales. No abras la aplicación instalada en Windows. Inicia QGC en la misma distribución Ubuntu que POSIM o, si usas Docker, dentro del mismo contenedor POSIM. WSLg puede mostrar la ventana de Linux en el escritorio de Windows; la aplicación sigue ejecutándose en Linux.

Las direcciones `127.0.0.1` de esta guía corresponden al entorno donde se ejecuta QGC. No configuran la aplicación de Windows para recibir telemetría o vídeo desde WSL o Docker.

| Instalación | Dónde ejecutar QGC |
|---|---|
| Ubuntu nativo | Instala y abre la aplicación Linux en Ubuntu. |
| Windows con WSL nativo | Instala y abre la aplicación Linux en la misma distribución Ubuntu. |
| Linux o Windows con Docker | Abre la aplicación incluida dentro del contenedor POSIM. |
| macOS con Docker y RDP | Abre la aplicación desde una terminal del escritorio RDP. |

Para una instalación nativa, sigue la [guía oficial de instalación de QGC](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html) y elige la versión Linux para tu arquitectura. En Docker, utiliza la terminal B:

{{CODE:qgroundcontrol:0}}

El símbolo `&` permite seguir escribiendo comandos mientras QGC permanece abierto. Si lo abriste desde otra terminal, déjala abierta. Si aparecen las preferencias iniciales, selecciona **Metric System**.

QGC, MAVROS y Gazebo comparten el mismo entorno Linux: la telemetría usa **UDP 14550** y el vídeo **UDP 5600**. No hace falta publicar estos puertos en Docker. En macOS, accedes a ese entorno mediante RDP.

## 2. Configurar los joysticks

Abre **Application Settings → Fly View → Virtual Joystick**:

- Activa **Virtual Joystick**.
- Activa **Auto-Center Throttle** para que la orden vuelva al centro al soltar la palanca.
- Deja **Left-Handed Mode** desactivado para utilizar la disposición de este ejemplo.

## 3. Configurar el vídeo

Abre **Application Settings → Video**. Selecciona primero la fuente de vídeo; después introduce la dirección UDP y los demás valores de la tabla.

| Ajuste | Valor |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`, proporción automática |
| Disable Video Stream When Disarmed | Desactivado |
| Low Latency Mode | Activado |
| Auto-reconnect on stream loss | Activado |

## 4. Iniciar el robot y la cámara

Detén la simulación anterior. En la terminal A preparada:

{{CODE:qgroundcontrol:1}}

Gazebo abre la escena del fondo marino y QGC se conecta a ArduSub. Espera a que termine la inicialización. En la terminal B:

{{CODE:qgroundcontrol:2}}

Vuelve a **Fly View** y pulsa la vista previa del vídeo para ampliarla.

{{MEDIA:qgroundcontrol}}

La cámara genera imágenes de **640 × 480 a 5 Hz en tiempo simulado**. El renderizado por software puede ralentizar la simulación y reducir la frecuencia de imágenes mostrada. BlueROV2 tiene flotabilidad positiva y puede ascender sin armar los motores, dejando los objetos fuera de la imagen.

`qgc_video:=true` añade el emisor H.264. El vídeo RGB incluye los [efectos submarinos del mundo](underwater-rendering.md). El [filtro de cámara](camera.md) independiente tiene su propia salida ROS. GNSS pierde la posición al sumergirse la antena y la recupera al emerger; telemetría, vídeo y control manual siguen conectados. Usa Manual bajo el agua. La lente queda por debajo de la antena, por lo que puede verse una imagen submarina con una posición GNSS válida.

## 5. Armar, mover y detener

1. Espera a **Ready** y selecciona **Manual**.
2. Abre el menú de estado del vehículo y mantén pulsado **Arm** hasta completar la acción.
3. Mueve brevemente la palanca derecha hacia delante y hacia atrás.
4. Suéltala para volver a una orden neutra. Las corrientes y la inercia pueden seguir desplazando el robot.
5. Mantén pulsado **Disarm** en el menú de estado al terminar.

Mantén desactivados el control por teclado y el joystick web para evitar órdenes simultáneas. Detén el vídeo desde B:

{{CODE:qgroundcontrol:3}}

Pulsa **Ctrl+C** en A y cierra QGC.

MAVROS espera hasta 60 segundos sin recibir un mensaje de latido del vehículo antes de indicar que se ha perdido la conexión. Este margen permite un renderizado por software lento; por ello, una pérdida real de los mensajes de latido puede tardar hasta 60 segundos en aparecer en `/mavros/state`.

## Resolver problemas de conexión

- **No aparece el vehículo:** comprueba que QGC comparte el entorno Linux de POSIM y que no hay otra instancia utilizando UDP 14550.
- **Waiting for video:** revisa la fuente H.264, la dirección UDP y `qgc_video:=true`; activa el envío desde B y espera varios fotogramas.
- **Otro puerto:** utiliza `qgc_video_port:=5602` y `127.0.0.1:5602` en QGC.
- **Falta la opción `qgc_video`:** actualiza POSIM o [descarga la imagen y recrea el contenedor](docker.md#update-an-existing-container).
- **Avisos de firmware o puerto serie:** la simulación usa el ArduSub incluido y una conexión de red; no requiere cargar firmware ni configurar un puerto serie.

Consulta los [ajustes oficiales de vídeo](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html) y las [interfaces ROS de los propulsores](robot-control.md).
