# Configurar y ajustar el sonar

Elige cobertura, detalle y frecuencia para tu experimento. La [referencia del sonar multihaz](sonar.md) contiene el inicio rápido, SDF y parámetros; esta guía explica cómo comparar configuraciones y preparar CUDA.

## Entorno necesario

Usa las [terminales Linux preparadas](terminals.md) en Ubuntu o WSL 2 compatible con GPU NVIDIA, o en un contenedor con acceso a ella. Edita y compila allí; `POSIM_WS` es su espacio de trabajo. Docker/RDP en macOS no expone una GPU CUDA: ejecuta este sonar en un equipo Linux o WSL 2 con NVIDIA.

## Medir una configuración de referencia

Inicia el [ejemplo BlueView P900](sonar.md#run-a-sonar-example). Mantén fijos escena, distancia al blanco y pose del sensor. Confirma imagen y datos sin procesar antes de medir. Si falta CUDA, completa primero su instalación.

En otra terminal preparada, mide la frecuencia recibida:

{{CODE:sonar-tuning:0}}

Tras una muestra estable, detén con Ctrl+C. Consulta por separado la velocidad de simulación:

{{CODE:sonar-tuning:1}}

El nombre interno de este mundo es `default`. Registra `real_time_factor`: solicitar 30 Hz simulados no garantiza 30 imágenes por segundo real. Ctrl+C detiene la consulta.

## Comparar una variable

Cambia un ajuste en una copia del [descriptor](sonar.md#sensor-configuration), recompila, carga el entorno y reinicia con la misma pose.

| Experimento | Comparación | Qué observar |
|---|---|---|
| Detalle angular | 512 frente a 256 haces horizontales, mismo campo de visión | Separación de blancos y frecuencia |
| Muestreo vertical | 300 frente a 150 rayos | Continuidad de blancos y tiempo de cálculo |
| Cobertura | Cambiar alcance de rayos y `maxDistance` juntos | Blanco más lejano y escala de distancia |
| Actualización | 30 frente a 10 Hz | Frecuencia recibida y factor de tiempo real |
| Salto de rayos | `raySkips` de 10 frente a 5 | Detalle vertical y coste de cálculo |

Mide el resultado de cada comparación. Desactiva `writeLog` y `debugFlag` al medir rendimiento; actívalos por separado para estudiar retornos o tiempos. Guarda SDF, revisión POSIM, versiones GPU/CUDA y observaciones. La [guía de datos de sensores](visualization.md) explica la grabación de tópicos.

## Preparar las herramientas

El [plugin](sonar.md) necesita controlador NVIDIA, compilador y runtime CUDA, cuFFT, cuBLAS y renderizado Gazebo. En el entorno que ejecutará el sonar:

{{CODE:sonar-tuning:2}}

Comprueba GPU y compilador por separado. Para Docker Engine en Linux, configura [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) en el host. En Windows, sigue la [configuración GPU de Docker Desktop/WSL 2](https://docs.docker.com/desktop/features/gpu/). La imagen también necesita las bibliotecas CUDA.

## Elegir la arquitectura CUDA

Usa un valor de `CMAKE_CUDA_ARCHITECTURES` compatible con GPU y toolkit. Este ejemplo usa SM 75: sustituye `75` por tu arquitectura y ejecuta desde el espacio de trabajo preparado.

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` enumera los destinos admitidos. Sin una arquitectura explícita, CMake toma la predeterminada del compilador. Si hay varias instalaciones CUDA, usa `-DCUDAToolkit_ROOT=/path/to/cuda` y añade su directorio `bin` a `PATH`.

## Comprobar los componentes instalados

{{CODE:sonar-tuning:4}}

La presencia de un paquete no basta si se omitieron sus componentes CUDA. Comprueba las bibliotecas:

{{CODE:sonar-tuning:5}}

Cada comando debe terminar con código 0. Revisa componentes omitidos o errores de carga y recibe los datos indicados en [Sonar multihaz](sonar.md).

## Interpretar las medidas

Calienta el renderizador antes de medir. Con la misma escena y pose, registra tiempo por fotograma, frecuencia recibida, factor de tiempo real, uso de CPU/GPU y memoria. Distingue física, renderizado, cálculo CUDA, publicación y registro.

`blazingSonarImage` selecciona semillas aleatorias variables o la semilla fija `1234`; compara la repetibilidad. Conserva datos de referencia al cambiar los núcleos numéricos y compara intensidad y distancia, además de las imágenes. La [página de citas](citation.md) recoge la investigación original.

## Procesamiento en GPU

{{MEDIA:sonar-tuning}}

El proceso incluye suma de rayos, ventana/corrección y FFT. Al perfilar, separa esos tiempos de las transferencias de memoria GPU. Imagen: wiki Notion de POSIM.
