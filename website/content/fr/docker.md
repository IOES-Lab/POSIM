# Environnement Docker

Les images POSIM contiennent Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, MAVROS et QGroundControl. Commencez par [choisir votre environnement](requirements.md) pour suivre les réglages d’affichage et de pilotes adaptés à votre système.

## Télécharger une image publiée

Les images sont disponibles sur [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags). Docker choisit automatiquement la variante AMD64 ou ARM64.

{{CODE:docker:0}}

| Étiquette | Utilisation |
|---|---|
| `latest` | Version publiée de la branche principale, pour les deux architectures |
| `sha-<12-character-commit>` | Révision précise du code ; recopiez une étiquette existante sur Docker Hub |
| Suffixe `-amd64` ou `-arm64` de l’étiquette SHA | Choix explicite de l’architecture |
| Numéro de version, lorsqu’il est publié | Version identifiée par une étiquette de publication |

L’étiquette de métadonnées donne la révision complète. Conservez aussi le digest pour réutiliser exactement la même image lors d’une expérience.

{{CODE:docker:1}}

Consultez la documentation de la révision choisie. Le téléchargement d’une nouvelle image ne met pas à jour un conteneur existant.

## Construire l’image à partir des sources

Sur la machine hôte :

{{CODE:docker:2}}

Pour AMD64 :

{{CODE:docker:3}}

Pour ARM64 :

{{CODE:docker:4}}

Dans votre guide d’installation, remplacez `ioeslab/posim:latest` par l’étiquette locale. Les deux images compilent les sources présentes et la révision fixée de la bibliothèque de vagues. L’image ARM64 fournit aussi un bureau XFCE et un serveur RDP.

## Ouvrir l’environnement

- [Linux avec Docker](install-ubuntu-docker.md) : affichage sur le bureau Linux.
- [Windows avec WSL et Docker](install-windows-docker.md) : fenêtres Linux affichées par WSLg.
- [macOS avec Docker et RDP](install-macos.md) : connexion au bureau Linux distant.

Chaque guide crée un conteneur nommé `posim`. La page [Préparer les terminaux](terminals.md) indique où saisir les commandes et charger l’espace de travail.

## Conserver vos fichiers

L’espace de travail se trouve dans `/opt/posim_ws` sur AMD64 et `/home/docker/posim_ws` sur ARM64. Son emplacement est exposé par `POSIM_WS` ou `POSIM_UNDERLAY`.

Enregistrez vos fichiers SDF et vos acquisitions dans `/work`, associé au volume `posim-data`. Les autres fichiers restent présents après un arrêt, mais disparaissent si le conteneur est supprimé. Enregistrez vos modifications de code dans Git ou copiez-les ailleurs avant de le remplacer. Le volume reste disponible tant que vous ne le supprimez pas explicitement.

## Arrêter, reprendre et mettre à jour {#update-an-existing-container}

Arrêtez d’abord la simulation avec **Ctrl+C**, puis exécutez sur l’hôte :

{{CODE:docker:5}}

Pour mettre à jour, sauvegardez votre travail, arrêtez le conteneur, téléchargez l’image choisie et supprimez le conteneur arrêté. Reprenez la commande de création de votre guide en conservant le volume `posim-data`.

## Affichage et GPU {#graphics-and-gpu-access}

Sous Linux, configurez le pilote de l’hôte et [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). Sous Windows, suivez la [configuration GPU de Docker Desktop avec WSL2](https://docs.docker.com/desktop/features/gpu/). Le sonar CUDA exige aussi les bibliothèques nécessaires dans l’image : voir [Configurer le sonar](sonar-tuning.md). Les premières commandes utilisent le rendu logiciel, parfois plus lent.
