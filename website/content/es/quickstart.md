# Primera simulación

Explora una escena de inspección en aguas someras con BlueROV2 y después un recorrido de boyas con WAM-V. Cada ejemplo abre Gazebo y una ventana de RViz ya configurada. Ejecuta una sola simulación a la vez.

## Antes de empezar

Completa una [instalación](requirements.md) y carga el mismo espacio de trabajo de ROS/POSIM en las [terminales A y B](terminals.md). A ejecuta el simulador; B consulta datos e inicia el vídeo. Con Docker, utiliza el escritorio Linux, WSLg o RDP configurado durante la instalación. Ambos ejemplos forman parte de la instalación estándar; no requieren una imagen de superficie aparte.

## Inspección submarina con BlueROV2

En la **terminal A**:

{{CODE:quickstart:0}}

BlueROV2 empieza unos 3,1 m por debajo de la superficie media, sobre un fondo situado a 4 m, mirando hacia una caja, una tubería, un pilar y una esfera. Hay oleaje suave, turbidez submarina, absorción, cáusticas y nieve marina. El lastre se ajusta para mantener el vehículo desarmado cerca de los objetos. Gazebo muestra la cámara en directo junto a la escena.

Para añadir sonar multihaz, agrega `sonar:=true` al comando anterior. Se selecciona BlueROV2 Heavy con sonar: Gazebo añade una imagen de sonar y RViz muestra la imagen y la nube de puntos. El sonar consume muchos recursos y **puede reducir la frecuencia de imagen y la respuesta de los controles**. Según el equipo y la configuración del sensor, `auto` prueba CUDA, después WGPU y finalmente CPU. Se mantienen los valores habituales de renderizado. Consulta los [requisitos del sonar](sonar-tuning.md).

Las capturas siguientes utilizan la variante con sonar. RViz carga automáticamente el modelo, la cámara y las vistas de los sensores. La cuadrícula de 1 m sirve como referencia en el sistema de coordenadas del vehículo; no representa un fondo medido. **Move Camera** está seleccionado: arrastra para girar, usa el botón central para desplazar y la rueda para ampliar. Las hélices mostradas conservan su ángulo de reposo; la dinámica simulada no cambia.

{{MEDIA:quickstart:0}}

### Comprobar datos en directo

En la **terminal B**:

{{CODE:quickstart:1}}

Para la variante con sonar, sustituye `bluerov2` por `bluerov2_heavy_multibeam_sonar` en el tópico. Las posiciones deben seguir llegando aunque el vehículo esté casi inmóvil. Detén la consulta con **Ctrl+C** y comprueba que el tiempo simulado avanza:

{{CODE:quickstart:2}}

Pulsa Ctrl+C para cerrar esta consulta. Si faltan mensajes o el reloj está pausado, consulta [Solución de problemas](troubleshooting.md).

### Ver vídeo y controlar con QGC

En Windows/WSL, ejecuta la versión de Linux de QGC en la misma distribución Ubuntu que POSIM; con Docker, ejecútala dentro del mismo contenedor POSIM. No abras la aplicación instalada en Windows para estos tutoriales.

Sigue los pasos 1–3 de la [configuración de QGC](qgroundcontrol.md) y activa los joysticks virtuales. Selecciona **UDP h.264 Video Stream** y la dirección **127.0.0.1:5600**. Activa **Low Latency Mode** y desactiva **Disable Video Stream When Disarmed**. Mantén el tutorial en ejecución e inicia el vídeo en la terminal B:

{{CODE:quickstart:3}}

Haz clic en el panel pequeño de vídeo de QGC para ampliarlo. **No GPS Lock** es normal mientras la antena GNSS marina está sumergida. La telemetría, el vídeo y el control Manual siguen disponibles. Espera a **Ready**, selecciona **Manual**, arma y mueve brevemente el joystick. Suéltalo para volver al neutro y desarma. Consulta los [pasos de control con QGC](qgroundcontrol.md). No inicies otro controlador de teclado o joystick al mismo tiempo.

{{MEDIA:quickstart:1}}

## Sensores de superficie con WAM-V

Detén primero la sesión submarina. En la **terminal A**:

{{CODE:quickstart:4}}

WAM-V empieza sobre olas suaves frente a cuatro boyas ancladas. RViz abre la cámara y una vista conjunta del vehículo y la nube lidar, con cuadrícula de 1 m y Move Camera seleccionado. El lidar realiza un barrido horizontal: sus puntos perfilan las boyas a la altura del sensor, sin reconstruir todo el objeto en 3D.

En el panel **Lidar** de Gazebo, pulsa actualizar y selecciona `/tutorial/lidar`. Desactiva **Show Non Hitting Rays** para mostrar solo los rayos que alcanzan objetos. El panel de cámara permanece junto a la escena.

{{MEDIA:quickstart:2}}

La cámara utiliza el mismo tópico `/tutorial/camera`, el mismo comando de inicio y la misma configuración de vídeo de QGC que el ejemplo submarino. WAM-V emplea el modo de embarcación de ArduRover y la posición/velocidad simulada como navegación externa; consulta [Robots de superficie y olas](surface.md). Esta entrada es independiente del receptor GNSS marino de BlueROV2.

En el modo Manual de QGC, WAM-V usa el movimiento vertical del joystick izquierdo para el acelerador. Arma, muévelo brevemente, suéltalo al neutro y desarma.

{{MEDIA:quickstart:3}}

Lee un barrido lidar en la **terminal B**:

{{CODE:quickstart:5}}

La nube está en `/tutorial/lidar/points` y la odometría en `/model/wamv/odometry`.

## Terminar la sesión

Desarma, cierra RViz y QGC y pulsa **Ctrl+C en la terminal A**. Espera a que terminen los procesos. Si RViz pregunta si quieres guardar cambios de vista, descártalos para conservar el ajuste proporcionado. Si no se cierra correctamente, consulta los [problemas del visor](troubleshooting.md). Cerrar una consulta de tópicos no detiene la simulación. Termina la sesión actual antes de iniciar otro ejemplo.

Para volver a la posición inicial, detén la sesión y ejecuta de nuevo su comando de lanzamiento. El reinicio completo del mundo en Gazebo elimina los modelos añadidos después del arranque, incluido el vehículo del tutorial, y no reinicia su piloto automático.

## Continuar aprendiendo

- [Ver y grabar datos de sensores](visualization.md): examinar imágenes y guardar mediciones.
- [Controlar con Python](python-control.md): iniciar el ejemplo REXROV separado y probar un propulsor.
- [ROV y BlueROV2](rovs.md): usar ArduSub, teclado o joystick local.
- [Biblioteca de mundos](worlds.md): elegir entornos y consultar argumentos de lanzamiento.
