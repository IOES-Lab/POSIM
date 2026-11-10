# Afficher et enregistrer les données des capteurs

Gardez le simulateur actif dans le terminal A. Utilisez B pour vérifier les mesures, ouvrir un outil de visualisation ou enregistrer une séquence.

## Recevoir une image

Préparez [les deux terminaux dans le même environnement](terminals.md). Arrêtez la simulation précédente avec Ctrl+C dans A, puis lancez cette scène. Gazebo s'affiche sur le bureau préparé pendant l'installation.

{{CODE:visualization:0}}

Dans B :

{{CODE:visualization:1}}

Vérifiez que largeur et hauteur sont non nulles, puis contrôlez l'encodage et l'horodatage. `--no-arr` masque le tableau de pixels. Pour voir l'image, ajoutez un affichage **Image** dans RViz et sélectionnez ce topic. La page [Caméra sous-marine](camera.md) décrit les réglages d'atténuation.

## Choisir un affichage

Dans RViz, utilisez **Image** pour les caméras, **PointCloud2** pour les nuages de points et les affichages **TF/pose** pour les repères et l'état du robot.

Chaque client ROS séparé doit installer et charger `posim_interfaces` pour les messages propres à POSIM. Les références [caméra](camera.md), [pression](pressure.md), [DVL](dvl.md), [USBL](usbl.md) et [sonar](sonar.md) donnent les topics et leurs types. Les pages [Courants](currents.md) et [Coordonnées sphériques](coordinates.md) présentent aussi les services ROS permettant de modifier les entrées de l'expérience.

## Enregistrer et relire

Ce lancement autonome de la caméra ne crée pas de publisher ROS `/clock`. Pour enregistrer aussi le temps de simulation, préparez le terminal C dans le même environnement ROS que A et B et laissez fonctionner le pont unidirectionnel suivant. Ne lancez pas un second pont si `/clock` possède déjà un publisher.

{{CODE:visualization:2}}

Dans le terminal B, vérifiez la réception d’un message d’horloge avant de lancer l’enregistrement :

{{CODE:visualization:3}}

{{CODE:visualization:4}}

Appuyez sur Ctrl+C pour terminer. Choisissez un nouveau dossier pour chaque enregistrement, puis vérifiez le nombre de messages :

{{CODE:visualization:5}}

Après l’enregistrement, vérifiez que le bag contient le topic image et `/clock`. Arrêtez le pont du terminal C avec Ctrl+C avant de relire le bag.

Arrêtez le simulateur avant de relire les données sur les mêmes topics :

{{CODE:visualization:6}}
