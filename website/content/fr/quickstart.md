# Première simulation

Explorez une scène d’inspection en eau peu profonde avec BlueROV2, puis un parcours de bouées avec WAM-V. Chaque exemple ouvre Gazebo et une fenêtre RViz préconfigurée. Exécutez une seule simulation à la fois.

## Avant de commencer

Terminez une [installation](requirements.md), puis chargez le même espace de travail ROS/POSIM dans les [terminaux A et B](terminals.md). A exécute le simulateur ; B consulte les données et démarre la vidéo. Avec Docker, utilisez le bureau Linux, WSLg ou RDP configuré à l’installation. Les deux exemples font partie de l’installation standard, sans image de surface séparée.

## Inspection sous-marine avec BlueROV2

Dans le **terminal A** :

{{CODE:quickstart:0}}

BlueROV2 démarre environ 3,1 m sous la surface moyenne, au-dessus d’un fond situé à 4 m, face à une boîte, un tuyau, un pilier et une sphère. Petites vagues, voile sous-marin, absorption, caustiques et neige marine sont activés. Le ballast est ajusté pour garder le véhicule désarmé près des objets. Gazebo affiche la caméra en direct à côté de la scène.

Pour ajouter le sonar multifaisceaux, ajoutez `sonar:=true` à cette commande. La variante BlueROV2 Heavy avec sonar est alors sélectionnée : Gazebo affiche une image sonar supplémentaire, et RViz l’image sonar et le nuage de points. Le sonar demande beaucoup de calcul et **peut réduire la fréquence d’affichage et la réactivité des commandes**. Selon l’hôte et les réglages du capteur, `auto` essaie CUDA, puis WGPU, puis CPU. Le rendu conserve ses réglages habituels. Voir les [exigences du sonar](sonar-tuning.md).

Les captures ci-dessous utilisent la variante avec sonar. RViz charge automatiquement le modèle, la caméra et les affichages des capteurs. La grille de 1 m sert de repère dans le référentiel du véhicule ; ce n’est pas un fond mesuré. **Move Camera** est sélectionné : faites glisser pour tourner, utilisez le bouton central pour translater et la molette pour zoomer. Les hélices affichées gardent leur angle de repos ; la dynamique simulée reste inchangée.

{{MEDIA:quickstart:0}}

### Vérifier les données

Dans le **terminal B** :

{{CODE:quickstart:1}}

Avec le sonar, remplacez `bluerov2` dans le topic par `bluerov2_heavy_multibeam_sonar`. Les positions doivent continuer d’arriver même si le véhicule est presque immobile. Arrêtez l’affichage avec **Ctrl+C**, puis vérifiez que le temps simulé avance :

{{CODE:quickstart:2}}

Arrêtez cet affichage avec Ctrl+C. Si les messages manquent ou si l’horloge est arrêtée, consultez le [dépannage](troubleshooting.md).

### Voir et piloter avec QGC

Suivez les étapes 1–3 de la [configuration QGC](qgroundcontrol.md). Ouvrez QGroundControl dans le même environnement Linux et activez les joysticks virtuels. Choisissez **UDP h.264 Video Stream**, l’adresse **127.0.0.1:5600**, et désactivez **Disable Video Stream When Disarmed**. Gardez le tutoriel en cours et démarrez la vidéo dans le terminal B :

{{CODE:quickstart:3}}

Cliquez sur le petit panneau vidéo de QGC pour l’agrandir. **No GPS Lock** est normal lorsque l’antenne GNSS marine est immergée. Télémétrie, vidéo et pilotage manuel restent disponibles. Attendez **Ready**, choisissez **Manual**, armez, puis déplacez brièvement le joystick. Relâchez-le au neutre et désarmez. Consultez les [étapes de pilotage QGC](qgroundcontrol.md). Ne lancez pas simultanément un autre contrôleur au clavier ou au joystick.

{{MEDIA:quickstart:1}}

## Perception en surface avec WAM-V

Arrêtez d’abord la simulation sous-marine. Dans le **terminal A** :

{{CODE:quickstart:4}}

WAM-V démarre sur de petites vagues face à quatre bouées ancrées. RViz affiche la caméra ainsi que le véhicule et le nuage lidar dans une vue commune, avec une grille de 1 m et Move Camera sélectionné. Le lidar balaie horizontalement : ses points dessinent les contours à hauteur du capteur, sans reconstruction 3D complète.

Dans le panneau **Lidar** de Gazebo, cliquez sur l’actualisation et choisissez `/tutorial/lidar`. Désactivez **Show Non Hitting Rays** pour ne montrer que les rayons qui rencontrent des objets. Le panneau caméra reste à côté de la scène.

{{MEDIA:quickstart:2}}

La caméra utilise le même topic `/tutorial/camera`, la même commande de démarrage et les mêmes réglages vidéo QGC que l’exemple sous-marin. WAM-V utilise le mode bateau d’ArduRover et la position/vitesse simulée comme navigation externe ; voir [Robots de surface et vagues](surface.md). Cette entrée est distincte du récepteur GNSS marin de BlueROV2.

En mode Manual de QGC, le déplacement vertical du joystick gauche commande les gaz du WAM-V. Armez, déplacez-le brièvement, relâchez-le au neutre, puis désarmez.

{{MEDIA:quickstart:3}}

Lisez un balayage lidar dans le **terminal B** :

{{CODE:quickstart:5}}

Le nuage de points est `/tutorial/lidar/points` ; l’odométrie du véhicule est `/model/wamv/odometry`.

## Terminer la session

Désarmez, fermez RViz et QGC, puis appuyez sur **Ctrl+C dans le terminal A** et attendez la fin des processus. Si RViz propose d’enregistrer les modifications de vue, abandonnez-les pour conserver le préréglage fourni. En cas de fermeture incorrecte, consultez le [dépannage des visualiseurs](troubleshooting.md). Fermer l’affichage d’un topic n’arrête pas la simulation. Terminez la session avant de lancer un autre exemple.

## Pour continuer

- [Afficher et enregistrer les capteurs](visualization.md) : examiner les images et sauvegarder les mesures.
- [Commander avec Python](python-control.md) : lancer l’exemple REXROV séparé et tester un propulseur.
- [ROV et BlueROV2](rovs.md) : utiliser ArduSub, le clavier ou le joystick local.
- [Bibliothèque de mondes](worlds.md) : choisir un environnement et consulter les arguments de lancement.
