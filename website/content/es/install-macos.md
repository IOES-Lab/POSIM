# macOS: Docker y RDP

En un Mac con Apple Silicon, ejecuta la imagen ARM64 de POSIM y conéctate a su escritorio Linux mediante RDP. Gazebo y las terminales de los ejemplos se abren dentro de ese escritorio.

## 1. Preparar Docker y el cliente RDP

Instala y abre [Docker Desktop para Mac](https://docs.docker.com/desktop/setup/install/mac-install/). Para la conexión remota, instala [Windows App para macOS](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps).

En la aplicación Terminal de macOS:

{{CODE:install-macos:0}}

Apple Silicon debe mostrar `arm64`. El segundo comando debe indicar la versión del servidor Docker.

## 2. Descargar y crear el contenedor

Docker selecciona automáticamente ARM64:

{{CODE:install-macos:1}}

El puerto RDP solo queda disponible en este Mac. Guarda tus modelos y grabaciones en `/work` para conservarlos en el volumen `posim-data`. Consulta [las versiones de imagen y la compilación local](docker.md).

## 3. Conectarse al escritorio Linux

En Windows App, añade un equipo con la dirección **127.0.0.1:3389**. En la pantalla de xrdp, selecciona **Xorg** e inicia sesión con el usuario **docker** y la contraseña **docker**.

Dentro del escritorio Linux, abre una terminal y cambia la contraseña de esta cuenta de desarrollo local:

{{CODE:install-macos:2}}

Ejecuta los comandos de POSIM en este escritorio. Usa la Terminal de macOS para administrar Docker. [Preparar las terminales](terminals.md) explica dónde está el espacio de trabajo.

## 4. Detener y volver a entrar

Cerrar RDP no detiene la simulación. Pulsa primero **Ctrl+C** en la terminal que la ejecuta. Después, desde macOS:

{{CODE:install-macos:3}}

Vuelve a conectarte con Windows App. El renderizado es por software y las escenas complejas pueden ser lentas. El sonar multihaz CUDA requiere una GPU NVIDIA compatible y no está incluido en esta imagen ARM64. Empieza con los [ejemplos de cámara y presión](features.md).

## 5. Comprobar la instalación

En una terminal **dentro del escritorio RDP**, ejecuta la comprobación final:

{{CODE:install-macos:4}}

Gazebo debe mostrar el océano. Mantén la terminal abierta mientras se ejecuta. Detén la simulación con **Ctrl+C** y continúa con [tu primer robot](quickstart.md).

{{MEDIA:install-macos}}
