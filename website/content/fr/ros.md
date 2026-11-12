# Terminaux et topics ROS

Le simulateur tourne dans A ; B sert à lire les données ou à envoyer des commandes. Les deux terminaux doivent charger la même distribution ROS et le même espace de travail POSIM.

## Suivre les données

<div class="docs-flow"><div><strong>Programmes et commandes ROS</strong><span>Abonnements, services et consignes</span></div><div><strong>Interface ROS</strong><span>Pont ros_gz ou publication par un plugin</span></div><div><strong>Gazebo et plugins POSIM</strong><span>Physique, état et mesures</span></div></div>

Les mesures remontent vers le programme ROS ; les commandes vont au simulateur. Un topic Gazebo devient disponible dans ROS lorsqu'un pont ou un plugin ROS le publie. Le lancement du robot charge sa configuration de ponts.

## Préparer A et B

Suivez la [préparation des terminaux adaptée à votre installation](terminals.md). Gardez la [première simulation](quickstart.md) active dans A et exécutez les commandes suivantes dans B.

Ces topics correspondent à BlueROV2 sans sonar. Avec le sonar, remplacez `bluerov2` par `bluerov2_heavy_multibeam_sonar` ; pour l’exemple de surface, utilisez `wamv`.

## Examiner les connexions

`--no-daemon` interroge directement ROS, sans dépendre du cache du démon de la CLI.

{{CODE:ros:0}}

`ros2 topic list` énumère les topics ROS ; `gz topic -l`, ceux de Gazebo Transport. Vérifiez type de message, publications, abonnements et QoS avant de connecter votre programme.

## Observer une mesure

{{CODE:ros:1}}

Une position presque fixe reste normale si les mesures et leurs horodatages continuent d'arriver. **Ctrl+C dans B** arrête l'affichage ; A poursuit la simulation. Utilisez `--once` pour une seule mesure. Les nœuds utilisant le temps simulé doivent définir `use_sim_time:=true` et recevoir `/clock`.

## Connecter un autre ordinateur ROS

Utilisez des paquets ROS et messages compatibles, le même domaine et un réseau permettant la découverte et les échanges DDS. Installez `posim_interfaces` pour les messages propres à POSIM. Si des publications existent sans mesures reçues, consultez le [dépannage](troubleshooting.md).

Poursuivez avec [Python](python-control.md), les [propulseurs et joysticks](robot-control.md) ou la [visualisation et l'enregistrement](visualization.md).
