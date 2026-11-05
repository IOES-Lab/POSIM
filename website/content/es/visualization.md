# Ver y grabar datos de sensores

Mantén el simulador en la terminal A y utiliza la B para comprobar las mediciones, abrir un visor o grabar datos.

## Recibir una imagen

Prepara [ambas terminales en el mismo entorno](terminals.md). Detén la simulación anterior con Ctrl+C en A e inicia esta escena. Gazebo se abre en el escritorio configurado durante la instalación.

{{CODE:visualization:0}}

En B:

{{CODE:visualization:1}}

Comprueba que la anchura y la altura sean mayores que cero; revisa la codificación y la marca de tiempo. `--no-arr` oculta los valores de los píxeles. Para ver la imagen, añade una visualización **Image** en RViz y selecciona este tópico. Los parámetros de atenuación se explican en [Cámara submarina](camera.md).

## Elegir un visor

En RViz, utiliza **Image** para cámaras, **PointCloud2** para nubes de puntos y visualizaciones de **TF/pose** para sistemas de referencia y estado.

Cada cliente ROS independiente necesita instalar y cargar `posim_interfaces` para los mensajes específicos de POSIM. Consulta los tópicos y tipos en las referencias de [cámara](camera.md), [presión](pressure.md), [DVL](dvl.md), [USBL](usbl.md) y [sonar](sonar.md). [Corrientes](currents.md) y [coordenadas esféricas](coordinates.md) también permiten cambiar entradas mediante servicios ROS.

## Grabar y reproducir

{{CODE:visualization:2}}

Pulsa Ctrl+C para terminar. Usa una carpeta de salida nueva para cada grabación y comprueba los recuentos de mensajes:

{{CODE:visualization:3}}

Detén el simulador antes de reproducir los datos sobre los mismos tópicos:

{{CODE:visualization:4}}
