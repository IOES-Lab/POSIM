# Linux: instalación con Docker

Usa la imagen publicada de POSIM si tu escritorio tiene otra versión de Ubuntu o una distribución diferente. Esta guía utiliza **AMD64 en un equipo x86-64**, con X11 o XWayland. En Ubuntu 26.04 recomendamos la [instalación nativa](install.md).

## 1. Preparar Docker y la pantalla

Instala [Docker Engine para tu distribución](https://docs.docker.com/engine/install/); en Ubuntu, sigue [estos pasos](https://docs.docker.com/engine/install/ubuntu/). Si tu usuario no puede ejecutar `docker`, completa la [configuración posterior a la instalación](https://docs.docker.com/engine/install/linux-postinstall/).

Abre una terminal en tu sesión de escritorio e instala `xauth`. En Ubuntu:

{{CODE:install-ubuntu-docker:0}}

Comprueba la dirección de la pantalla y la versión del servidor:

{{CODE:install-ubuntu-docker:1}}

## 2. Descargar POSIM

{{CODE:install-ubuntu-docker:2}}

Consulta [Docker](docker.md) para elegir una revisión o compilar tus cambios. La primera ejecución utiliza renderizado por software y puede ser más lenta que una instalación nativa con GPU.

## 3. Autorizar el acceso a tu pantalla

Copia la autorización X11 de esta sesión a un archivo que solo pueda leer tu usuario:

{{CODE:install-ubuntu-docker:3}}

El último comando comprueba que el archivo contiene datos. Si falla, verifica que la terminal pertenece a tu sesión gráfica y que XWayland está disponible. Corrige la autorización antes de continuar.

## 4. Crear el contenedor

{{CODE:install-ubuntu-docker:4}}

Guarda tus modelos y grabaciones en `/work`, dentro del volumen `posim-data`. Para detener y reanudar el contenedor, utiliza `docker stop posim` y `docker start posim`. Si la pantalla deja de funcionar después de volver a iniciar sesión en Linux, actualiza el archivo de autorización.

Para los ejercicios, [prepara dos terminales](terminals.md) conectadas a este mismo contenedor. Antes de activar aceleración gráfica o sonar CUDA, revisa la [configuración de GPU](docker.md#graphics-and-gpu-access).

## 5. Comprobar el mundo de demostración

Entra en el contenedor:

{{CODE:install-ubuntu-docker:5}}

Ejecuta dentro de él la comprobación final:

{{CODE:install-ubuntu-docker:6}}

Gazebo debe abrir el mundo oceánico. Mantén la terminal abierta mientras funciona y pulsa **Ctrl+C** para detenerlo. Continúa con [tu primera simulación](quickstart.md).
