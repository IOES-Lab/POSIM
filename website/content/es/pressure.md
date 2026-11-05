# Presión hidrostática

El plugin calcula presión a partir de la profundidad del modelo y publica mensajes ROS `FluidPressure`. También puede publicar una profundidad estimada a partir de esa presión.

## Observar REXROV

Inicia el robot en la [terminal A preparada para tu instalación](terminals.md):

{{CODE:pressure:0}}

En la terminal B preparada:

{{CODE:pressure:1}}

La presión se expresa en **Pa**, la varianza en **Pa²** y la profundidad en **m**.

## Integrar el plugin

Coloca el bloque directamente dentro del `<model>` que quieras medir. `namespace` identifica sus tópicos:

{{CODE:pressure:2}}

La salida es `/model/<namespace>/<topic>`; la profundidad añade `_depth`. Se mide el modelo al que está asociado el plugin. El namespace puede ser distinto del nombre del modelo.

## Parámetros

| Parámetro | Significado y valor predeterminado |
|---|---|
| `namespace` | Obligatorio; prefijo válido de tópico ROS |
| `topic` | Nombre de salida; `sea_pressure` |
| `standard_pressure` | Presión superficial finita y no negativa, kPa; `101.325` |
| `kPa_per_meter` | Incremento hidrostático finito y positivo, kPa/m; `9.80638` |
| `estimate_depth_on` | Publicar profundidad calculada; `true` |
| `update_rate` | Frecuencia finita, Hz; `0`. Un valor no positivo publica en cada avance físico |
| `saturation` | Valor aceptado, `3000`; no limita la presión en esta implementación |

La profundidad es `max(0, -z)` en coordenadas del mundo, incluidas las transformaciones de modelos anidados. La referencia es el plano z = 0, independiente de las olas. El cálculo usa kPa y la publicación convierte a Pa.

## Tiempo y validez

Presión ROS, profundidad y presión Gazebo comparten la misma marca temporal simulada. La pausa o una marca repetida no generan una nueva medida; la publicación se reanuda al avanzar o retroceder el tiempo.

Una configuración inválida deja el plugin inactivo. Si la pose falta, no es finita o el cálculo desborda, no se publica una muestra.

La varianza publicada es fija: **9 000 000 Pa²**. No se añade ruido aleatorio a la presión. Tenlo en cuenta al configurar un estimador. El [código del plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc) muestra ambos cálculos.

## Comparar profundidades

Mide dos posiciones z conocidas sin cambiar la pendiente de presión. La diferencia debe ser el incremento por metro multiplicado por el cambio de profundidad. La profundidad derivada usa el mismo cálculo y no constituye una medición independiente. Ajusta unidades y presión de referencia del estimador de forma coherente.

## Salidas de presión y profundidad

{{MEDIA:pressure}}

Imágenes: wiki Notion de POSIM; atribución DAVE en [Citas y licencias](citation.md). Usa los comandos y tópicos de los bloques de esta página.
