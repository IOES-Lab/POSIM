# Ubuntu : installation native

Compilez POSIM sous Ubuntu 26.04. Ouvrez l’application Terminal et exécutez les blocs un par un, en attendant la fin de chaque étape. Pour utiliser une image prête à l’emploi, suivez [Linux avec Docker](install-ubuntu-docker.md).

## 1. Récupérer les sources

{{CODE:install:0}}

Conservez le nom du répertoire `posim` : le manifeste des dépôts l’utilise dans les étapes suivantes.

## 2. Installer la pile logicielle

Lisez d’abord `src/posim/extras/ros-lyrical-gz-jetty-install.sh`. Ce script configure des dépôts, utilise `sudo` et lance `apt-get full-upgrade -y`. Un environnement Ubuntu dédié est recommandé.

{{CODE:install:1}}

Il installe ROS 2 Lyrical, Gazebo Jetty, ArduSub, ArduRover, MAVROS et les bibliothèques de vagues. Wave Sim est récupéré à une révision fixée puis compilé comme dépendance externe ; un clonage récursif de POSIM n’est pas nécessaire.

## 3. Préparer les dépendances de l’espace de travail

{{CODE:install:2}}

`--skip-existing` conserve le dépôt POSIM déjà présent. Vérifiez chaque erreur de dépendance avant de compiler.

Le manifeste importe également une révision fixée de `ros_gz` avec les corrections du cycle de vie du pont et du lancement POSIX. Un répertoire `src/ros_gz` existant reste inchangé : comparez son origine et sa révision au manifeste, ou utilisez un nouvel espace de travail.

## 4. Compiler et charger l’environnement

{{CODE:install:3}}

Les deux chemins `ros_gz` doivent appartenir au répertoire `install/` de cet espace de travail. Un chemin sous `/opt/ros/lyrical` indique que le paquet système reste sélectionné. Dans chaque nouveau terminal, chargez ROS et l’espace de travail comme indiqué dans [Préparer les terminaux](terminals.md).

### Si la compilation manque de mémoire

`--executor sequential` sérialise les paquets, mais pas les processus du compilateur. Pour limiter les deux :

{{CODE:install:5}}

### Mettre à jour l’espace de travail

Enregistrez vos modifications dans Git avant la mise à jour. Notez la révision, récupérez les changements, puis reprenez la résolution des dépendances et la compilation :

{{CODE:install:6}}

Passez à la [première simulation](quickstart.md). Pour le sonar, consultez la [compilation et la configuration CPU/WGPU/CUDA](sonar-tuning.md).

## 5. Lancer le monde de démonstration

Cette dernière vérification ouvre la scène océanique dans Gazebo. Le premier lancement peut télécharger des modèles Fuel. Arrêtez avec **Ctrl+C** avant de poursuivre avec [les terminaux](terminals.md).

{{CODE:install:7}}
