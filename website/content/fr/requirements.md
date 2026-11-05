# Choisir un environnement

Choisissez une méthode adaptée à votre ordinateur. **Ubuntu 26.04 en installation native est recommandé** pour accéder directement à Gazebo, aux outils ROS et aux périphériques NVIDIA/CUDA.

## Méthodes d’installation

| Système d’exploitation | Méthode | Guide |
| --- | --- | --- |
| Ubuntu 26.04 | Compiler et exécuter POSIM directement | [Ubuntu natif](install.md) |
| Ubuntu ou autre distribution Linux prise en charge par Docker, x86-64 | Image Ubuntu de POSIM dans Docker ; fenêtres sur le bureau Linux | [Linux avec Docker](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | Ubuntu 26.04 dans WSL2 ; compilation native et affichage avec WSLg | [Windows avec WSL](install-windows.md) |
| Windows 10/11, x86-64 | Docker Desktop avec WSL2 ; image POSIM et affichage avec WSLg | [WSL et Docker](install-windows-docker.md) |
| macOS sur Apple Silicon | Image ARM64 dans Docker ; bureau Linux accessible par RDP | [macOS avec Docker et RDP](install-macos.md) |

L’image fournit son propre environnement Ubuntu. Le système Linux hôte peut donc utiliser une autre version ou distribution compatible avec Docker. L’émulation ARM64 sur Mac Intel n’est pas couverte ici.

## Logiciels et affichage

Toutes les méthodes utilisent **ROS 2 Lyrical** et **Gazebo Sim — Gazebo Jetty**. L’installation native les prépare dans Ubuntu ; les images les contiennent déjà. Installez le pilote graphique sur l’hôte selon le guide choisi.

Gazebo et les caméras couleur ou de profondeur nécessitent un moteur de rendu fonctionnel. Commencez avec un robot et ses capteurs par défaut. Le [sonar multifaisceaux CUDA](sonar-tuning.md) exige aussi un GPU NVIDIA compatible, les outils CUDA et ses bibliothèques. Docker sur macOS ne rend pas Apple Metal accessible comme périphérique NVIDIA/CUDA Linux.

## Ressources et téléchargements

La charge varie selon le nombre de véhicules, le détail du terrain, la résolution et la fréquence des capteurs. Surveillez la mémoire, le GPU et le facteur de temps réel avant d’augmenter ces réglages. Prévoyez de l’espace pour les sources, compilations, images Docker, modèles Fuel et enregistrements.

L’installation accède aux dépôts ROS/Gazebo, à GitHub et à Docker Hub. Certains mondes téléchargent des modèles Fuel au premier lancement. Une fois le monde de démonstration fonctionnel, [préparez les terminaux](terminals.md).
