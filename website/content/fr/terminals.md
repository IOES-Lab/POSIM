# Préparer les terminaux

Un **terminal** est une fenêtre où l’on saisit des commandes et lit leurs résultats. Les guides utilisent deux fenêtres ordinaires : le **terminal A** fait tourner la simulation, tandis que le **terminal B** sert à consulter les capteurs ou à envoyer des commandes. Ces lettres désignent des rôles, pas des logiciels différents.

## Où ouvrir les deux fenêtres

| Installation | Terminaux A et B |
|---|---|
| [Ubuntu natif](install.md) | Deux fenêtres de l’application Terminal d’Ubuntu |
| [Docker sous Linux](install-ubuntu-docker.md) | Deux terminaux Linux, tous deux dans le même conteneur |
| [WSL sous Windows](install-windows.md) | Deux onglets Ubuntu de Windows Terminal, dans la même distribution WSL |
| [WSL avec Docker](install-windows-docker.md) | Deux onglets Ubuntu, tous deux dans le même conteneur |
| [Docker et RDP sous macOS](install-macos.md) | Deux terminaux ouverts sur le bureau Linux accessible par RDP |

Avec Docker sous Linux ou Windows, entrez dans le conteneur `posim` depuis chacun des terminaux de l’hôte :

{{CODE:terminals:0}}

Le bureau RDP utilisé sous macOS se trouve déjà dans le conteneur : exécutez-y les commandes ROS. PowerShell et Terminal de macOS servent aux commandes de l’hôte, notamment au démarrage des conteneurs.

## Comprendre l’espace de travail

L’**espace de travail ROS** regroupe les sources et les paquets compilés. `src/` contient les sources, `build/` les fichiers de compilation intermédiaires et `install/` les paquets chargés par ROS.

| Installation | Chemin |
|---|---|
| Ubuntu ou WSL natif | `~/posim_ws` |
| Docker AMD64 | `/opt/posim_ws` |
| Docker ARM64 avec RDP | `/home/docker/posim_ws` |

`~` désigne le dossier personnel de l’utilisateur Linux. La variable `POSIM_WS` contient le chemin de l’espace de travail utilisé dans la suite des guides.

## Charger l’environnement dans A et B

**Ubuntu ou WSL natif :**

{{CODE:terminals:1}}

**Docker, y compris sur le bureau RDP :**

{{CODE:terminals:2}}

`source` charge les chemins et les réglages nécessaires pour trouver ROS et POSIM. Recommencez dans chaque nouveau terminal.

## Vérifier les chemins

{{CODE:terminals:3}}

Les deux chemins doivent appartenir au dossier `install/` de votre espace de travail. Si un paquet manque, reprenez le guide d’installation. Si `ros_gz_sim` se trouve sous `/opt/ros/lyrical`, la version de l’espace de travail n’a pas été chargée.

Passez à [Votre première simulation](quickstart.md). Une commande qui affiche encore des messages est toujours active. Arrêtez-la avec **Ctrl+C dans son terminal**, puis attendez le retour de l’invite avant de saisir la suivante.
