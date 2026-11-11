# Corrientes oceánicas

POSIM genera corrientes en el mundo, calcula el flujo en cada robot y permite ajustarlo mediante ROS. La corriente uniforme y el perfil que varía con la profundidad tienen configuraciones y tópicos distintos.

## Ejecutar el ejemplo

Prepara los [terminales A y B](terminals.md) en tu entorno POSIM: Ubuntu nativo, WSL, Docker o el escritorio remoto de macOS. Inicia el ejemplo en A:

{{CODE:currents:0}}

En B, consulta primero Gazebo y después ROS. Pulsa Ctrl+C para detener el primer visor antes de ejecutar el segundo comando:

{{CODE:currents:1}}

{{CODE:currents:2}}

## Configurar la corriente del mundo

Coloca ambos plugins directamente en `<world>`, primero el de corriente y después el de ROS. El [mundo de ejemplo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world) muestra la configuración completa.

{{CODE:currents:3}}

Ambos necesitan el mismo `namespace`, no vacío. Incluye los bloques `constant_current` y `transient_current`; si falta una configuración obligatoria, se desactiva la publicación y se informa del error. La corriente uniforme tiene modelos de Gauss–Markov separados para la velocidad y los ángulos horizontal y vertical.

| Parámetro | Función y valor predeterminado |
| --- | --- |
| `namespace` en ambos plugins | Espacio de nombres compartido; `hydrodynamics` en el ejemplo |
| `constant_current/use_constant_current` | Publicar corriente uniforme; `false` |
| `constant_current/topic` | Tópico Gazebo uniforme; `ocean_current` |
| `transient_current/topic_stratified` | Tópico Gazebo por capas dentro del espacio de nombres; `stratified_current_velocity` |
| `transient_current/databasefileName` | CSV instalado en `posim_worlds/worlds`; `transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | Ruta CSV opcional; tiene prioridad sobre el nombre de archivo |
| `mean` de cada modelo estocástico | Media inicial de velocidad (m/s) o ángulo (rad); 0 |
| `min`, `max` | Límites; −1 y 1. Mantén `min < max` y la media dentro del intervalo |
| `noiseAmp` | Amplitud del incremento aleatorio; 0 |
| `mu` | Coeficiente de decaimiento de Gauss–Markov; 0 |
| `tidal_oscillation` | Bloque opcional de datos de marea |

## Preparar un perfil de profundidad

El [CSV de ejemplo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv) contiene velocidad en el eje X del mundo (m/s), velocidad en el eje Y del mundo (m/s) y profundidad (m), en ese orden. Los nombres heredados `velocity_north`, `velocity_east` y `velocity_down` corresponden directamente a los ejes X, Y y Z del mundo.

- Conserva las tres primeras filas descriptivas y de cabecera.
- Incluye al menos dos capas, con tres números finitos por fila.
- Usa profundidades no negativas y estrictamente crecientes.
- Un archivo ausente o incorrecto desactiva la publicación de corriente.

El plugin ROS del mundo transmite la base de datos completa. El plugin del modelo interpola la velocidad a la profundidad del vehículo; la hidrodinámica usa el flujo relativo al movimiento del robot.

## Aplicar el perfil al robot

El ejemplo de REXROV usa inicialmente `/ocean_current`, uniforme. Para aplicar el perfil, activa una sola vez el bloque comentado del [modelo REXROV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf), directamente en `<model>`:

{{CODE:currents:4}}

Añade también `<namespace>rexrov</namespace>` al plugin existente `gz-sim-hydrodynamics-system`, conservando su enlace, densidad y coeficientes de arrastre. Vuelve a compilar el espacio de trabajo y carga su entorno.

| Parámetro del modelo | Función y valor predeterminado |
| --- | --- |
| `namespace` | Obligatorio; debe coincidir con el de hidrodinámica, aquí `rexrov` |
| `flow_velocity_topic` | Sufijo bajo `/model/<namespace>/`; `ocean_current` |
| `transient_current/topic_stratified` | Entrada ROS de la base de datos; `hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | Ajustes estocásticos opcionales por componente; las componentes omitidas no añaden ruido |
| `mean` de cada componente | Velocidad inicial, m/s; 0. Al recibir datos se sustituye por el perfil interpolado |
| `mu` | Decaimiento de 0 a 1, aplicado según los segundos de simulación transcurridos; 0 |
| `noiseAmp` | Desviación permitida respecto a la velocidad interpolada, m/s; 0 |
| `noiseFreq` | Amplitud del incremento aleatorio por actualización; 0. No es una frecuencia en Hz |
| `tide_oscillation` | Aplicar los datos de marea al modelo; `false` |

El resultado se publica en `/model/rexrov/ocean_current`: `gz.msgs.Vector3d` en Gazebo y `geometry_msgs/msg/TwistStamped` en ROS. Comprueba tanto este dato como la respuesta del vehículo.

La profundidad se obtiene de la posición del modelo en el mundo; por encima del agua se usa cero. El flujo se interpola entre capas y, fuera del intervalo, toma la capa más cercana. Las marcas de tiempo repetidas o la pausa no hacen avanzar el modelo. Un mensaje de base de datos inválido se rechaza completo y se conserva el último perfil válido.

## Sistemas y tópicos

| Sistema | Responsabilidad |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | Generar la corriente del mundo y los datos por profundidad |
| Plugin de corriente del modelo | Interpolar la corriente en la posición del robot |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | Publicar datos ROS y ofrecer servicios de configuración |

| Tópico del ejemplo | Transporte y tipo |
| --- | --- |
| `/ocean_current` | Gazebo, `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo, datos por capas |
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS, `posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS, `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS, `posim_interfaces/msg/StratifiedCurrentDatabase` |

Comprueba los nombres del SDF y conecta la hidrodinámica a la fuente de corriente prevista.

## Ajustar la corriente con servicios ROS

En B, consulta los servicios y solicita una corriente horizontal de 0.3 m/s:

{{CODE:currents:5}}

La velocidad se expresa en m/s y los ángulos en radianes. Comprueba `success` y los datos publicados. Para consultar la interfaz de una capa:

{{CODE:currents:6}}

Lee el modelo, cambia sus límites y ajusta la primera capa:

{{CODE:currents:7}}

| Servicio, bajo `/hydrodynamics` | Interfaz | Campos |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | Solicitud vacía |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

### Límites de las solicitudes

- Cada cambio se aplica completo o devuelve `success: false` sin modificar la corriente.
- Los valores deben ser finitos y respetar los límites configurados. Los límites de velocidad son no negativos; los angulares se mantienen en ±π horizontalmente y ±π/2 verticalmente.
- Los ajustes del modelo afectan al modelo global y a todas las capas, conservando sus medias. Los nuevos límites deben contener también esas medias.
- `layer` empieza en 0. El campo ROS `noise` corresponde a `noiseAmp` en SDF.

## Incorporar datos de marea

Configura `<tidal_oscillation>` con un CSV o componentes armónicas, como M2/S2/N2, las direcciones de flujo y reflujo y una hora inicial GMT. Las unidades de los ejemplos armónicos son metros para la amplitud, grados para la fase y grados/hora para la velocidad angular. No las confundas con los ángulos en radianes de los servicios.

En el modo de mareas, la señal armónica calculada, o la velocidad del CSV dividida entre 100, se multiplica por la primera columna del perfil de profundidad. Las amplitudes de altura del ejemplo se usan como factores numéricos; el modelo no convierte la altura de marea en velocidad de corriente. Calibra conjuntamente la señal y el perfil para obtener la velocidad prevista.

El CSV de mareas tiene una cabecera y filas `YYYY-MM-DD HH:MM,event,speed`:

- Eventos: `flood`, `ebb` o `slack`. La velocidad debe ser finita y estar en cm/s; se admite `-` en filas `slack` que se omiten.
- Las fechas deben ser válidas y estrictamente crecientes.
- El lector conserva picos positivos y negativos alternados y necesita al menos dos. Fuera del intervalo mantiene el primer o último pico.
- Los archivos ausentes, los ajustes incompletos o los datos mal formados desactivan el plugin de corriente del mundo.

Estos datos son entradas del experimento. Conserva sus unidades, intervalo de interpolación, origen del mundo y referencia temporal. Compara corriente y movimiento del vehículo con la misma pose inicial.

## Flujo y respuesta del vehículo

{{MEDIA:currents}}

Imágenes y vídeos: POSIM Notion Wiki. Consulta las [referencias y licencias](citation.md) para la atribución de DAVE. Usa los comandos y tópicos de esta página.
