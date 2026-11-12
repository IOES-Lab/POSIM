# Windows: instalación en WSL

Instala POSIM en Ubuntu 26.04 dentro de Windows Subsystem for Linux 2 (WSL2). WSLg muestra Gazebo en el escritorio de Windows. Para usar una imagen preparada, elige [WSL con Docker](install-windows-docker.md).

## 1. Instalar WSL y Ubuntu

Abre PowerShell como administrador:

{{CODE:install-windows:0}}

Reinicia si Windows lo solicita. Abre Ubuntu y crea tu usuario y contraseña de Linux. Comprueba en PowerShell que la distribución usa WSL 2:

{{CODE:install-windows:1}}

Si ya tienes WSL o falla la instalación, consulta la [guía de Microsoft](https://learn.microsoft.com/es-es/windows/wsl/install).

## 2. Preparar el controlador gráfico y WSLg

Instala o actualiza el **controlador de Windows** de tu GPU Intel, AMD o NVIDIA. También es necesario con NVIDIA. WSLg utiliza ese controlador para la aceleración gráfica; consulta la [guía de aplicaciones gráficas](https://learn.microsoft.com/es-es/windows/wsl/tutorials/gui-apps).

Con NVIDIA, sigue [CUDA en WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html): instala el controlador de Windows, no un controlador de pantalla Linux dentro de WSL. El toolkit CUDA solo es necesario para compilar el backend CUDA del sonar.

Actualiza WSL desde PowerShell:

{{CODE:install-windows:2}}

Guarda el trabajo de los terminales WSL antes de cualquier reinicio solicitado y vuelve a abrir Ubuntu.

## 3. Instalar POSIM en Ubuntu

Usa el perfil **Ubuntu** de Windows Terminal para los comandos Linux. Comprueba la distribución:

{{CODE:install-windows:3}}

Sigue [Ubuntu: instalación nativa](install.md), desde la descarga del código hasta la compilación. Mantén el espacio de trabajo en `~/posim_ws`, dentro de Linux. El instalador usa `sudo` y actualiza los paquetes de Ubuntu.

Si aparecen tópicos de imagen pero no llegan fotogramas, consulta [Resolución de problemas](troubleshooting.md). [Preparar los terminales](terminals.md) explica las dos ventanas utilizadas en los siguientes ejemplos.

## 4. Ejecutar el mundo de demostración

Ejecuta lo siguiente en Ubuntu. Gazebo debe abrirse en el escritorio de Windows y mostrar el océano. El primer inicio puede descargar modelos de Fuel. Pulsa Ctrl+C antes de iniciar otro ejemplo.

{{CODE:install-windows:4}}

Ejecuta la versión de Linux de QGC en la misma distribución Ubuntu que POSIM. Para esta configuración, no uses la aplicación instalada en Windows. Sigue la [configuración de QGC](qgroundcontrol.md).
