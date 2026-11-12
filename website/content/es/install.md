# Ubuntu: instalación nativa

Compila POSIM en Ubuntu 26.04. Abre la aplicación Terminal y ejecuta los bloques de uno en uno, esperando a que termine cada paso. Para usar una imagen preparada, sigue [Linux con Docker](install-ubuntu-docker.md).

## 1. Descargar el código

{{CODE:install:0}}

Mantén el nombre `posim` para que coincida con el manifiesto de repositorios.

## 2. Instalar ROS, Gazebo y las dependencias

Lee primero `src/posim/extras/ros-lyrical-gz-jetty-install.sh`. El script configura repositorios, usa `sudo` y ejecuta `apt-get full-upgrade -y`. Se recomienda un entorno Ubuntu dedicado.

{{CODE:install:1}}

Instala ROS 2 Lyrical, Gazebo Jetty, ArduSub, ArduRover, MAVROS y las bibliotecas de olas. Wave Sim se descarga en una revisión fijada y se compila como dependencia externa; no hace falta clonar POSIM de forma recursiva.

## 3. Preparar los paquetes del workspace

{{CODE:install:2}}

`--skip-existing` conserva el código POSIM que acabas de clonar. Revisa cualquier error de dependencias antes de compilar.

El manifiesto también importa una revisión fijada de `ros_gz` con correcciones del ciclo de vida del puente y del lanzamiento POSIX. Si ya existe `src/ros_gz`, no se actualiza automáticamente: compara su origen y revisión con el manifiesto o utiliza un espacio de trabajo nuevo.

## 4. Compilar y cargar el entorno

{{CODE:install:3}}

Ambas rutas de `ros_gz` deben apuntar a la carpeta `install/` de este espacio de trabajo. Si aparece `/opt/ros/lyrical`, todavía se está usando el paquete del sistema. En cada terminal nueva, carga ROS y el workspace como explica [Preparar las terminales](terminals.md).

### Si falta memoria durante la compilación

`--executor sequential` limita los paquetes simultáneos, pero no los procesos del compilador. Para limitar ambos:

{{CODE:install:5}}

### Actualizar el espacio de trabajo

Guarda tus cambios en Git antes de actualizar. Anota la revisión, descarga los cambios y repite la resolución de dependencias y la compilación:

{{CODE:install:6}}

Continúa con la [primera simulación](quickstart.md). Para el sonar, consulta la [compilación y configuración de CPU/WGPU/CUDA](sonar-tuning.md).

## 5. Ejecutar el mundo de demostración

La última comprobación abre Gazebo con la escena oceánica. La primera ejecución puede descargar modelos Fuel. Detén la simulación con **Ctrl+C** antes de continuar con [las terminales](terminals.md).

{{CODE:install:7}}
