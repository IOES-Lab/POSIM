# Propulseurs et commande au joystick

Choisissez l'interface adaptée au robot. Vérifiez les capteurs, puis activez un seul contrôleur à la fois.

| Véhicule | Chemin de commande | Guide |
|---|---|---|
| REXROV | Forces ROS → propulseurs Gazebo | [Python](python-control.md) |
| BlueROV2 | ROS/joystick → MAVROS → ArduSub | [QGC et vidéo](qgroundcontrol.md) · [ROS/joystick](rovs.md) |
| WAM-V | Intégration de surface et contrôleur du véhicule | [Robots de surface](surface.md) |

## Propulseurs de REXROV

Dans les [terminaux préparés](terminals.md), gardez REXROV dans A et envoyez les commandes depuis B. Sa configuration relie huit topics `cmd_thrust` de type `std_msgs/msg/Float64`. Les valeurs sont des poussées en newtons. Pour remettre le premier propulseur au neutre :

{{CODE:robot-control:0}}

Répartissez les forces selon la position et l'axe de chaque propulseur. Réguler la vitesse du véhicule exige de les coordonner. À la fin, envoyez zéro à tous les propulseurs utilisés.

## BlueROV2 avec ArduSub

Lancez l'[exemple BlueROV2](rovs.md#start-bluerov2), attendez la connexion de l'autopilote et examinez les interfaces :

{{CODE:robot-control:1}}

La durabilité `volatile` sélectionne les nouvelles mesures ; le filtre attend `connected: true` après l'établissement de la liaison. Si l'attente se prolonge, vérifiez les journaux de lancement et la configuration MAVROS/ArduSub.

L'adaptateur convertit `/joy` et `/keyboard/joy` en commandes MAVROS :

- Fréquence de publication : **20 Hz**.
- Expiration d'une entrée : **0,3 s**.

Utilisez une seule source de commandes et vérifiez l'armement et l'état de navigation. La page [ROV et BlueROV2](rovs.md) décrit le clavier et le pont WebSocket.
