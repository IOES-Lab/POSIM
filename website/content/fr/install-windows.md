# Windows : installation dans WSL

Installez POSIM dans Ubuntu 26.04 avec Windows Subsystem for Linux 2 (WSL2). WSLg affiche Gazebo sur le bureau Windows. Pour utiliser une image prête à l’emploi, choisissez [WSL avec Docker](install-windows-docker.md).

## 1. Installer WSL et Ubuntu

Ouvrez PowerShell en tant qu’administrateur :

{{CODE:install-windows:0}}

Redémarrez Windows si nécessaire. Ouvrez Ubuntu et créez votre compte Linux. Dans PowerShell, vérifiez que la distribution utilise WSL 2 :

{{CODE:install-windows:1}}

Consultez le [guide Microsoft](https://learn.microsoft.com/fr-fr/windows/wsl/install) si WSL est déjà installé ou si l’installation échoue.

## 2. Préparer le pilote graphique et WSLg

Installez ou mettez à jour le **pilote Windows** de votre GPU Intel, AMD ou NVIDIA. Cette étape concerne aussi NVIDIA. WSLg utilise ce pilote pour l’accélération graphique ; suivez le [guide des applications graphiques](https://learn.microsoft.com/fr-fr/windows/wsl/tutorials/gui-apps).

Avec NVIDIA, suivez [CUDA sous WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) : installez le pilote Windows, pas de pilote d’affichage Linux dans WSL. Le toolkit CUDA est nécessaire uniquement pour compiler le backend CUDA du sonar.

Mettez WSL à jour depuis PowerShell :

{{CODE:install-windows:2}}

Enregistrez le travail des terminaux WSL avant tout redémarrage demandé, puis rouvrez Ubuntu.

## 3. Installer POSIM dans Ubuntu

Utilisez le profil **Ubuntu** de Windows Terminal pour les commandes Linux. Vérifiez la distribution :

{{CODE:install-windows:3}}

Suivez [Ubuntu : installation native](install.md), du téléchargement des sources à la compilation. Conservez l’espace de travail dans `~/posim_ws`, dans le système Linux. L’installateur utilise `sudo` et met à jour les paquets Ubuntu.

Si les topics d’image sont visibles mais que les images n’arrivent pas, consultez le [dépannage](troubleshooting.md). [Préparer les terminaux](terminals.md) explique les deux fenêtres utilisées dans la suite.

## 4. Lancer le monde de démonstration

Exécutez ces commandes dans Ubuntu. Gazebo doit s’ouvrir sur le bureau Windows et afficher l’océan. Au premier lancement, des modèles Fuel peuvent être téléchargés. Arrêtez la simulation avec Ctrl+C avant de lancer un autre exemple.

{{CODE:install-windows:4}}
