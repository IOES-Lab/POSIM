# Piloter avec QGroundControl

Connectez QGroundControl (QGC) à BlueROV2, utilisez les joysticks virtuels et regardez l'image de sa caméra avant. Commencez par la [première simulation](quickstart.md), puis préparez les [terminaux A et B](terminals.md).

## 1. Ouvrir QGC dans l'environnement de POSIM

| Installation | Où lancer QGC |
|---|---|
| Ubuntu natif | Installez et lancez l'application Linux dans Ubuntu. |
| Windows avec WSL natif | Installez et lancez l'application Linux dans la même distribution Ubuntu. |
| Linux ou Windows avec Docker | Lancez l'application fournie dans le conteneur POSIM. |
| macOS avec Docker et RDP | Lancez l'application depuis un terminal du bureau RDP. |

Pour une installation native, suivez le [guide officiel de QGC](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html) et choisissez la version Linux adaptée au processeur. Dans Docker, utilisez le terminal B :

{{CODE:qgroundcontrol:0}}

Le `&` rend la main au terminal sans fermer QGC. Si vous avez lancé l'application dans un autre terminal, gardez-le ouvert. Lorsque les préférences du premier démarrage apparaissent, choisissez **Metric System**.

QGC, MAVROS et Gazebo partagent le même environnement Linux. La télémétrie utilise **UDP 14550**, la vidéo **UDP 5600**. Aucun de ces ports ne doit être publié par Docker dans cette configuration. Sous macOS, RDP donne accès à cet environnement.

## 2. Préparer les joysticks

Dans **Application Settings → Fly View → Virtual Joystick**, activez **Virtual Joystick** et **Auto-Center Throttle**. Laissez **Left-Handed Mode** désactivé. Relâcher une manette ramène alors sa consigne au centre.

{{MEDIA:qgroundcontrol:0}}

## 3. Régler la réception vidéo

Dans **Application Settings → Video** :

| Réglage | Valeur |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`, proportions automatiques |
| Disable Video Stream When Disarmed | Désactivé |
| Auto-reconnect | Activé |

{{MEDIA:qgroundcontrol:1}}

## 4. Lancer le robot et la caméra

Arrêtez toute simulation précédente. Dans le terminal A préparé :

{{CODE:qgroundcontrol:1}}

Gazebo ouvre la scène sous-marine et QGC se connecte à ArduSub. Attendez la fin de l'initialisation. Dans le terminal B :

{{CODE:qgroundcontrol:2}}

Revenez à **Fly View** et cliquez sur l'aperçu vidéo pour l'agrandir.

{{MEDIA:qgroundcontrol:2}}

La caméra produit des images de **640 × 480 à 5 Hz en temps simulé**. Le rendu logiciel peut ralentir la simulation. BlueROV2 possède une flottabilité positive : même désarmé, il peut remonter et perdre les objets de vue.

`qgc_video:=true` ajoute l'émetteur H.264 pour cette exécution. Il diffuse la caméra RVB normale ; le [filtre de caméra sous-marine](camera.md) dispose d'une sortie ROS distincte.

## 5. Armer, déplacer et arrêter

1. Attendez **Ready**, puis sélectionnez **Manual**.
2. Ouvrez le menu d'état du véhicule et maintenez **Arm** jusqu'à la fin de l'action.
3. Déplacez brièvement la manette droite vers l'avant, puis vers l'arrière.
4. Relâchez-la pour ramener la consigne au neutre. Le courant et l'inertie peuvent encore déplacer le robot.
5. Maintenez **Disarm** dans le menu d'état lorsque vous avez terminé.

Gardez les commandes au clavier et le joystick web désactivés pour éviter les consignes concurrentes. Arrêtez la vidéo depuis B :

{{CODE:qgroundcontrol:3}}

Appuyez sur **Ctrl+C** dans A, puis fermez QGC.

## Résoudre un problème de connexion

- **Véhicule absent :** vérifiez que QGC utilise le même environnement Linux que POSIM et qu'aucune autre instance n'occupe UDP 14550.
- **Waiting for video :** vérifiez la source H.264, l'adresse UDP et `qgc_video:=true`. Activez l'envoi depuis B et attendez quelques images.
- **Autre port :** utilisez `qgc_video_port:=5602` et saisissez `127.0.0.1:5602` dans QGC.
- **Option `qgc_video` absente :** mettez POSIM à jour ou [téléchargez l'image et recréez le conteneur](docker.md#update-an-existing-container).
- **Avertissement de firmware ou de port série :** la simulation utilise la version fournie d'ArduSub et une liaison réseau ; aucune programmation du firmware ni configuration série n'est nécessaire.

Consultez les [réglages vidéo officiels](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html) et les [interfaces ROS des propulseurs](robot-control.md).
