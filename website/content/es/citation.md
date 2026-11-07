# Referencias y licencias

IOES-Lab mantiene POSIM. Conserva la autoría y los avisos de cada componente al utilizarlo o distribuirlo.

## Citar la base de la simulación

El artículo de DAVE describe el entorno de simulación acuática:

Zhang y colaboradores, *DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*, IEEE/OES Autonomous Underwater Vehicles Symposium, 2022. [DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808).

Para el modelo de sonar multihaz:

Choi y colaboradores, *Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*, Frontiers in Robotics and AI, 2021. [DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646).

Registra también la revisión de POSIM, la configuración del robot y del mundo, y las dependencias relevantes para reproducir el experimento. Obtén la lista completa de autores y los datos bibliográficos del registro de la publicación original.

## Atribución de componentes

| Componente | Fuente y avisos |
| --- | --- |
| Código propio y heredado bajo licencia Apache | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE) y avisos de cada archivo |
| Base DAVE | [Project DAVE](https://github.com/Field-Robotics-Lab/dave), autoría y avisos de origen |
| Wave Sim externo | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim), licencia GPLv3 del proyecto original |
| Recursos externos WAM-V/VRX | Avisos conservados con la dependencia descargada |
| Modelos Gazebo Fuel | Origen y licencia de cada recurso |

La compilación de olas conserva por separado el código original y sus modificaciones. Consulta las notas de dependencias y distribución en [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md). La separación de repositorios no sustituye las condiciones de licencia de cada componente.

## Fuentes de la documentación

La [documentación original del proyecto ROS 2](https://dave-ros2.notion.site/) sirve de referencia para modelos, entornos y sensores. Los comandos de estas páginas utilizan los nombres de paquetes POSIM y las versiones de ROS/Gazebo indicadas en la instalación.

Puedes consultar también la [POSIM Notion Wiki de IOES-Lab](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74) y los enlaces de origen de cada página.

Laboratorio y mantenimiento: [IOES-Lab · KMOU](https://lab.wschoi.com). Código e incidencias: [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
