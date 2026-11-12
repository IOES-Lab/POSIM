# Posicionamiento USBL

El posicionamiento acústico de base ultracorta (USBL) localiza equipos bajo el agua. Este ejemplo conecta un transceptor con dos transpondedores. Puedes consultar todos o seleccionar uno por ID.

## Iniciar el mundo de ejemplo

En la [terminal A preparada para tu instalación](terminals.md):

{{CODE:usbl:0}}

{{MEDIA:usbl:0}}

El mundo ya contiene los tres dispositivos. En la terminal B preparada, recibe sus posiciones:

{{CODE:usbl:1}}

Ambos mensajes incluyen `transponder_id`, pero sus campos tienen significados distintos:

| Salida | x | y | z |
|---|---|---|---|
| `transponder_location` | Ángulo horizontal, grados | Distancia, m | Elevación, grados |
| `transponder_location_cartesian` | x relativa, m | y relativa, m | z relativa, m |

Se usa la diferencia entre las posiciones en el sistema de coordenadas del mundo. El ángulo horizontal vale 0° hacia +x y 90° hacia +y; la elevación positiva apunta a +z. Ten en cuenta estos ejes al combinar datos en la referencia del vehículo.

## Integración SDF

Coloca el transceptor dentro del modelo `box`. Cada ID corresponde al modelo situado en la misma posición de la lista `transponder_attached_object`:

{{CODE:usbl:2}}

Coloca este transpondedor en `sphere` y otro en `sphere2` con `transponder_ID` igual a `2`. Nombre, ID y namespace del transceptor deben coincidir en ambos extremos:

{{CODE:usbl:3}}

## Parámetros

| Parámetro | Dispositivo | Significado y predeterminado |
|---|---|---|
| `namespace` | Ambos | Namespace obligatorio, p. ej. `USBL` |
| `transceiver_device`, `transceiver_ID` | Ambos | Nombre e ID obligatorios para las rutas |
| `transponder_device`, `transponder_ID` | Ambos | Nombre e ID obligatorios; el transceptor admite IDs separados por comas |
| `transponder_attached_object` | Transceptor | Modelos obligatorios separados por comas, en el mismo orden que los IDs |
| `transceiver_model` | Transpondedor | Modelo obligatorio del transceptor para consultar su posición |
| `sound_speed` | Ambos | Velocidad de referencia a 10 °C y z = 0, m/s; `1540`, finita y positiva |
| `enable_ping_scheduler` | Transceptor | Booleano obligatorio; `false` usa pings manuales |
| `ping_frequency` | Transceptor | Frecuencia en tiempo simulado, Hz; `1`, ejemplo `0.5`; finita y positiva |
| `interrogation_mode` | Transceptor | `common` o `individual`; predeterminado `common` |
| `mu`, `sigma` | Transpondedor | Media y desviación estándar del ruido de posición, m; `0` y `1`, ejemplo `0` y `0`; finitas y sigma ≥ 0 |

Usa modelos existentes, IDs únicos y listas de IDs/modelos de igual longitud. Los IDs son enteros decimales de **0 a 2147483647**, sin signo ni ceros iniciales. Cada plugin necesita un modelo con un enlace. La [definición completa](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) incluye geometría y posiciones.

Una configuración inválida deja el plugin inactivo. Se ignoran destinos de canal o comando desconocidos; si el canal es desconocido, se conserva la selección actual. El primer transpondedor de la lista es el canal inicial. Los pings programados se detienen con la simulación y se emiten como máximo una vez por actualización.

## Tiempo de propagación

Cada consulta tiene dos trayectos: llegada de la petición al transpondedor y retorno de la posición al transceptor. Cada retardo es distancia dividida por la velocidad del sonido del dispositivo correspondiente.

- Los retardos fraccionarios siguen el tiempo simulado, se pausan con el mundo y se cancelan al retroceder el tiempo.
- Cada trayecto admite una muestra pendiente por transpondedor; las peticiones o respuestas adicionales se ignoran.
- El intervalo entre pings debe superar el tiempo de ida y vuelta esperado para recibir cada consulta.

La velocidad efectiva es `sound_speed + 0.017 × depth + 4 × (temperature − 10)`. Usa m/s, profundidad en m bajo z = 0 y temperatura en °C. Sobre la superficie, la profundidad es cero. La temperatura predeterminada es 10 °C.

Publica `std_msgs/msg/Float64` en `/USBL/transceiver_manufacturer_168/temperature` o `/USBL/transponder_manufacturer_1/temperature`. El cambio afecta a trayectos posteriores. Se ignoran temperaturas no finitas o cambios que produzcan una velocidad no positiva o no finita.

La aproximación sigue el [resumen de velocidad del sonido de DOSITS](https://dosits.org/decision-makers/tutorials/science/sound-speed/). Usa distancias rectas, sin refracción, salinidad, multitrayecto ni interferencias. El retorno usa la posición recibida, incluido su ruido configurado.

## Consultar todos los transpondedores

{{CODE:usbl:4}}

El modo `common` emite una difusión por ping programado y puede devolver varios IDs en el mismo topic. Conserva el ID de cada respuesta. El transceptor retiene el modo para los transpondedores que se incorporen después.

## Seleccionar un transpondedor

{{CODE:usbl:5}}

Cambiar de canal selecciona el modo `individual`; cada ping programado se dirige a ese ID. Vuelve a `common` para consultar todos los dispositivos.

## Dispositivos y modos de consulta

{{MEDIA:usbl:1}}
{{MEDIA:usbl:2}}
{{MEDIA:usbl:3}}

Imágenes: wiki Notion de POSIM; atribución DAVE en [Citas y licencias](citation.md). Usa los comandos y topics de esta página.
