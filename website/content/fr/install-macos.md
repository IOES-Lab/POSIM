# macOS : Docker et RDP

Sur un Mac avec Apple Silicon, lancez l’image ARM64 de POSIM et connectez-vous à son bureau Linux par RDP. Gazebo et les terminaux des exemples s’ouvrent dans ce bureau.

## 1. Préparer Docker et le client RDP

Installez puis démarrez [Docker Desktop pour Mac](https://docs.docker.com/desktop/setup/install/mac-install/). Installez aussi [Windows App pour macOS](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps) pour la connexion au bureau distant.

Dans l’application Terminal de macOS :

{{CODE:install-macos:0}}

Apple Silicon doit afficher `arm64`. La seconde commande doit donner la version du serveur Docker.

## 2. Télécharger l’image et créer le conteneur

Docker sélectionne automatiquement ARM64 :

{{CODE:install-macos:1}}

Le port RDP n’est accessible que depuis ce Mac. Enregistrez vos modèles et acquisitions dans `/work` pour les conserver dans le volume `posim-data`. Le [guide Docker](docker.md) détaille les versions d’images et la construction locale.

## 3. Se connecter au bureau Linux

Dans Windows App, ajoutez un PC à l’adresse **127.0.0.1:3389**. Sur l’écran de connexion xrdp, choisissez **Xorg**, puis utilisez l’identifiant **docker** et le mot de passe **docker**.

Ouvrez un terminal dans le bureau Linux et changez le mot de passe de ce compte de développement local :

{{CODE:install-macos:2}}

Saisissez les commandes POSIM dans ce bureau. Gardez le Terminal de macOS pour gérer Docker. La page [Préparer les terminaux](terminals.md) explique où se trouve l’espace de travail.

## 4. Arrêter et reprendre

Fermer RDP n’arrête pas la simulation. Appuyez d’abord sur **Ctrl+C** dans son terminal. Depuis macOS :

{{CODE:install-macos:3}}

Reconnectez-vous avec Windows App après le démarrage du conteneur. Le rendu Linux est logiciel ; les scènes complexes peuvent être lentes. Pour le sonar, commencez par CPU après avoir [compilé et vérifié les paquets](sonar-tuning.md#build-the-sonar-packages). Metal est accessible aux processus macOS natifs, pas à ce conteneur Linux. Les [caméras et le capteur de pression](features.md) permettent une première vérification.

## 5. Vérifier l’installation

Dans un terminal **du bureau RDP**, lancez la vérification finale :

{{CODE:install-macos:4}}

Gazebo doit afficher le monde océanique. Laissez le terminal ouvert pendant l’exécution. Arrêtez avec **Ctrl+C**, puis passez à [votre premier robot](quickstart.md).

{{MEDIA:install-macos}}
