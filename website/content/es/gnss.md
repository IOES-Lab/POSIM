# GNSS marino

`posim_gz_gnss` simula un receptor satelital en el enlace de antena del modelo. Convierte su posición en el mundo en coordenadas geográficas, añade errores gaussianos configurables y publica posición, velocidad, marca temporal y estado en un único mensaje. Incluye pérdida de recepción bajo el agua, bloqueo explícito, readquisición y latencia.

## Compilar y montar el receptor

En la [terminal A preparada para tu instalación](terminals.md); en macOS, usa la terminal Linux del escritorio RDP:

{{CODE:gnss:0}}

El modelo necesita un enlace físico de antena, normalmente `gnss_link`, colocado y unido al vehículo. El mundo requiere [coordenadas WGS 84](coordinates.md). Añade al SDF del modelo:

{{CODE:gnss:1}}

Ajusta enlace y topic a tu robot. La guía para [crear un robot](custom-robots.md) explica cómo preparar y ejecutar el modelo.

Los [perfiles](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json) dan posiciones respecto a `base_link`: WAM-V z = 1.8 m, BlueROV2 0.3 m y REXROV 0.8 m. WAM-V y los tres perfiles BlueROV2 activan el receptor; REXROV lo desactiva por defecto. Los perfiles son datos de montaje: debes incorporar enlace y plugin al modelo. WAM-V también necesita el [modelo externo Wave Sim](surface.md).

BlueROV2, BlueROV2 Heavy y la variante sonar incluyen el receptor y el adaptador `marine_gnss_input`. Con ArduSub, los mensajes MAVROS `GPSINPUT` sustituyen al GPS sintético de SITL. El adaptador convierte velocidad ENU a NED y altura elipsoidal WGS 84 a altitud EGM96 sobre el nivel medio del mar mediante los datos GeographicLib instalados. Las mediciones sumergidas, bloqueadas, inválidas, futuras o de más de 1 s indican ausencia de posición. El número de satélites es desconocido (`255`) incluso con posición válida; no se calculan geometría satelital ni DOP. El receptor del ejemplo usa el nivel medio z = 0, independientemente de las olas visibles.

## Parámetros

| Parámetro SDF | Unidad o significado | Predeterminado |
|---|---|---|
| `update_rate` | Muestras por segundo simulado; >0 hasta 100 Hz | 5 |
| `latency` | Retardo de entrega, segundos simulados; 0–10 s | 0.2 |
| `reacquire_seconds` | Tiempo continuo de adquisición fuera del agua, s | 3 |
| `dry_margin` | Separación sobre el agua para adquirir, m | 0.05 |
| `horizontal_stddev` | Desviación estándar de posición horizontal, m | 0.7 |
| `vertical_stddev` | Desviación estándar de posición vertical, m | 1.2 |
| `velocity_stddev` | Desviación estándar de velocidad, m/s | 0.1 |
| `water_level` | z media del agua sin topic de olas, m | 0 |
| `seed` | Semilla aleatoria del receptor | 42 |
| `enabled` | Receptor activo | true |
| `link_name` | Enlace de antena del modelo | `gnss_link` |
| `topic` | Topic Gazebo de medición | `/model/<model-name>/gnss` |
| `wave_topic` | Topic opcional de altura del agua | Vacío |

Los ajustes numéricos deben ser finitos. Latencia, tiempo de adquisición, margen y desviaciones estándar deben ser no negativos.

## Altura local del agua

Sin `wave_topic`, se usa el plano medio del agua. Con un topic configurado, hacen falta muestras locales recientes; datos ausentes o inválidos producen `surface unavailable`.

El mensaje `gz.msgs.Double_V` contiene, en orden: esquema `1`, tamaño de cuadrícula, dimensiones periódicas X/Y, velocidad y ángulo del viento, pendiente de ola, tiempo simulado y alturas por filas.

- Tamaño de cuadrícula: entero entre 2 y 256; el número de valores de altura es el cuadrado de ese entero.
- Dimensiones X/Y: finitas y positivas.
- Marca temporal: finita, con antigüedad máxima de **0.6 segundos simulados**, nunca futura.

Configura por separado un publicador compatible. Los metadatos de viento no gobiernan la recepción.

## Leer mediciones y bloqueo

Tras iniciar el modelo configurado, lee su topic en la terminal B preparada:

{{CODE:gnss:2}}

Cada mensaje tiene 15 números de doble precisión. Los índices empiezan en cero. Conserva la marca de adquisición aunque la solución llegue con retraso.

| Índice | Campo y unidad |
|---|---|
| 0–3 | Esquema `1`, época de reinicio, adquisición en segundos simulados, estado |
| 4–6 | Latitud en grados, longitud en grados, altitud en m |
| 7–9 | Velocidades este, norte y arriba, m/s |
| 10–12 | Varianzas de posición horizontal/vertical en m² y de velocidad en (m/s)² |
| 13–14 | z mundial de antena y altura local del agua, m |

Estados: `0 disabled` (desactivado), `1 submerged` (sumergido), `2 acquiring` (adquiriendo), `3 fix` (solución válida), `4 blocked` (bloqueado) y `5 surface unavailable` (superficie no disponible).

Una medición inválida contiene coordenadas y velocidades `NaN`. El contacto con agua o el bloqueo eliminan las soluciones pendientes. La readquisición exige que la antena se mantenga de forma continua por encima del margen requerido. Un reinicio borra temporizador y muestras retrasadas, y aumenta la época.

Para bloquear la antena; usa `false` para liberarla:

{{CODE:gnss:3}}

## Interpretar y comprobar

Compara antena quieta fuera del agua, sumergida, bloqueada y recuperada. Comprueba adquisición, retardo, época tras reinicio y marcas temporales crecientes. El receptor de los datos debe rechazar soluciones antiguas o futuras y gestionar estados inválidos.

El adaptador BlueROV2 incluido alimenta ArduSub; otros consumidores ROS `NavSatFix` y autopilotos personalizados siguen necesitando un adaptador.

El modelo se basa en la posición de la antena. No calcula geometría de satélites, propagación RF, multitrayecto físico ni posicionamiento acústico. La precisión configurada es una entrada de simulación, no una garantía del fabricante. Consulta la [implementación](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc) y la [guía del paquete](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss).
