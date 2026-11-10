# Crear un mundo

Empieza con un mundo pequeño y un sensor de presión a profundidad conocida. Así podrás modificar el entorno y medir el resultado antes de añadir terreno y un robot móvil.

## Preparar el entorno de trabajo

Usa las [terminales Linux preparadas](terminals.md) en Ubuntu nativo, WSL o Docker. En macOS, abre la terminal del escritorio RDP. Edita y compila dentro de ese entorno; `POSIM_WS` indica su espacio de trabajo.

## Guardar el mundo completo

Guarda este contenido como `calm_water.sdf`. Utiliza geometría básica y plugins instalados; no necesita descargar mallas.

{{CODE:custom-worlds:0}}

El mundo contiene:

- **Física:** paso máximo de 0,001 s, factor de tiempo real objetivo de 1 y gravedad descendente.
- **Iluminación y escena:** luz solar direccional, luz ambiente y color de fondo.
- **Fondo marino:** caja fija en z = −20 m, con geometrías visual y de colisión coincidentes.
- **Estación de presión:** marcador fijo en z = −2 m. Su plugin de [presión](pressure.md), asociado al modelo, publica presión y profundidad estimada.

La referencia de presión es el plano z = 0 del mundo. Añade oleaje, flotabilidad y movimiento después de comprobar este caso básico.

## Validar y ejecutar

En la terminal A, desde el directorio del archivo:

{{CODE:custom-worlds:1}}

La validación debe completarse y abrirse Gazebo. `-r` inicia el tiempo simulado. Selecciona `pressure_station` en el árbol de entidades y centra la vista en ella.

En otra terminal con el mismo entorno cargado:

{{CODE:custom-worlds:2}}

Espera aproximadamente **120 938 Pa** y **2 m**. El plugin publica directamente en ROS; este ejemplo no requiere un puente adicional.

## Cambiar la profundidad

1. Detén la terminal A con Ctrl+C.
2. Cambia la pose de la estación de `0 0 -2 0 0 0` a `0 0 -4 0 0 0`.
3. Valida y vuelve a iniciar el mundo.
4. Repite las dos consultas. Debes obtener unos **140 551 Pa** y **4 m**.

Mantén los coeficientes de presión. El incremento esperado es **19 612,76 Pa**. Conserva ambos archivos junto con tus observaciones.

## Ubicar cada ajuste

| Ruta SDF | Unidad | Función |
|---|---|---|
| `world/physics/max_step_size` | s | Paso máximo: `0.001` |
| `world/physics/real_time_factor` | — | Relación objetivo entre tiempo simulado y real: `1.0` |
| `world/gravity` | m/s² | Vector de gravedad |
| `world/scene`, `world/light` | RGBA / dirección | Fondo e iluminación |
| `model/pose` | m, rad | Posición y orientación |
| `model/static` | booleano | Mantener fijos estación y fondo |
| `model/plugin` | según el plugin | Parámetros de [presión](pressure.md) |

La velocidad real depende de la carga. La definición de un sensor y su sistema del mundo tienen ámbitos distintos; [Añadir sensores](custom-sensors.md) incorpora una IMU y su puente ROS a este archivo.

## Ampliar el mundo

- [Terrenos](heightmaps.md): sustituir el fondo y comprobar colisiones.
- [Añadir un robot](custom-robots.md): mecánica, propulsión y ROS.
- [Corrientes](currents.md): flujo del mundo y respuesta del modelo.
- [Coordenadas esféricas](coordinates.md): origen geográfico.
- [Efectos submarinos](underwater-rendering.md): representación visual.

Guarda los mundos distribuibles en `models/posim_worlds/worlds` y sus recursos en el mismo paquete. Recompila, carga el entorno y usa URI de recursos instalados al compartirlos.
