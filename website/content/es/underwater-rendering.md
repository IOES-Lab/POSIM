# Efectos de escena subacuática

`posim_gz_rendering` aplica turbidez con la distancia, absorción RGB, luz solar dependiente de la profundidad, cáusticas y partículas suspendidas a las superficies PBR de Ogre2. Las partículas representan nieve marina y empiezan **desactivadas**.

La escena Gazebo, las cámaras RGB y el color RGBD comparten estos efectos. La profundidad numérica y las colisiones se mantienen. El filtro independiente basado en profundidad se explica en [Cámara subacuática](camera.md).

## Compilar e incorporar el sistema

Prepara un entorno de desarrollo Gazebo Jetty/Ogre2 compatible con el servidor. En la [terminal A preparada](terminals.md), compila desde el código de POSIM; en macOS, usa la terminal Linux del escritorio RDP:

{{CODE:underwater-rendering:0}}

CMake necesita los paquetes de desarrollo Gazebo y las entradas pkg-config `OGRE-Next` y `OGRE-Next-Hlms`. El comando incluye la ruta del paquete ROS `gz_ogre_next_vendor` en Ubuntu. Usa bibliotecas, cabeceras y sombreadores de la misma compilación.

Añade este plugin a `<world>` y sustituye `/absolute/underwater` por el prefijo de instalación anterior. Carga una sola instancia por proceso Gazebo y asigna un `session_nonce` distinto a cada sesión:

{{CODE:underwater-rendering:1}}

Linux usa `.so`, también dentro de Docker/RDP en macOS. Las bibliotecas compiladas directamente para macOS usan `.dylib`. La escena necesita una cámara activa para generar imágenes.

Si también usas `UnderwaterCamera`, configura `POSIM_UNDERWATER_PLUGIN` con la ruta del sistema cargado en el entorno del servidor. Así pasan los píxeles ya coloreados sin segunda atenuación. Actívalo únicamente cuando el sistema de escena esté instalado y cargado.

## Efectos y valores predeterminados

| Ajuste | Efecto | Predeterminado |
|---|---|---|
| Óptica del agua | Interruptor general de los efectos ópticos | Activada |
| Turbidez | Mezcla de color según el recorrido bajo el agua | Activada |
| Absorción RGB | Pérdida de color según longitud de onda | Activada |
| Atenuación solar | Luz direccional más débil con la profundidad | Activada |
| Cáusticas | Patrones animados de luz sobre superficies sumergidas | Activadas |
| Partículas / nieve marina | Puntos brillantes procedimentales dispersos | Desactivadas |
| Textura del fondo | Detalle de los mapas PBR del fondo | Activada |
| Cuadrícula del fondo | Separación 10 m, ajustable de 1 a 500 m | Desactivada |
| Curvas de profundidad | Separación 5 m, ajustable de 1 a 50 m | Desactivadas |

Desactivar la óptica conserva las selecciones de textura, cuadrícula y curvas. Las superposiciones reconocen visuales llamados `seabed` o `shallow`, incluidos sus nombres completos de Gazebo. Incluye las texturas en los recursos del terreno; consulta [Terreno con mapas de altura](heightmaps.md).

<a id="turn-on-marine-snow"></a>
## Activar la nieve marina

Con el mundo iniciado, consulta el estado desde la terminal B preparada:

{{CODE:underwater-rendering:2}}

Los primeros 13 valores siguen este orden; los índices empiezan en cero:

| Índice | Valor |
|---|---|
| 0–1 | Esquema `1`, revisión |
| 2–5 | Óptica, textura del fondo, cuadrícula, curvas |
| 6–10 | Turbidez, absorción, atenuación solar, cáusticas, partículas |
| 11–12 | Separación de cuadrícula y curvas, m |

En un sistema recién iniciado con revisión `0`, esta solicitud activa las partículas y conserva los demás valores predeterminados:

{{CODE:underwater-rendering:3}}

Para cambios posteriores, conserva los valores deseados y aumenta la revisión en uno. Los booleanos aceptan `0` o `1`. Puedes repetir una solicitud idéntica.

Después de renderizar, vuelve a leer `/state`: añade `ready`, número de cámaras y `renderer_failed` en 13–15. Confirma la revisión solicitada, `ready = 1` y `renderer_failed = 0`. La respuesta de `/set` solo confirma recepción.

Para activar partículas desde el inicio, añade esta etiqueta al plugin en lugar de la primera solicitud:

{{CODE:underwater-rendering:4}}

WWW-POSIM utiliza este renderizador público y añade su propio panel de apariencia, gestión de sesiones y preferencias.

## Alcance del modelo y comparación

El cálculo de la longitud del recorrido óptico bajo el agua usa el plano del nivel medio del mar en **z = 0 m**, sin seguir cada cresta. Cáusticas y partículas son aproximaciones visuales; no modelan concentraciones calibradas ni ruido óptico del sensor.

Las luces locales conservan iluminación directa. Los rayos del cielo reciben atenuación subacuática; los materiales sin iluminación y los sombreadores propios de superficie oceánica mantienen su renderizado. La cara inferior de la ola depende del material de superficie.

Compara vistas sobre/bajo el agua, RGB y color RGBD, y cada interruptor. Mantén posición y luz constantes. Comprueba profundidad idéntica byte a byte, pausa de efectos con el tiempo simulado y notificación de fallos. El [paquete público](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering) incluye ejemplos de comprobación RGB/RGBD, sombreadores y detalles de compilación.
