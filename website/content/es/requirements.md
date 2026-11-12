# Elegir un entorno

Elige según el sistema operativo de tu equipo. **Recomendamos Ubuntu 26.04 nativo** para acceder directamente a Gazebo, ROS y los dispositivos NVIDIA/CUDA.

## Opciones de instalación

| Sistema operativo | Método de instalación | Guía |
| --- | --- | --- |
| Ubuntu 26.04 | Compilar y ejecutar POSIM directamente | [Ubuntu nativo](install.md) |
| Ubuntu u otra distribución Linux compatible con Docker, x86-64 | Imagen Ubuntu de POSIM en Docker; ventanas en el escritorio Linux | [Linux con Docker](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | Ubuntu 26.04 en WSL2; compilación nativa y ventanas mediante WSLg | [Windows con WSL](install-windows.md) |
| Windows 10/11, x86-64 | Docker Desktop con WSL2; imagen POSIM y ventanas mediante WSLg | [WSL y Docker](install-windows-docker.md) |
| macOS con Apple Silicon | Imagen ARM64 en Docker; escritorio Linux mediante RDP | [macOS con Docker y RDP](install-macos.md) |

La imagen aporta su propio entorno Ubuntu; el anfitrión Linux puede usar otra versión o distribución compatible con Docker. Esta guía no cubre la emulación ARM64 en Mac Intel.

Para esta versión, usa Docker/RDP en macOS. La instalación nativa en macOS aún no se ha validado por completo. Las referencias a Metal nativo se refieren a componentes individuales y no confirman la compatibilidad de todo el entorno del tutorial.

## Software y gráficos

Todas las opciones usan **ROS 2 Lyrical** y **Gazebo Sim — Gazebo Jetty**. La instalación nativa los prepara en Ubuntu; las imágenes ya los incluyen. Instala el controlador gráfico en el anfitrión siguiendo la guía elegida.

Gazebo y las cámaras de color/profundidad necesitan un renderizador operativo. Empieza con un robot y sus sensores predeterminados. El sonar multihaz ofrece CPU, WGPU y CUDA: [elige y prepara el motor de cálculo](sonar-tuning.md). WGPU necesita Vulkan utilizable en Linux o Metal en macOS nativo. Docker/RDP en macOS ejecuta Linux sin acceso a Metal; empieza con CPU.

## Recursos y descargas

La carga depende del número de vehículos, el detalle del terreno, la resolución y la frecuencia de los sensores. Vigila la memoria, la GPU y el factor de tiempo real antes de aumentarlos. Reserva espacio para código, compilaciones, imágenes Docker, modelos Fuel y grabaciones.

La instalación necesita acceso a los repositorios de ROS/Gazebo, GitHub y Docker Hub. Algunos mundos descargan modelos Fuel al abrirse por primera vez. Cuando funcione el mundo de demostración, [prepara las terminales](terminals.md).
