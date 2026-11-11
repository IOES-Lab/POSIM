# Mundos disponibles

Los mundos están en `models/posim_worlds/worlds`. Selecciónalos por su nombre de archivo, sin la extensión `.world`.

## Iniciar un mundo

Usa la [terminal POSIM de tu instalación](terminals.md), tanto en Ubuntu nativo como en WSL, Docker o RDP.

{{CODE:worlds:0}}

El nombre de archivo puede diferir del nombre interno: `posim_ocean_waves.world` declara `oceans_waves`. Este último aparece en tópicos y servicios Gazebo. Comprueba `<world name="…">` antes de construir una ruta.

## Elegir según el experimento

| Experimento | Mundo |
|---|---|
| Primer vehículo | `posim_ocean_waves` |
| Efectos de cámara submarina | `camera_tutorial` |
| Mediciones DVL | `dvl_world` |
| Interrogación USBL | `usbl_tutorial` |
| Configuración de corrientes | `ocean_current_plugin` |
| Escena costera y geográfica | `posim_Santorini` |
| Sonar | `posim_multibeam_sonar` y mundos oceánicos con sonar en el nombre |
| Manipulación | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

Los mundos de sonar requieren su [entorno de ejecución](sonar-tuning.md). Las referencias Fuel pueden descargar modelos en el primer uso.

## Archivos SDF

Abre un archivo para consultar sus sistemas, modelos, origen, luces y cámaras.

| SDF del mundo |
|---|
{{WORLD_CATALOG}}

## Crear otro entorno

Copia un SDF adecuado en `models/posim_worlds/worlds` y asigna nombres nuevos al archivo y al mundo. Utiliza URI que resuelvan los recursos instalados. Vuelve a compilar, carga el entorno y ejecuta el nuevo archivo. Consulta [Terrenos con mapas de alturas](heightmaps.md) y [Objetos](objects.md).

## Esperar las descargas de Fuel

`posim_integrated` y `posim_ocean_models` usan modelos remotos. Espera a que termine la primera descarga; evita iniciar varias simulaciones durante la carga. `verbose:=true` muestra los mensajes de recursos.

En otra terminal con el mismo entorno, consulta la escena cargada:

{{CODE:worlds:1}}

Comprueba los modelos de la respuesta, el avance del reloj y las mediciones. Si la descarga se detiene, revisa el mensaje de carga y la red del equipo o contenedor que ejecuta Gazebo.

## Consultar los argumentos de lanzamiento

{{CODE:worlds:2}}

| Argumento | Función | Valor inicial del lanzamiento de sensores |
|---|---|---|
| `namespace` | Directorio del modelo instalado | Vacío; especifícalo |
| `world_name` | Archivo del mundo | `empty.sdf` |
| `paused` | Iniciar en pausa | `true` |
| `gui` / `headless` | Mostrar / ocultar la interfaz Gazebo | `true` / `false` |
| `use_sim_time` | Usar el reloj simulado en ROS | `true` |
| `x`, `y`, `z` | Posición inicial, m | `0.0` cada uno |
| `roll`, `pitch`, `yaw` | Orientación inicial, rad | `0.0` cada uno |
| `debug` / `verbosity_level` | Registro detallado / nivel | `false` / `1` |
| `use_ned_frame` | Publicar la transformación `world_ned` (ENU a NED) | `false` |

Usa `paused:=false` para que los sensores produzcan datos. Consulta `--show-args` del lanzamiento concreto para ver las opciones de robots y mundos.
