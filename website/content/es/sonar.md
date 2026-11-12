# Sonar multihaz

El sonar multihaz de POSIM utiliza un modelo de dispersores puntuales para generar datos de intensidad y distancia e imágenes acústicas. El mismo plugin permite calcular con CPU, WGPU o CUDA. WGPU utiliza Vulkan en Linux o Metal en un entorno nativo de macOS para trabajar con GPU de distintos fabricantes. Consulta la [configuración del cálculo](sonar-tuning.md).

## Ejecutar un ejemplo {#run-a-sonar-example}

Prepara las [terminales A y B](terminals.md) y, si faltan, [compila los paquetes del sonar](sonar-tuning.md#build-the-sonar-packages). Inicia la simulación en A y consulta los datos en B. El primer comando selecciona el motor de cálculo automáticamente; el ejemplo específico selecciona WGPU y abre RViz.

{{CODE:sonar:0}}

El paquete `posim_multibeam_sonar_demo` también ofrece un ejemplo independiente:

{{CODE:sonar:1}}

{{MEDIA:sonar:0}}

Ejecuta un solo ejemplo a la vez. Sustituye `wgpu` por `cpu`, `cuda` o `auto` para cambiar el motor de cálculo. Para un sonar a bordo, selecciona `bluerov2_heavy_multibeam_sonar` con un mundo adecuado.

## Cómo funciona el sensor

Gazebo aporta la geometría de la escena. El modelo estima las normales de superficie, calcula la retrodispersión acústica, aplica la respuesta de recepción y comprime los retornos en celdas de distancia. Los datos acústicos y la imagen corresponden a la misma observación. Usa los datos brutos para el análisis y conserva los ajustes de ganancia y visualización.

Un valor positivo de `spec/receiveBeamWidth` activa el modo físico, que usa las coordenadas XYZ medidas por una cámara de profundidad que cubre todo el campo del sonar. Las muestras geométricas, las direcciones receptoras y las salidas de distancia son independientes. Las celdas cuyo centro está dentro del campo reparten su ángulo sólido completo; sus áreas de superficie proyectadas y la respuesta receptora con signo determinan la contribución a cada haz. Se usa un pulso Hamming de banda base, ancho de banda fijo y pico unitario. El intervalo de cálculo interno es mayor que el intervalo de distancia publicado. El paso de salida es `soundSpeed / (2 × bandwidth)`; la longitud de cálculo no determina la ganancia del pulso. El intervalo de guarda interno no se publica ni cambia ese paso.

`ray/scan/horizontal/samples` define las columnas geométricas, `ray/scan/vertical/rays` las filas de imagen y `beams` las direcciones receptoras. El modo físico requiere un campo simétrico de menos de 180° en cada eje. Los límites de distancia son radiales; los píxeles fuera del campo o de esos límites tienen XYZ no válidas en la nube organizada. `spec/receiveBeamWidth` es la anchura total a media potencia, en radianes; `spec/receiveGain` es la ganancia lineal de presión (predeterminada: 1). Omitir la anchura o ponerla a cero selecciona el modo de cuadrícula predeterminado. El modo físico admite CPU y WGPU, pero no CUDA. No hay un límite fijo de 4096 muestras de frecuencia; las cuadrículas grandes siguen sujetas a los recursos del motor y a las dimensiones representables. Los coeficientes de dispersión se generan por imagen y celda geométrica. Se modelan retornos de banda base, sin propagación de fase portadora ni calibración de amplitud del equipo.

## Consultar los datos

En el terminal B:

{{CODE:sonar:2}}

- La imagen usa QoS `reliable`; selecciona esa fiabilidad también en el panel Image de RViz.
- Añade PointCloud2 para `/sensor/multibeam_sonar/point_cloud`.
- Los datos acústicos sin procesar usan `marine_acoustic_msgs/msg/ProjectedSonarImage`.
- El lanzamiento de BlueView conecta el reloj de Gazebo con ROS `/clock`. Si un nodo usa `use_sim_time`, comprueba que el mundo esté en marcha y que avance el reloj.

## Configuración del sensor {#sensor-configuration}

Añade estos sistemas directamente dentro de `<world>`:

{{CODE:sonar:3}}

Coloca el sensor P900 dentro de un `<link>` del modelo:

{{CODE:sonar:4}}

El ejemplo solicita 512 haces horizontales en unos 130°, 300 rayos verticales en unos 12° y un alcance de 0.1–10 m. Usa 900 kHz, un ancho de banda de 29.9 kHz y una velocidad del sonido de 1500 m/s. Los 30 Hz configurados son la frecuencia solicitada; la frecuencia recibida depende del cálculo y del renderizado. Consulta el [modelo completo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf).

## Parámetros de `<spec>`

| Parámetro | Función | Valor predeterminado | Ejemplo P900 |
| --- | --- | --- | --- |
| `verticalFOV` | Apertura vertical acústica, grados | 10 | 12 |
| `sonarFreq` | Frecuencia acústica, Hz | 900000 | 900000 |
| `bandwidth` | Ancho de banda, Hz | 29500000 | 29900 |
| `soundSpeed` | Velocidad del sonido, m/s | 1500 | 1500 |
| `sourceLevel` | Nivel de la fuente, dB | 220 | 220 |
| `maxDistance` | Distancia de procesamiento, m; debe coincidir con el alcance de los rayos | 60 | 10 |
| `raySkips` | Paso de muestreo vertical; entero positivo | 10 | 10 |
| `sensorGain` | Ganancia de amplitud de ecos en la cuadrícula CPU predeterminada; sin uso en modo físico ni en WGPU/CUDA | 0.02 | 0.02 |
| `blazingSonarImage` | Semilla aleatoria basada en el reloj real; con `false`, semilla fija 1234 | false | true |
| `pointCloudTopicName` | Sufijo del tópico de nube de puntos de Gazebo | `point_cloud` | Igual |
| `sonarImageRawTopicName` | Sufijo del tópico ROS de datos acústicos | `sonar_image_raw` | Igual |
| `sonarImageTopicName` | Sufijo del tópico ROS de imagen | `sonar_image` | Igual |
| `frameName` | Identificador del sistema de referencia óptico | `forward_sonar_optical_link` | Igual |
| `writeLog` | Guardar CSV en un directorio propio de cada ejecución | false | false |
| `writeFrameInterval` | Número de fotogramas entre escrituras; entero positivo | 10 | 5 |
| `debugFlag` | Salida de diagnóstico en la consola | false | false |
| `plotScaler` | Se lee y se muestra, pero no interviene en la generación de imágenes | 10 | Omitido |

### Tópicos y sistemas de referencia

Los sufijos se añaden al `<topic>` del sensor, aquí `/sensor/multibeam_sonar`. La nube de puntos se publica como `gz.msgs.PointCloudPacked` y necesita el puente ROS incluido para obtener `sensor_msgs/msg/PointCloud2`. Haz coincidir `pointCloudTopicName` con `sensor_config.py` y usa el sistema de referencia óptico correcto en `frameName`.

Los archivos de lanzamiento del BlueView P900 y del sonar BlueROV2 publican una transformación identidad de `world` al sistema de referencia del sonar para la visualización en RViz. Las coordenadas de la nube de puntos siguen siendo relativas al sensor. Esta transformación estática no sigue el movimiento del vehículo ni incorpora la posición y orientación de montaje del sonar. Para crear mapas en coordenadas del mundo, sustitúyela por una cadena de transformaciones que incluya ambas poses.

### Geometría de los rayos

Los valores predeterminados de `<ray>` son 256 haces horizontales, 3 rayos verticales, ángulos horizontales ±π/4, verticales ±π/8 y alcance 0.1–5 m. Los ángulos se expresan en radianes; con `degrees="true"` se usan grados. El `<reference_frame>` opcional dentro de `<gz:multibeam_sonar>` parte de una pose nula.

Mantén completos los bloques `<ray>` y `<spec>`: los valores predeterminados se aplican a sus campos internos. Ajusta el alcance y `maxDistance` de forma coherente. La [implementación](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc) define cómo se interpretan los parámetros y se publican los datos.

### Mediciones repetibles

Deja desactivado el registro al medir rendimiento, salvo que forme parte del experimento. Consulta [Configuración y ajuste del sonar](sonar-tuning.md) y conserva los parámetros acústicos y `blazingSonarImage` al comparar ejecuciones.

### Motor de cálculo {#compute-backend}

| Selección | Comportamiento |
| --- | --- |
| `auto` | Prueba CUDA, WGPU y CPU, en ese orden; opción predeterminada |
| `wgpu` | Usa un adaptador Vulkan/Metal compatible; si no está disponible, el sensor no se inicializa |
| `cuda` | Requiere GPU NVIDIA y el backend CUDA compilado e instalado; falla si no está disponible |
| `cpu` | Calcula los ecos en la CPU; Gazebo sigue necesitando renderizado |

El ejemplo específico admite `compute_backend`; los demás lanzamientos leen `POSIM_SONAR_COMPUTE_BACKEND`. Comprueba la línea `Using sonar compute backend:` al arrancar. La guía [Configurar el sonar](sonar-tuning.md) explica la selección del adaptador, los requisitos y las diferencias numéricas.

## Añadir otro sensor

1. Copia `models/posim_sensor_models/description/blueview_p900/` con un nuevo identificador.
2. Prepara su `config/<identificador>/sensor_config.py`. Alinea nombres de modelo, enlaces, sistemas de referencia y tópicos del puente.
3. Ajusta muestras, ángulos y alcance. Incluye los dos sistemas del apartado de configuración en el mundo; usa el [SDF de ejemplo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world) como referencia.
4. Prepara el motor de cálculo y el renderizado, vuelve a compilar el espacio de trabajo y carga su entorno. Selecciona el sensor con los [argumentos de lanzamiento](worlds.md).

## Guardar y representar datos sin procesar

Con `writeLog=true`, cada sensor crea `SonarRawData_<run>_<suffix>/` en el directorio de trabajo del simulador e imprime la ruta. Guarda CSV numerados y `SonarRawData_beam_angles.csv`, sin sobrescribir grabaciones anteriores. Los intervalos inferiores a 1 se ajustan a 1; si no puede crear el directorio, desactiva el registro y comunica el error.

Desde el directorio indicado, abre el primer fotograma:

{{CODE:sonar:6}}

Para guardar las figuras en PNG:

{{CODE:sonar:7}}

La [herramienta de representación](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py) se instala también sin CUDA. Genera una imagen de sonar y cinco perfiles de haces repartidos uniformemente, o todos si hay menos de cinco.

| Opción | Efecto |
| --- | --- |
| `--range-max` | Distancia máxima mostrada, m |
| `--beam-step` | Mostrar un haz de cada N |
| `--dynamic-range` | Intervalo bajo el máximo, dB; predeterminado: 60 |

Los colores son relativos al máximo de cada fotograma: compara los valores CSV para contrastar ejecuciones. La primera columna contiene distancias; las demás, retornos complejos por haz. Conserva el archivo de ángulos junto a los datos y vigila el espacio ocupado en disco.

## Escenas y observaciones

{{MEDIA:sonar:1}}
{{MEDIA:sonar:2}}
{{MEDIA:sonar:3}}
{{MEDIA:sonar:4}}
{{MEDIA:sonar:5}}
{{MEDIA:sonar:6}}

Imágenes y vídeos: POSIM Notion Wiki. Las [referencias y licencias](citation.md) recogen la atribución de DAVE y del modelo de sonar. Usa los comandos y tópicos de los bloques de esta página.
