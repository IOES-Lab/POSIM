# Preparar los terminales

Un **terminal** es una ventana donde se escriben comandos y se consulta su salida. Usaremos dos ventanas normales: el **terminal A** ejecuta la simulación y el **terminal B** permite consultar sensores o enviar órdenes. A y B indican su función; no son aplicaciones distintas.

## Dónde abrir las dos ventanas

| Instalación | Terminales A y B |
|---|---|
| [Ubuntu nativo](install.md) | Dos ventanas de Terminal en Ubuntu |
| [Docker en Linux](install-ubuntu-docker.md) | Dos terminales de Linux, ambos dentro del mismo contenedor |
| [WSL en Windows](install-windows.md) | Dos pestañas de Ubuntu en Windows Terminal, en la misma distribución WSL |
| [WSL con Docker](install-windows-docker.md) | Dos pestañas de Ubuntu, ambas dentro del mismo contenedor |
| [Docker y RDP en macOS](install-macos.md) | Dos terminales abiertos en el escritorio Linux al que se accede por RDP |

En Linux y Windows con Docker, entra en el contenedor `posim` desde cada terminal del equipo:

{{CODE:terminals:0}}

El escritorio RDP de macOS ya está dentro del contenedor. Ejecuta allí los comandos de ROS. PowerShell y Terminal de macOS sirven para las operaciones del equipo anfitrión, como iniciar un contenedor.

## Qué es el espacio de trabajo

El **espacio de trabajo de ROS** reúne el código y los paquetes compilados: `src/` contiene el código fuente, `build/` los archivos intermedios y `install/` los paquetes que carga ROS.

| Instalación | Ruta |
|---|---|
| Ubuntu o WSL nativo | `~/posim_ws` |
| Docker AMD64 | `/opt/posim_ws` |
| Docker ARM64 con RDP | `/home/docker/posim_ws` |

`~` representa la carpeta personal del usuario de Linux. La variable `POSIM_WS` guarda la ruta del espacio de trabajo para los ejemplos siguientes.

## Cargar el entorno en A y B

**Ubuntu o WSL nativo:**

{{CODE:terminals:1}}

**Docker, incluido el escritorio RDP:**

{{CODE:terminals:2}}

`source` carga las rutas y la configuración necesarias para encontrar ROS y POSIM. Repite estos pasos en cada terminal nuevo.

## Comprobar la configuración

{{CODE:terminals:3}}

Ambas rutas deben estar dentro de `install/` en el espacio de trabajo. Si falta un paquete, revisa la instalación. Si `ros_gz_sim` aparece bajo `/opt/ros/lyrical`, no se ha cargado la versión del espacio de trabajo.

Continúa con [Tu primera simulación](quickstart.md). Si un comando sigue mostrando mensajes, continúa en ejecución. Para detenerlo, pulsa **Ctrl+C en ese terminal** y espera a que vuelva el indicador de comandos.
