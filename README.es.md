# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![Compilación y publicación de imágenes Docker de POSIM](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM es una biblioteca de código abierto para la simulación de robótica marina. La mantiene el [IOES Lab. de la Universidad Marítima y Oceánica de Corea (KMOU)](https://lab.wschoi.com).
Reúne entornos oceánicos, robots submarinos y de superficie, sensores simulados e interfaces de control ROS 2 para Gazebo Sim.

![GraphicalAbstract](docs/release/POSIM_Graphical_Abstract_Release1.0.0_FF.png)

## Documentación y primeros pasos

**[Leer la documentación en español](https://ioes-lab.github.io/POSIM/es/)**

[Registro de cambios](CHANGELOG.md)

POSIM utiliza ROS 2 Lyrical y Gazebo Sim Jetty. Elige cómo instalarlo según tu sistema operativo:

- **Ubuntu 26.04:** recomendamos la [instalación nativa](https://ioes-lab.github.io/POSIM/es/install.html). En Linux también puedes optar por la [instalación con Docker](https://ioes-lab.github.io/POSIM/es/install-ubuntu-docker.html).
- **Windows:** [WSL](https://ioes-lab.github.io/POSIM/es/install-windows.html) o [WSL + Docker](https://ioes-lab.github.io/POSIM/es/install-windows-docker.html).
- **macOS:** [Docker + RDP](https://ioes-lab.github.io/POSIM/es/install-macos.html).

Compara las opciones en [Elegir un entorno](https://ioes-lab.github.io/POSIM/es/requirements.html),
[prepara las terminales](https://ioes-lab.github.io/POSIM/es/terminals.html)
y ejecuta tu [primera simulación](https://ioes-lab.github.io/POSIM/es/quickstart.html).

El [catálogo de funciones y ejemplos](https://ioes-lab.github.io/POSIM/es/features.html) enlaza las guías de robots,
mundos, sensores y control. Incluye los [efectos submarinos y la nieve marina](https://ioes-lab.github.io/POSIM/es/underwater-rendering.html)
y el [GNSS marino](https://ioes-lab.github.io/POSIM/es/gnss.html).

## Imágenes Docker

Descarga la imagen actual:

```bash
docker pull ioeslab/posim:latest
```

Docker selecciona la variante AMD64 o ARM64 compatible con tu equipo. Para reproducir un experimento con la misma versión,
elige en [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags) una etiqueta `sha-<12-character-commit>`,
formada con los primeros 12 caracteres del identificador del commit.
Antes de crear el contenedor, consulta [Elegir un entorno](https://ioes-lab.github.io/POSIM/es/requirements.html)
para configurar la visualización en Linux, Windows WSL o macOS.

## Aplicación relacionada

[WWW-POSIM](https://www-posim.vercel.app/) combina POSIM con bibliotecas de terreno geográfico, planificación de rutas
y control de vehículos, y añade una interfaz gráfica y aplicaciones web y de escritorio.
Su documentación explica cómo utilizar esas aplicaciones.

## Contribuir

Puedes comunicar errores o proponer cambios en [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
La [guía de contribución](https://ioes-lab.github.io/POSIM/es/contributing.html) describe las comprobaciones de desarrollo
y el procedimiento para actualizar la documentación.

## Licencia y atribución

POSIM incluye código de [Project DAVE](https://github.com/Field-Robotics-Lab/dave) y conserva sus avisos de autoría y copyright.
El código escrito para POSIM se distribuye bajo la [licencia Apache 2.0](LICENSE).
El código y los recursos de terceros mantienen sus condiciones originales.
La dependencia externa Wave Sim se compila desde su repositorio de origen y está sujeta a la GPL;
consulta los [detalles de esta dependencia y su distribución](extras/surface/README.md#licensing).
