# Linux : installation avec Docker

Utilisez l’image publiée de POSIM si votre bureau fonctionne avec une autre version d’Ubuntu ou une autre distribution. Ce guide concerne **AMD64 sur un ordinateur x86-64**, avec X11 ou XWayland. Pour Ubuntu 26.04, nous recommandons l’[installation native](install.md).

## 1. Préparer Docker et l’affichage

Installez [Docker Engine pour votre distribution](https://docs.docker.com/engine/install/), ou suivez les [instructions Ubuntu](https://docs.docker.com/engine/install/ubuntu/). Si votre compte ne peut pas exécuter `docker`, terminez les [étapes après installation](https://docs.docker.com/engine/install/linux-postinstall/).

Ouvrez un terminal dans votre session graphique et installez `xauth`. Sous Ubuntu :

{{CODE:install-ubuntu-docker:0}}

Vérifiez l’adresse de l’affichage et la version du serveur :

{{CODE:install-ubuntu-docker:1}}

## 2. Télécharger POSIM

{{CODE:install-ubuntu-docker:2}}

Le [guide Docker](docker.md) explique comment choisir une révision ou construire vos modifications. Le premier lancement utilise le rendu logiciel, qui peut être plus lent qu’une installation native avec GPU.

## 3. Autoriser l’accès à votre affichage

Copiez l’autorisation X11 de la session dans un fichier lisible uniquement par votre compte :

{{CODE:install-ubuntu-docker:3}}

La dernière commande vérifie que le fichier n’est pas vide. Si elle échoue, assurez-vous que le terminal appartient à votre session graphique et que XWayland est disponible. Corrigez l’autorisation avant de continuer.

## 4. Créer le conteneur

{{CODE:install-ubuntu-docker:4}}

Enregistrez vos modèles et acquisitions dans `/work`, associé au volume `posim-data`. Utilisez `docker stop posim` et `docker start posim` pour arrêter et reprendre le conteneur. Si l’affichage échoue après une nouvelle ouverture de session Linux, actualisez le fichier d’autorisation.

Pour les exercices, [préparez deux terminaux](terminals.md) reliés à ce même conteneur. Consultez la [configuration GPU](docker.md#graphics-and-gpu-access) avant d’activer l’accélération graphique ou le sonar CUDA.

## 5. Vérifier le monde de démonstration

Entrez dans le conteneur :

{{CODE:install-ubuntu-docker:5}}

Exécutez ensuite la vérification finale à l’intérieur :

{{CODE:install-ubuntu-docker:6}}

Gazebo doit ouvrir le monde océanique. Laissez le terminal ouvert pendant l’exécution et arrêtez avec **Ctrl+C**. Passez ensuite à [votre première simulation](quickstart.md).
