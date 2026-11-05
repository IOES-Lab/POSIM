# Cámara subacuática

`UnderwaterCamera` usa la profundidad para atenuar el color RGB con la distancia. Ajusta la atenuación y el color de fondo según el agua que quieras representar. Para efectos compartidos por toda la escena, consulta el [renderizado subacuático](underwater-rendering.md).

<a id="run-the-example"></a>
## Ejecutar el ejemplo

Este ejemplo usa el filtro de cámara basado en profundidad. El renderizador de escena tiene su propia [compilación y configuración](underwater-rendering.md). En la [terminal A preparada para tu instalación](terminals.md):

{{CODE:camera:0}}

En la terminal B preparada:

{{CODE:camera:1}}

Añade una visualización Image en RViz. Si el topic aparece pero deja de recibir imágenes, revisa el [transporte de imágenes grandes](troubleshooting.md#image-topics-appear-but-frames-stall).

## Configurar el plugin

Inserta este sensor RGBD dentro de un `<link>`. El mundo necesita `gz-sim-sensors-system` con `<render_engine>ogre2</render_engine>`, como en el [mundo de cámara](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world). El plugin lee RGB/profundidad de Gazebo y publica directamente en ROS `/underwater_camera/simulated_image`.

{{CODE:camera:2}}

| Parámetro SDF | Significado | Predeterminado |
|---|---|---|
| `attenuationR` | Atenuación roja por metro, finita y ≥ 0 | `1/30` |
| `attenuationG` | Atenuación verde por metro, finita y ≥ 0 | `1/30` |
| `attenuationB` | Atenuación azul por metro, finita y ≥ 0 | `1/30` |
| `backgroundR` | Fondo rojo, entero de 0 a 255 | `0` |
| `backgroundG` | Fondo verde, entero de 0 a 255 | `0` |
| `backgroundB` | Fondo azul, entero de 0 a 255 | `0` |

## Topics y geometría de cámara

`<topic>` debe generar un nombre ROS válido. Si se omite, se usa el ámbito Gazebo `/world/<world>/model/<model>/link/<link>/sensor/<sensor>` y se añade `/simulated_image` para la salida ROS. Los modelos anidados incluyen los ámbitos de sus modelos superiores.

Para convertir profundidad en distancia, se prioriza una proyección de lente explícita, después los parámetros intrínsecos explícitos y, por último, el tamaño de imagen y el campo de visión horizontal. El punto principal puede tener coordenadas cero. Las distancias focales deben ser positivas, los valores de lente finitos y los planos de recorte cumplir `0 < near < far`, en metros. Una configuración inválida genera un error y desactiva la salida.

## Sincronización y valores del ejemplo

Los fotogramas RGB y de profundidad se emparejan por la marca temporal de adquisición, independientemente del orden de llegada. ROS conserva la marca temporal y el identificador del marco de referencia del RGB. Al llenarse el búfer se descartan fotogramas sin pareja; reiniciar el tiempo vacía los pendientes y permite empezar de nuevo.

El ejemplo solicita **320 × 240 a 10 Hz**, atenuación RGB **0.8/0.5/0.2** y fondo **85/107/47**. El SDF prevalece sobre los valores del código.

## Ajustar y comparar

Coloca un objeto reconocible a distancias conocidas. Mantén fijas posición, luz, materiales y cámara mientras modificas un canal. Comprueba marcas temporales y frecuencia recibida además de la imagen.

La atenuación es exponencial por canal y se combina con el fondo. Para una comparación sin atenuación, fija los **seis parámetros** de la tabla a cero. Omitir etiquetas recupera los valores predeterminados.

El [renderizador de escena](underwater-rendering.md) aplica efectos comunes a superficies PBR y cámaras. Con ese sistema nativo activo, `POSIM_UNDERWATER_PLUGIN` deja pasar los píxeles ya coloreados para evitar un segundo filtrado. El [modelo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) define conexiones y topics.

## Partículas suspendidas

El renderizador puede mostrar partículas en el color RGB/RGBD. Están desactivadas inicialmente; actívalas mediante el [servicio de apariencia](underwater-rendering.md#turn-on-marine-snow).

- Comparten posiciones mundiales entre cámaras y se desplazan uniformemente con el tiempo simulado, independientemente de las corrientes.
- Las superficies opacas las ocultan. Se muestran entre **0.05 y 3 m**, limitadas por la escena y el nivel medio del agua.
- La profundidad numérica y los rayos acústicos conservan sus valores.

Son efectos ópticos procedimentales. Para estudiar dispersión calibrada necesitas un modelo óptico adecuado. En navegación visual, trátalas como observaciones móviles y conserva los ajustes entre comparaciones.

## Comparar imágenes subacuáticas

{{MEDIA:camera}}

Imágenes: wiki Notion de POSIM; atribución DAVE en [Citas y licencias](citation.md). Usa los comandos y topics de los bloques de esta página.
