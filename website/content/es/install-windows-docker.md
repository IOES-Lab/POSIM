# Windows: WSL y Docker

Docker Desktop ejecuta POSIM y WSLg muestra Gazebo en el escritorio de Windows. Ejecuta los comandos Docker de esta página en la **terminal de Ubuntu dentro de WSL**.

## 1. Preparar WSL y los gráficos

Completa los dos primeros pasos de [instalación en Windows](install-windows.md): instala Ubuntu 26.04 en WSL2 y actualiza WSL y el controlador gráfico de **Windows**. Esto también se aplica a NVIDIA. No instales un controlador de pantalla de Linux dentro de WSL.

Instala [Docker Desktop con WSL2](https://docs.docker.com/desktop/features/wsl/) y activa **Settings → Resources → WSL Integration → Ubuntu-26.04**. Utiliza su motor Docker; no necesitas instalar otro dentro de Ubuntu.

Comprueba el servidor y la conexión con la pantalla:

{{CODE:install-windows-docker:0}}

Si falta el socket, termina la [configuración de WSLg](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps).

## 2. Descargar POSIM

{{CODE:install-windows-docker:1}}

Consulta las versiones y compilaciones en [Entorno Docker](docker.md). Esta configuración utiliza renderizado por software. Para una GPU NVIDIA compatible, revisa los [requisitos de acceso a la GPU](docker.md#graphics-and-gpu-access).

## 3. Crear el contenedor

{{CODE:install-windows-docker:2}}

Guarda tus archivos en `/work`, dentro del volumen `posim-data`. Puedes detener el contenedor con `docker stop posim` y reanudarlo con `docker start posim`.

Para los ejemplos siguientes, abre [dos terminales de Ubuntu](terminals.md) y conecta ambas al mismo contenedor. Las ventanas de Gazebo aparecen directamente en Windows.

## 4. Ejecutar el mundo de demostración

Entra en el contenedor:

{{CODE:install-windows-docker:3}}

Comprueba la instalación ejecutando:

{{CODE:install-windows-docker:4}}

Gazebo debe mostrar el océano. Mantén abierta la terminal; **Ctrl+C** detiene la simulación. Continúa con [tu primer robot](quickstart.md).
