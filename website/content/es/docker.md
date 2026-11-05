# Entorno Docker

Las imágenes de POSIM incluyen Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, MAVROS y QGroundControl. Elige primero la [guía de tu sistema operativo](requirements.md) para configurar la pantalla y los controladores.

## Descargar una imagen publicada

Las imágenes están en [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags). Docker selecciona automáticamente AMD64 o ARM64.

{{CODE:docker:0}}

| Etiqueta | Cuándo utilizarla |
|---|---|
| `latest` | Compilación publicada de la rama principal, para ambas arquitecturas |
| `sha-<12-character-commit>` | Una revisión concreta del código; copia una etiqueta existente de Docker Hub |
| Sufijo `-amd64` o `-arm64` en la etiqueta SHA | Una arquitectura concreta |
| Versión de lanzamiento, si está publicada | Una versión etiquetada del proyecto |

Consulta la revisión completa y guarda el digest si necesitas repetir un experimento con exactamente la misma imagen.

{{CODE:docker:1}}

Utiliza la documentación correspondiente a esa revisión. Descargar una imagen nueva no modifica los contenedores existentes.

## Compilar desde el código fuente

Ejecuta estos comandos en el sistema anfitrión:

{{CODE:docker:2}}

Para AMD64:

{{CODE:docker:3}}

Para ARM64:

{{CODE:docker:4}}

Sustituye `ioeslab/posim:latest` por tu etiqueta local en la guía de instalación. Ambas imágenes compilan el código del repositorio y la revisión fijada de la biblioteca de olas. ARM64 incluye además un escritorio XFCE y un servidor RDP.

## Abrir el entorno

- [Linux con Docker](install-ubuntu-docker.md): utiliza la pantalla del escritorio Linux.
- [Windows con WSL y Docker](install-windows-docker.md): muestra las ventanas mediante WSLg.
- [macOS con Docker y RDP](install-macos.md): accede al escritorio Linux remoto.

Las tres guías crean un contenedor llamado `posim`. Consulta [cómo preparar las terminales](terminals.md) antes de ejecutar los ejemplos.

## Conservar tus archivos

El espacio de trabajo está en `/opt/posim_ws` en AMD64 y en `/home/docker/posim_ws` en ARM64. Las variables `POSIM_WS` o `POSIM_UNDERLAY` indican su ubicación.

Guarda tus modelos SDF y grabaciones en `/work`, que corresponde al volumen `posim-data`. Los demás archivos del contenedor sobreviven a una parada, pero se pierden al eliminarlo. Guarda los cambios del código en Git o cópialos fuera antes de sustituir el contenedor. El volumen permanece hasta que lo elimines expresamente.

## Detener, reanudar y actualizar {#update-an-existing-container}

Detén primero la simulación con **Ctrl+C**. Después, desde el anfitrión:

{{CODE:docker:5}}

Para actualizar, guarda tu trabajo, detén el contenedor, descarga la imagen elegida y elimina el contenedor detenido. Vuelve a ejecutar el comando de creación de tu guía, reutilizando `posim-data`.

## Gráficos y GPU {#graphics-and-gpu-access}

En Linux, configura el controlador del anfitrión y [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). En Windows, sigue la [configuración de GPU para Docker Desktop y WSL2](https://docs.docker.com/desktop/features/gpu/). El sonar CUDA también necesita sus bibliotecas dentro de la imagen: consulta [configuración del sonar](sonar-tuning.md). Las instrucciones iniciales utilizan renderizado por software, que puede ser más lento.
