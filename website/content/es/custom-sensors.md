# Añadir y configurar sensores

Un sensor necesita su definición en el modelo, el sistema que lo actualiza y una ruta hasta la aplicación receptora. Añade uno cada vez y comprueba sus mediciones.

## Entorno de trabajo

Usa las [terminales Linux preparadas](terminals.md) en Ubuntu nativo, WSL o Docker. En macOS, trabaja dentro del escritorio RDP. Edita y compila allí; `POSIM_WS` indica ese espacio de trabajo.

## Elegir dónde integrarlo

| Medición | Ubicación en el modelo | Sistema del mundo / conexión ROS |
|---|---|---|
| IMU | `link/sensor type="imu"` | Sistema IMU; puente de `gz.msgs.IMU` a ROS `Imu` |
| [Cámara submarina](camera.md) | Sensor RGBD y plugin de efectos | Sistema Sensors con renderizador y tópicos de cámara |
| [DVL](dvl.md) | Sensor DVL y configuración de haces | Sistema DVL y puente de mensajes acústicos incluido |
| [Presión](pressure.md) | Plugin directamente bajo `model` | Publicación ROS directa de presión y profundidad |
| [GNSS marino](gnss.md) | Plugin del receptor en el modelo | Salidas ROS configuradas del receptor |
| [USBL](usbl.md) | Plugins del transceptor y transpondedor | Identificadores acústicos compatibles e interfaces ROS |
| [Sonar multihaz](sonar.md) | Sensor personalizado bajo un eslabón | Renderizador Sensors, sistema multihaz del mundo y CUDA |

Consulta el SDF y los parámetros de cada referencia. Respeta su ubicación: los sistemas del mundo van en el mundo; los plugins de modelo necesitan la entidad sobre la que actúan.

## Añadir una IMU al mundo de ejemplo

Abre `calm_water.sdf` de [Crear un mundo](custom-worlds.md). Añade este sistema directamente en `<world>`, junto al sistema de física:

{{CODE:custom-sensors:0}}

Dentro de `<link name="base_link">` del modelo `pressure_station`, añade:

{{CODE:custom-sensors:1}}

La pose es relativa al eslabón, en metros y radianes. La frecuencia usa tiempo simulado. Como la estación está fija, la velocidad angular debe ser cercana a cero; la aceleración incluye la respuesta a la gravedad.

## Conectar y observar

Detén el mundo anterior. En la terminal A, valida y reinicia:

{{CODE:custom-sensors:2}}

En la **terminal B**, con el entorno cargado, inicia el puente y déjalo ejecutándose:

{{CODE:custom-sensors:3}}

En la **terminal C**, preparada del mismo modo, consulta una medida:

{{CODE:custom-sensors:4}}

Comprueba marca temporal, identificador del sistema de referencia, velocidad angular y aceleración. Quita `--once` para observar continuamente; Ctrl+C detiene la lectura. Al terminar, detén puente y simulador. [Terminales y tópicos ROS](ros.md) explica cómo comparten el entorno.

## Cambiar la frecuencia

Cambia `update_rate` de `20` a `5` y reinicia mundo y puente. Los intervalos entre marcas temporales deben pasar de unos **0,05 s a 0,2 s** de tiempo simulado. La frecuencia recibida en tiempo real depende también del factor de tiempo real y del transporte.

En cámaras, revisa juntos resolución, campo de visión y distancias cercana y lejana. En sensores de barrido, comprueba haces y límites de alcance. Antes de mover un sensor, confirma su sistema de referencia y orientación.

## Comprobar la integración

- Modelo: pose válida, configuración y tópico único.
- Mundo: sistema requerido y renderizador para sensores de imagen.
- ROS: dirección del puente, tipo de mensaje, sistema de referencia y QoS.
- Datos: marcas temporales recientes y respuesta a un cambio controlado.

Verifica el fondo con [Terrenos](heightmaps.md). Para comparar sonar, usa [Configuración y rendimiento](sonar-tuning.md) junto con sus [parámetros](sonar.md).
