# Windows : WSL et Docker

Docker Desktop exécute POSIM et WSLg affiche Gazebo sur le bureau Windows. Lancez les commandes Docker de cette page dans le **terminal Ubuntu de WSL**.

## 1. Préparer WSL et l'affichage

Suivez les deux premières étapes de l'[installation sous Windows](install-windows.md) : installez Ubuntu 26.04 dans WSL2, puis mettez à jour WSL et le pilote graphique de **Windows**, y compris avec une carte NVIDIA. N'installez pas de pilote d'affichage Linux dans WSL.

Installez [Docker Desktop avec WSL2](https://docs.docker.com/desktop/features/wsl/) et activez **Settings → Resources → WSL Integration → Ubuntu-26.04**. Utilisez le moteur de Docker Desktop ; un second moteur dans Ubuntu est inutile.

Vérifiez le serveur et le socket d'affichage :

{{CODE:install-windows-docker:0}}

Si le socket est absent, terminez la [configuration de WSLg](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps).

## 2. Télécharger POSIM

{{CODE:install-windows-docker:1}}

La page [Environnement Docker](docker.md) présente les versions et la compilation des images. La configuration ci-dessous utilise le rendu logiciel. Pour une carte NVIDIA compatible, consultez les [conditions d'accès au GPU](docker.md#graphics-and-gpu-access).

## 3. Créer le conteneur

{{CODE:install-windows-docker:2}}

Enregistrez vos fichiers dans `/work`, associé au volume `posim-data`. Utilisez `docker stop posim` pour arrêter le conteneur et `docker start posim` pour le redémarrer.

Pour les exemples suivants, ouvrez [deux terminaux Ubuntu](terminals.md) et connectez-les au même conteneur. Gazebo s'affiche directement dans une fenêtre Windows.

## 4. Lancer le monde de démonstration

Entrez dans le conteneur :

{{CODE:install-windows-docker:3}}

Vérifiez l'installation :

{{CODE:install-windows-docker:4}}

Gazebo doit afficher l'océan. Gardez le terminal ouvert ; **Ctrl+C** arrête la simulation. Passez ensuite à votre [premier robot](quickstart.md).
