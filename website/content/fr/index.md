# Présentation

POSIM est une bibliothèque libre de simulation robotique marine pour ROS 2 et Gazebo. Elle permet d’exécuter des environnements océaniques, des robots de surface ou sous-marins, des capteurs simulés et des expériences de commande. Elle est issue de [Project DAVE](https://field-robotics-lab.github.io/dave.doc/), lui-même fondé sur [UUV Simulator](https://uuvsimulator.github.io/).

{{OVERVIEW}}

## Par où commencer ?

[Choisissez votre environnement](requirements.md), puis suivez l’installation pour [Ubuntu natif](install.md), [Linux avec Docker](install-ubuntu-docker.md), [Windows avec WSL](install-windows.md), [WSL avec Docker](install-windows-docker.md) ou [macOS avec Docker et RDP](install-macos.md). L’installation native sur Ubuntu est recommandée.

[Préparez ensuite vos terminaux](terminals.md) avant de lancer votre [première simulation](quickstart.md).

- ROS : **ROS 2 Lyrical**
- Simulateur Gazebo Sim : **Gazebo Jetty**

## Que peut-on simuler ?

La page [Fonctionnalités et exemples](features.md) relie les fonctions disponibles à leur code et à leurs guides.

| Objectif | Guides |
| --- | --- |
| Préparer l’océan et le fond marin | [Mondes](worlds.md), [courants](currents.md), [effets sous-marins](underwater-rendering.md) |
| Choisir un véhicule | [ROV](rovs.md), [Slocum](gliders.md), [robots de surface](surface.md) |
| Observer les capteurs | [Caméra](camera.md), [GNSS](gnss.md), [pression](pressure.md), [DVL](dvl.md), [USBL](usbl.md), [sonar](sonar.md) |
| Commander et enregistrer une expérience | [ROS](ros.md), [Python](python-control.md), [propulseurs et joystick](robot-control.md), [données des capteurs](visualization.md) |

## Organisation d’une simulation

1. Définissez l’environnement et les systèmes du monde en SDF.
2. Décrivez les liens, collisions, inerties et capteurs du modèle.
3. Un fichier de lancement démarre Gazebo, le modèle et les ponts ROS.
4. Les nœuds ROS reçoivent les mesures et envoient les commandes.

Pour aller plus loin, créez un [monde](custom-worlds.md), ajoutez un [robot](custom-robots.md) ou configurez des [capteurs](custom-sensors.md). Chaque référence de plugin commence par une prise en main, un exemple SDF et ses paramètres.

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) associe POSIM à POSIM-Terrain pour le terrain géographique, POSIM-Routing pour la planification de trajets et POSIM-Control pour la commande des véhicules. Il y ajoute une interface graphique et des applications web et de bureau pour préparer les scènes, piloter les robots et consulter les données de simulation.

La vidéo présente ses principales fonctionnalités. Consultez la [documentation de WWW-POSIM](https://www-posim.vercel.app/guide/en.html) pour l’installation et l’utilisation.

{{INTRO_VIDEO}}

[Voir la présentation sur YouTube](https://www.youtube.com/watch?v=_cQtl5JzN2E).
