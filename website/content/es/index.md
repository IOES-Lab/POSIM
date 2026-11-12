# Presentación

POSIM es una biblioteca de simulación robótica marina de código abierto para ROS 2 y Gazebo. Permite ejecutar entornos oceánicos, robots de superficie y submarinos, sensores simulados y experimentos de control. Procede de [Project DAVE](https://field-robotics-lab.github.io/dave.doc/), basado en [UUV Simulator](https://uuvsimulator.github.io/).

{{TUTORIALS}}

{{OVERVIEW}}

## Empieza aquí

[Elige un entorno](requirements.md) y sigue la guía de [Ubuntu nativo](install.md), [Linux con Docker](install-ubuntu-docker.md), [Windows con WSL](install-windows.md), [WSL con Docker](install-windows-docker.md) o [macOS con Docker y RDP](install-macos.md). Recomendamos Ubuntu nativo.

Después, [prepara las terminales](terminals.md) y ejecuta tu [primera simulación](quickstart.md).

- ROS: **ROS 2 Lyrical** — LTS; fin de soporte: [mayo de 2031](https://docs.ros.org/en/lyrical/Releases/Release-Lyrical-Luth.html)
- Simulador Gazebo Sim: **Gazebo Jetty** — LTS; fin de soporte: [mayo de 2031](https://gazebosim.org/docs/latest/releases/#release-list)

## Qué puedes simular

Consulta [Funciones y ejemplos](features.md) para encontrar las implementaciones y sus guías.

| Trabajo | Guía |
| --- | --- |
| Preparar el océano y el fondo | [Mundos](worlds.md), [corrientes](currents.md), [efectos submarinos](underwater-rendering.md) |
| Elegir un vehículo | [ROV](rovs.md), [Slocum](gliders.md), [robots de superficie](surface.md) |
| Observar sensores | [Cámara](camera.md), [GNSS](gnss.md), [presión](pressure.md), [DVL](dvl.md), [USBL](usbl.md), [sonar](sonar.md) |
| Controlar y registrar un experimento | [ROS](ros.md), [Python](python-control.md), [propulsores y joystick](robot-control.md), [datos de sensores](visualization.md) |

## Cómo se organiza una simulación

1. Define el entorno y los sistemas del mundo en SDF.
2. Describe los enlaces, colisiones, inercias y sensores del modelo.
3. Un archivo de lanzamiento inicia Gazebo, el modelo y los puentes ROS.
4. Los nodos ROS reciben mediciones y envían comandos de control.

Para ampliar una escena, sigue las guías de [mundos](custom-worlds.md), [robots](custom-robots.md) y [sensores](custom-sensors.md). Las páginas de referencia de cada plugin incluyen un inicio rápido, el fragmento SDF y sus parámetros.

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) combina POSIM con POSIM-Terrain para generar terreno geográfico, POSIM-Routing para planificar rutas y POSIM-Control para controlar vehículos. Añade una interfaz gráfica y aplicaciones web y de escritorio para preparar escenas, manejar robots y consultar los datos de simulación.

El vídeo presenta sus funciones principales. Su instalación y uso se explican en la [documentación de WWW-POSIM](https://www-posim.vercel.app/guide/en.html).

{{INTRO_VIDEO}}

[Ver la presentación en YouTube](https://www.youtube.com/watch?v=_cQtl5JzN2E).
