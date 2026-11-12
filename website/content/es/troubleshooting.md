# Solución de problemas

Empieza por el primer error del registro de lanzamiento. Revisa, en este orden, el entorno, los recursos del modelo, el tiempo de simulación, los plugins y la recepción de datos. Conserva el error y las líneas cercanas que permitan reproducirlo.

## No se encuentra un paquete

{{CODE:troubleshooting:0}}

El resultado debe indicar `lyrical` y la ruta del espacio de trabajo elegido. Prepara el [terminal](terminals.md) según tu [método de instalación](requirements.md) y distingue las rutas del equipo de las del contenedor.

## Faltan modelos o mallas

Busca la primera URI que no se puede resolver. Comprueba que se hayan instalado los recursos del paquete y cargado el entorno correcto. Gazebo necesita acceso a la red para descargar recursos de Fuel. La primera descarga puede tardar: comprueba su progreso antes de iniciar otra simulación.

## No hay imagen o falla el renderizado

Revisa controladores y conexión de pantalla en la [guía de instalación](requirements.md). Gazebo y las cámaras necesitan renderizado. Para el sonar, comprueba el [motor elegido y los paquetes instalados](sonar-tuning.md).

Para los [efectos de la escena submarina](underwater-rendering.md), comprueba:

- El servicio `/posim/appearance/<session_nonce>/state`, las rutas del renderizador y los shaders, y el número de cámaras activas.
- La revisión aplicada y el estado de preparación después del renderizado.
- La activación explícita de las partículas suspendidas: vienen desactivadas. El ejemplo de atenuación de cámara no carga por sí solo el sistema de efectos de escena.

## El GNSS no obtiene una solución

Consulta el [estado de validez del GNSS](gnss.md), la altura de la antena en el sistema de coordenadas del mundo, la altura local del agua y la marca de tiempo de adquisición. Comprueba las coordenadas esféricas del mundo, que la antena esté fuera del agua y sin obstrucciones, y que transcurra suficiente tiempo de simulación para la readquisición y la latencia.

Si usas un tópico de olas, verifica que entregue muestras locales compatibles y recientes. Instalar el receptor no lo añade al modelo ni proporciona un adaptador ROS o de autopiloto.

## No llegan mensajes

Confirma primero que avance el tiempo y descubre el tópico y su tipo:

{{CODE:troubleshooting:1}}

Usa el transporte correcto, el mismo entorno preparado y ajustes compatibles de QoS y dominio. Revisa errores del puente y del plugin. En el lanzamiento de sensores, `namespace` debe identificar un descriptor existente. El [USBL](usbl.md) ya está incorporado en su mundo de ejemplo y se inicia con el lanzamiento del mundo.

## Aparecen los tópicos de imagen, pero no llegan fotogramas {#image-topics-appear-but-frames-stall}

Si Gazebo genera imágenes y ROS descubre el publicador, pero llegan pocos fotogramas o ninguno, revisa el transporte DDS. Los datagramas UDP grandes de Cyclone DDS pueden alcanzar límites de fragmentación IP. Reducir su tamaño puede ayudar en redes con MTU de 1500 bytes; consulta la [configuración de fragmentación de Cyclone DDS](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation).

### Preparar el perfil

Con `rmw_cyclonedds_cpp` instalado, crea este perfil una vez. Si ya tienes un perfil Cyclone DDS, incorpora los dos tamaños a su sección `General` y conserva la configuración de descubrimiento e interfaces.

{{CODE:troubleshooting:2}}

### Reiniciar con la misma configuración

Detén la simulación y los visores. Carga el entorno POSIM y selecciona el perfil en cada terminal antes de reiniciarlos. Si modificaste un perfil existente, usa su ruta.

{{CODE:troubleshooting:3}}

Si ya hay un daemon de la CLI de ROS en ese dominio, detenlo una vez para que las siguientes consultas utilicen la nueva configuración:

{{CODE:troubleshooting:4}}

Repite las [comprobaciones de imagen y frecuencia](camera.md#run-the-example), incluidas las marcas de tiempo de adquisición. Los paquetes pequeños añaden sobrecarga; una MTU inferior requiere tamaños todavía menores.

## El vehículo no se mueve

Comprueba el estado del controlador, el armado, la validez de navegación, la recepción de órdenes y el modo seleccionado. Revisa nombres de articulaciones y ejes de propulsores. Mantén una sola fuente de órdenes activa. Una entrada neutra, el mundo en pausa o el autopiloto desarmado pueden explicar que el vehículo permanezca quieto.

## La simulación no termina

Detén los visores, pulsa Ctrl+C una vez en el terminal de lanzamiento y espera a que terminen los procesos. Conserva los mensajes relevantes de aborto, fallo de segmentación o terminación forzada. Al [informar del problema](https://github.com/IOES-Lab/POSIM/issues), incluye comando, revisión, sistema operativo, arquitectura y renderizador.

### RViz falla al pulsar Discard

En Ubuntu 26.04, RViz 15.2.6 puede fallar de forma intermitente al cerrar la ventana y elegir **Discard**, aunque la visualización de sensores funcione. Detén los procesos de lanzamiento restantes con Ctrl+C antes de volver a empezar. Indica la versión de RViz y la acción de cierre en el informe.

Con renderizado por software en Docker sobre ARM64, esta versión de RViz también puede seguir ejecutándose después de que Ctrl+C cierre su ventana, hasta que ROS launch fuerce la terminación del proceso. Para ejecutar el sonar sin el visor, añade `rviz:=false` al comando `multibeam_sonar_demo.launch.py`. Las imágenes siguen disponibles en ROS.

## Recrear cámaras de profundidad en un SDK nativo de macOS

Para el SDK **macOS arm64 / Ogre-Next 2.3.3**, el [entorno Ogre aislado](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) incorpora la corrección del ciclo de vida de materiales necesaria al destruir y recrear cámaras de profundidad.

Sigue las comprobaciones exactas de SDK y código fuente, compila con el asistente y pasa el comando de Gazebo a `run_overlay.py`. Compilar no selecciona automáticamente esa biblioteca. Verifica las rutas cargadas y conserva el resultado de validación. El instalador normal no activa este entorno; no sustituye bibliotecas del sistema ni configura contenedores Linux. Otras versiones y plataformas necesitan un perfil compatible propio.

## Preparar un informe reproducible

Incluye el SHA del código o digest de la imagen, versiones de dependencias, comando y argumentos, primer error, resultado esperado, tipos de tópicos pertinentes y un ejemplo mínimo. Distingue entre descubrir un tópico y recibir datos por él.

## MAVROS 2.16.0 termina con SIGSEGV

El instalador de POSIM y las compilaciones oficiales de Docker incluyen la
[corrección del ciclo de vida del Router](https://github.com/IOES-Lab/POSIM/tree/main/extras/mavros)
para MAVROS 2.16.0. Tras cargar el entorno de POSIM, `ros2 pkg prefix mavros`
debe mostrar `/opt/posim/mavros-router/install`. Para una instalación existente,
sigue las instrucciones de compilación independiente del enlace. Si el fallo
persiste, adjunta al informe el registro de MAVROS y las rutas de las bibliotecas
que se han cargado.

## QGC se abre en Windows, pero POSIM se ejecuta en WSL {#qgc-on-windows-host}

Cierra la aplicación de Windows y abre la versión de Linux de QGC en la misma distribución Ubuntu que POSIM o dentro del contenedor POSIM. Una ventana de WSLg aparece en el escritorio de Windows, pero sigue siendo un proceso de Linux. La [configuración de QGC](qgroundcontrol.md) mantiene la telemetría y el vídeo dentro de ese entorno.
