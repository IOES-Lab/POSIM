# Références et licences

IOES-Lab assure la maintenance de POSIM. Conservez les mentions d'auteur et les notices de chaque composant lors de son utilisation ou de sa distribution.

## Citer les fondements de la simulation

L'article du projet DAVE décrit l'environnement de simulation aquatique :

Zhang et ses coauteurs, *DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*, IEEE/OES Autonomous Underwater Vehicles Symposium, 2022. [DOI : 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808).

Pour le modèle de sonar multifaisceaux :

Choi et ses coauteurs, *Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*, Frontiers in Robotics and AI, 2021. [DOI : 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646).

Consignez aussi la révision de POSIM, la configuration du robot et du monde, ainsi que les dépendances utiles à la reproduction de l'expérience. Reprenez la liste complète des auteurs et les métadonnées bibliographiques depuis la publication originale.

## Attribution des composants

| Composant | Source et notices |
| --- | --- |
| Sources propres à POSIM ou héritées sous licence Apache | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE) et notices de chaque fichier |
| Base DAVE | [Project DAVE](https://github.com/Field-Robotics-Lab/dave), mentions d'auteur et d'origine |
| Wave Sim externe | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim), licence GPLv3 du projet amont |
| Ressources externes WAM-V/VRX | Notices conservées avec la dépendance téléchargée |
| Modèles Gazebo Fuel | Source et licence propres à chaque ressource |

La compilation des vagues conserve séparément les sources amont et leurs modifications. Consultez les notes de dépendances et de distribution dans [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md). La séparation des dépôts ne remplace pas les conditions de licence des composants.

## Sources de la documentation

La [documentation originale du projet ROS 2](https://dave-ros2.notion.site/) sert de référence aux explications sur les modèles, les environnements et les capteurs. Les commandes de ces pages utilisent les noms de paquets POSIM et les versions ROS/Gazebo indiquées dans les guides d'installation.

La [POSIM Notion Wiki d'IOES-Lab](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74) et les liens de chaque page fournissent d'autres documents de référence.

Laboratoire et maintenance : [IOES-Lab · KMOU](https://lab.wschoi.com). Code et signalements : [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
