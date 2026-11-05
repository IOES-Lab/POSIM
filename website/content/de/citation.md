# Zitieren und Lizenzen

IOES-Lab betreut POSIM. Bewahren Sie bei Nutzung und Weitergabe die Urheber- und Komponentenhinweise.

## Die Simulationsgrundlage zitieren

Die DAVE-Veröffentlichung beschreibt die aquatische Simulationsumgebung:

Zhang und Mitautoren, *DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*, IEEE/OES Autonomous Underwater Vehicles Symposium, 2022. [DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808).

Für das Multibeam-Sonarmodell:

Choi und Mitautoren, *Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*, Frontiers in Robotics and AI, 2021. [DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646).

Halten Sie für reproduzierbare Versuche auch die POSIM-Revision, die Roboter- und Weltkonfiguration sowie relevante Abhängigkeiten fest. Übernehmen Sie vollständige Autorenlisten und bibliografische Angaben aus dem Originaleintrag der jeweiligen Veröffentlichung.

## Quellen der Komponenten

| Komponente | Quelle und Hinweise |
| --- | --- |
| Eigene und übernommene Quellen unter Apache-Lizenz | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE) und Hinweise in den einzelnen Dateien |
| DAVE-Grundlage | [Project DAVE](https://github.com/Field-Robotics-Lab/dave), Urheber- und Quellenangaben |
| Externes Wave Sim | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim), GPLv3-Lizenz des Ursprungsprojekts |
| Externe WAM-V/VRX-Ressourcen | Mit der heruntergeladenen Abhängigkeit erhaltene Hinweise |
| Gazebo-Fuel-Modelle | Quelle und Lizenz der jeweiligen Ressource |

Der Wellen-Build bewahrt die ursprünglichen Quellen und Änderungen getrennt auf. Hinweise zu Abhängigkeiten und Weitergabe stehen in [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md). Getrennte Repositories ersetzen nicht die Lizenzbedingungen der Komponenten.

## Dokumentationsquellen

Die [ursprüngliche ROS-2-Projektdokumentation](https://dave-ros2.notion.site/) dient als Quelle für Modelle, Umgebungen und Sensorszenarien. Die Befehle auf diesen Seiten verwenden POSIM-Paketnamen und die in den Installationsanleitungen genannten ROS/Gazebo-Versionen.

Weiteres Material finden Sie im [POSIM Notion Wiki von IOES-Lab](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74) und über die Quellenlinks der jeweiligen Seite.

Labor und Betreuung: [IOES-Lab · KMOU](https://lab.wschoi.com). Quellcode und Fehlerberichte: [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
