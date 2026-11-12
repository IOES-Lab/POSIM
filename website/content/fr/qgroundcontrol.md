# Piloter avec QGroundControl

Connectez QGroundControl (QGC) au BlueROV2 simulé, utilisez les joysticks virtuels et regardez l’image de sa caméra avant. Terminez l’[installation](requirements.md), puis préparez les [terminaux A et B](terminals.md).

## 1. Ouvrir QGC dans l'environnement de POSIM

**Windows/WSL :** utilisez la version Linux de QGC pour ces tutoriels. Ne lancez pas l’application installée sous Windows. Démarrez QGC dans la même distribution Ubuntu que POSIM ou, avec Docker, dans le même conteneur POSIM. WSLg peut afficher la fenêtre Linux sur le bureau Windows ; l’application continue de fonctionner dans Linux.

Les adresses `127.0.0.1` de ce guide désignent l’environnement où QGC s’exécute. Elles ne configurent pas l’application Windows pour recevoir la télémétrie ou la vidéo provenant de WSL ou de Docker.

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

Ouvrez **Application Settings → Fly View → Virtual Joystick** :

- Activez **Virtual Joystick**.
- Activez **Auto-Center Throttle** pour ramener la consigne au centre lorsque vous relâchez la manette.
- Laissez **Left-Handed Mode** désactivé pour garder la disposition de cet exemple.

## 3. Régler la réception vidéo

Ouvrez **Application Settings → Video**. Choisissez d'abord la source vidéo, puis renseignez l'adresse UDP et les autres réglages du tableau.

| Réglage | Valeur |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`, proportions automatiques |
| Disable Video Stream When Disarmed | Désactivé |
| Low Latency Mode | Activé |
| Auto-reconnect on stream loss | Activé |

## 4. Lancer le robot et la caméra

Arrêtez toute simulation précédente. Dans le terminal A préparé :

{{CODE:qgroundcontrol:1}}

Gazebo ouvre la scène sous-marine et QGC se connecte à ArduSub. Attendez la fin de l'initialisation. Dans le terminal B :

{{CODE:qgroundcontrol:2}}

Revenez à **Fly View** et cliquez sur l'aperçu vidéo pour l'agrandir.

{{MEDIA:qgroundcontrol}}

La caméra produit des images de **640 × 480 à 5 Hz en temps simulé**. Le rendu logiciel peut ralentir la simulation. BlueROV2 possède une flottabilité positive : même désarmé, il peut remonter et perdre les objets de vue.

`qgc_video:=true` ajoute l'émetteur H.264. La vidéo RVB inclut les [effets sous-marins du monde](underwater-rendering.md). Le [filtre de caméra](camera.md) indépendant conserve sa sortie ROS. Le GNSS perd la position quand l'antenne est immergée et la retrouve après émersion ; télémétrie, vidéo et commande manuelle restent connectées. Utilisez Manual sous l'eau. L'objectif est plus bas que l'antenne : une image sous-marine peut donc coexister avec une position GNSS valide.

## 5. Armer, déplacer et arrêter

1. Attendez **Ready**, puis sélectionnez **Manual**.
2. Ouvrez le menu d'état du véhicule et maintenez **Arm** jusqu'à la fin de l'action.
3. Déplacez brièvement la manette droite vers l'avant, puis vers l'arrière.
4. Relâchez-la pour ramener la consigne au neutre. Le courant et l'inertie peuvent encore déplacer le robot.
5. Maintenez **Disarm** dans le menu d'état lorsque vous avez terminé.

Gardez les commandes au clavier et le joystick web désactivés pour éviter les consignes concurrentes. Arrêtez la vidéo depuis B :

{{CODE:qgroundcontrol:3}}

Appuyez sur **Ctrl+C** dans A, puis fermez QGC.

MAVROS attend jusqu’à 60 secondes sans message de présence du véhicule avant de signaler une perte de connexion. Ce délai tient compte du rendu logiciel lent ; une interruption réelle de ces messages peut donc mettre jusqu’à 60 secondes à apparaître dans `/mavros/state`.

## Résoudre un problème de connexion

Les options `qgc_video` et `qgc_video_port` ci-dessous concernent le lancement séparé de l’étape 4. Les [premières simulations](quickstart.md) incluent déjà un émetteur vidéo ; utilisez leur commande `/tutorial/camera/enable_streaming`. Une forte charge du sonar peut espacer les images et faire apparaître temporairement **Waiting for video** dans QGC. Gardez **Low Latency Mode** activé et essayez le tutoriel sous-marin sans sonar.

- **Véhicule absent :** vérifiez que QGC utilise le même environnement Linux que POSIM et qu'aucune autre instance n'occupe UDP 14550.
- **Waiting for video :** vérifiez la source H.264, l'adresse UDP et `qgc_video:=true`. Activez l'envoi depuis B et attendez quelques images.
- **Autre port :** utilisez `qgc_video_port:=5602` et saisissez `127.0.0.1:5602` dans QGC.
- **Option `qgc_video` absente :** mettez POSIM à jour ou [téléchargez l'image et recréez le conteneur](docker.md#update-an-existing-container).
- **Avertissement de firmware ou de port série :** la simulation utilise la version fournie d'ArduSub et une liaison réseau ; aucune programmation du firmware ni configuration série n'est nécessaire.

Consultez les [réglages vidéo officiels](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html) et les [interfaces ROS des propulseurs](robot-control.md).
