# Commander avec Python

Gardez REXROV actif avec la [première simulation](quickstart.md). Dans un autre [terminal préparé](terminals.md), lisez sa position, puis appliquez une brève impulsion à un propulseur.

## S'abonner à la position

Enregistrez ce programme sous `observe.py`, puis lancez `python3 observe.py` dans l'environnement ROS chargé.

{{CODE:python-control:0}}

La position s'affiche environ une fois par seconde. Ctrl+C arrête l'observateur sans fermer le simulateur.

## Envoyer une commande limitée

Enregistrez le programme suivant sous `pulse_thruster.py` et lancez `python3 pulse_thruster.py`. Il attend un abonné, applique **5 N au propulseur 1** pendant environ deux secondes de temps réel, puis envoie zéro. L'axe de montage détermine la force et le couple obtenus.

{{CODE:python-control:1}}

Le réglage des signaux permet à Ctrl+C d'atteindre le bloc de nettoyage alors que la publication ROS reste disponible. En cas d'arrêt forcé du processus, arrêtez la simulation ou envoyez des consignes neutres selon la page [Commande des propulseurs](robot-control.md).

## Observer la réponse

Dans un autre terminal préparé, surveillez la vitesse angulaire pendant l'impulsion :

{{CODE:python-control:2}}

La vitesse doit réagir, puis revenir vers zéro. Comparez l'odométrie pour distinguer la réponse de l'actionneur du déplacement du véhicule. Ctrl+C ferme l'affichage.

## Construire un contrôleur

Répartissez les forces entre les huit propulseurs de REXROV selon l'état mesuré et votre loi de commande. Respectez les topics, les axes et les unités. Utilisez le temps simulé dans la logique de l'expérience et remettez au neutre tous les actionneurs commandés à l'arrêt.

BlueROV2 suit le chemin [MAVROS/ArduSub](robot-control.md). Pour comparer les essais, [enregistrez les observations](visualization.md).
