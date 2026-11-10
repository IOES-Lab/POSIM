# Terrenos con mapas de alturas

Un mapa de alturas representa el terreno mediante una cuadrícula regular de elevaciones. Configura juntas las geometrías visual y de colisión para que el fondo visto por la cámara coincida con la superficie de contacto.

## Entorno de trabajo

Usa las [terminales Linux preparadas](terminals.md) en Ubuntu nativo, WSL o Docker. En macOS, trabaja dentro del escritorio RDP. Edita y compila en ese entorno; `POSIM_WS` es su espacio de trabajo.

## Consultar ejemplos

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) muestra una escena geográfica con el modelo Fuel `Santorini Scaled`. El [fondo escalonado](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world) ofrece otro ejemplo de composición.

Para crear la geometría, consulta el [tutorial de Gazebo](https://gazebosim.org/api/sim/10/heightmap_dem.html) y la [referencia SDFormat](http://sdformat.org/spec?ver=1.12&elem=geometry).

## Guardar recursos en el paquete

Coloca el mundo en `models/posim_worlds/worlds` y los datos en `models/posim_worlds/media`. Comprueba las reglas de instalación CMake y las rutas de recursos del paquete. Usa URI portables de recursos instalados.

Dentro de una geometría visual o de colisión, la estructura general es:

{{CODE:heightmaps:0}}

Sustituye la URI por tu propio recurso. Elige formato ráster y muestreo compatibles con el renderizador y el motor físico. Comprueba cómo se codifica la elevación antes de ajustar tamaño vertical y desplazamiento.

## Conservar escala y coordenadas

`size` expresa extensión horizontal y escala vertical en metros; `pos` añade un desplazamiento. Una imagen normalizada no queda georreferenciada automáticamente. Registra extensión de origen, rango de elevación, referencia del nivel del mar y cualquier reproyección o remuestreo.

Si importan las posiciones geográficas, define `<spherical_coordinates>` en el mundo. [Coordenadas esféricas](coordinates.md) explica la conversión entre latitud/longitud y coordenadas locales.

## Compilar y comprobar

Recompila y carga el entorno después de añadir recursos. Inicia el mundo desde fuera del repositorio. Comprueba:

- Elevaciones reconocibles y coincidencia visual/colisión.
- Profundidad medida por los sensores.
- Contacto mediante una prueba controlada de caída.
- Nivel del mar y altura inicial del robot.

En Jetty, un `<heightmap><pos>` distinto de cero puede desplazar el terreno visible sin producir el mismo desplazamiento de colisión. Trasladar el desplazamiento a la pose del modelo también puede dejar diferencias. Verifica ambos con tu renderizador y motor físico. Si persisten, elige una geometría o motor que supere las pruebas de contacto y visualización antes de usarlo para navegación.
