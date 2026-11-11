# Configurar y ajustar el sonar

Elige el motor de cálculo y compara cobertura, detalle y frecuencia de actualización. La [referencia del sonar multihaz](sonar.md) reúne el inicio rápido, el SDF y los parámetros.

## Elegir el entorno

Usa las [terminales POSIM preparadas](terminals.md). Edita, compila y ejecuta dentro de la misma instalación de Ubuntu, WSL o Docker; `POSIM_WS` es su espacio de trabajo. Todos los motores necesitan los sensores de renderizado de Gazebo.

| Entorno | Opciones |
| --- | --- |
| Linux nativo | CPU; WGPU con adaptador Vulkan utilizable; CUDA con GPU NVIDIA y toolkit compatibles |
| WSL 2 o contenedor Linux | CPU; para calcular en GPU, el adaptador y el controlador deben ser accesibles **dentro** del entorno |
| Docker/RDP en macOS | Empieza con CPU. El contenedor Linux no expone el dispositivo Metal del Mac. |
| macOS nativo | WGPU puede usar Metal con un entorno ROS/Gazebo nativo preparado por separado; la [guía de macOS](install-macos.md) describe Docker/RDP. |

`auto` prueba CUDA → WGPU → CPU. Para comparar resultados, elige `cpu`, `wgpu` o `cuda` explícitamente. Si el elegido no está disponible, el sensor no se inicializa. Consulta las [opciones](sonar.md#compute-backend).

## Compilar los paquetes del sonar {#build-the-sonar-packages}

Parte de una [instalación desde el código fuente](install.md) con sus dependencias. `wgpu_vendor` compila la biblioteca Rust incluso para usar CPU. Comprueba Rust y Cargo; si faltan, sigue la [instalación oficial](https://www.rust-lang.org/tools/install).

{{CODE:sonar-tuning:0}}

Desde el espacio de trabajo con el entorno cargado, compila la biblioteca, el sensor, el sistema del mundo y el ejemplo:

{{CODE:sonar-tuning:1}}

CUDA es opcional. Para omitir un toolkit instalado y compilar CPU/WGPU, añade `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`. `Cargo.lock` fija las dependencias Rust; su primera descarga necesita Internet.

{{CODE:sonar-tuning:2}}

Comprueba los cuatro paquetes y ejecuta el [ejemplo](sonar.md#run-a-sonar-example) hasta recibir imágenes y datos brutos. Que un paquete aparezca en la lista no confirma la carga del sensor. Comprueba los paquetes dentro del contenedor; si faltan, compílalos desde el código fuente, sin dar por hecho que una etiqueta de imagen los incluye.

## Seleccionar el adaptador WGPU

En Linux, ejecuta `vulkaninfo --summary` donde se ejecutará Gazebo, con el controlador y las herramientas Vulkan del sistema. Un adaptador por software como llvmpipe puede ejecutar Vulkan sin aceleración de hardware. La salida de inicio de WGPU muestra el adaptador elegido.

{{CODE:sonar-tuning:3}}

| Variable | Uso |
| --- | --- |
| `WGPU_BACKEND` | `vulkan` en Linux, `metal` en un proceso macOS nativo; omítela para usar los valores de la plataforma |
| `WGPU_ADAPTER_NAME` | Primer adaptador cuyo nombre contiene el texto, sin distinguir mayúsculas; usa un nombre mostrado por el controlador o al arrancar |
| `WGPU_POWER_PREF` | `high`, `low` o `none`; por defecto `high` si no se indica nombre |

Define las variables antes del lanzamiento. Detén y vuelve a iniciar la simulación después de cambiarlas: la inicialización se comparte dentro del proceso. Consulta las API en [wgpu](https://github.com/gfx-rs/wgpu). Si falla la inicialización o la comprobación de recursos, revisa el adaptador y sus límites; selecciona `cpu` o reduce las dimensiones del sensor.

## CUDA opcional

Necesitas controlador NVIDIA, compilador/runtime CUDA, cuFFT y cuBLAS. Comprueba el entorno de ejecución:

{{CODE:sonar-tuning:4}}

Para contenedores Linux, configura [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) en el host. En Windows, sigue [CUDA en WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) y, si corresponde, la [configuración GPU de Docker Desktop](https://docs.docker.com/desktop/features/gpu/). Instala el controlador de pantalla en Windows, no uno de Linux dentro de WSL. El contenedor también necesita las bibliotecas CUDA.

Elige una arquitectura compatible. El ejemplo usa SM 75: cambia `75` por el valor de tu GPU. `OFF` reactiva la detección del toolkit tras una compilación solo CPU/WGPU.

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` enumera los destinos admitidos. Sin un valor explícito se usa el predeterminado del compilador. Si hay varios toolkits, elige `-DCUDAToolkit_ROOT=/path/to/cuda` y añade su `bin` a `PATH`. Ejecuta el ejemplo con `compute_backend:=cuda`.

## Medir una referencia

Mantén fijos escena, distancia al blanco y pose del sensor. Mide en la terminal B:

{{CODE:sonar-tuning:6}}

Detén con Ctrl+C y consulta la velocidad de simulación:

{{CODE:sonar-tuning:7}}

La entidad del mundo se llama `default`. Registra `real_time_factor` junto con la frecuencia recibida: pedir 30 Hz de tiempo simulado no garantiza 30 imágenes por segundo real. Cierra con Ctrl+C.

## Ajustar muestreo y alcance

Cambia una variable en una copia del [SDF](sonar.md#sensor-configuration), recompila, carga el entorno y arranca con la misma pose.

| Experimento | Comparación | Qué observar |
|---|---|---|
| Detalle angular | 512 frente a 256 haces horizontales, mismo campo de visión | Separación de blancos y frecuencia |
| Muestreo vertical | 300 frente a 150 rayos | Continuidad de blancos y tiempo de cálculo |
| Cobertura | Cambiar alcance de rayos y `maxDistance` juntos | Blanco más lejano y escala de distancia |
| Actualización | 30 frente a 10 Hz | Frecuencia recibida y factor de tiempo real |
| Salto de rayos | `raySkips` de 10 frente a 5 | Detalle vertical y coste de cálculo |

El paso de distancia es `soundSpeed / (2 × bandwidth)`; hay `ceil(maxDistance / paso) + 1` celdas. Con 1500 m/s, 29.9 kHz y 15 m son 599. La última coordenada puede superar ligeramente `maxDistance`, pero se excluyen los retornos fuera del alcance. Lee distancias y número de haces en el mensaje bruto o CSV.

WGPU conserva la longitud solicitada, aunque no sea potencia de dos, y elige las etapas FFT automáticamente. Más alcance o ancho de banda implica más memoria y cálculo. En el modo de cuadrícula predeterminado, CPU asigna cada retorno a la celda de distancia más próxima; GPU procesa en frecuencia. Compara también amplitud y respuesta en distancia al cambiar de motor.

## Guardar una comparación reproducible

Espera a que el renderizado se estabilice. Desactiva `writeLog` y `debugFlag` para medir rendimiento; registra por separado con [CSV](sonar.md) o [ROS](visualization.md). `blazingSonarImage=false` fija la semilla en 1234.

Conserva SDF, revisión POSIM, motor/adaptador, versiones de controlador/toolkit, tiempo por imagen, frecuencia recibida, factor de tiempo real y memoria. Compara datos brutos de intensidad/distancia además de capturas. El modelo se cita en [Referencias](citation.md).

## Procesamiento en GPU

{{MEDIA:sonar-tuning}}

Incluye suma de rayos, ventana/corrección y FFT. Mide por separado cálculo y transferencias de memoria. Figura: POSIM Notion Wiki.

Con el SDK de Gazebo Sim 10.5 probado en macOS, los mundos con sonar como único sensor pueden fallar al inicializar Ogre2. Si usas ese SDK, añade al mundo una cámara integrada pequeña.

## Transporte ROS en macOS nativo {#native-macos-ros-transport}

Con Fast DDS en macOS nativo, usa el perfil local instalado si no llegan los mensajes grandes de sonar con QoS best-effort. Configura un segmento de memoria compartida de 16 MiB y búferes de mensajes de transporte de 1 MiB. UDP en localhost usa datagramas de 8192 bytes y búferes de socket de envío y recepción de 2 MiB cada uno. Define la variable en cada terminal del simulador, del puente y del suscriptor antes de iniciar esos procesos:

{{CODE:sonar-tuning:8}}

El perfil es opcional y está limitado a procesos del mismo Mac. Para comunicarte con otros equipos, usa un perfil de red adecuado. Si `FASTDDS_DEFAULT_PROFILES_FILE` ya está definida, incorpora los ajustes de memoria compartida a ese perfil. Estos ajustes de transporte ROS se aplican al sonar CPU y WGPU.
