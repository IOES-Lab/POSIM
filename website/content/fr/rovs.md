# ROV et BlueROV2

L'argument `namespace` sélectionne le répertoire d'un modèle installé. Les topics ROS suivent la configuration de ce modèle.

## Modèles disponibles

| Modèle | Utilisation |
|---|---|
| `rexrov` | Propulseurs Gazebo, hydrodynamique et odométrie |
| `bluerov2` | BlueROV2 avec ArduSub SITL et MAVROS |
| `bluerov2_heavy` | Configuration Heavy |
| `bluerov2_heavy_multibeam_sonar` | Heavy avec sonar CUDA |

Utilisez le [terminal préparé pour votre installation](terminals.md). Pour REXROV, suivez la [première simulation](quickstart.md).

## Lancer BlueROV2 {#start-bluerov2}

{{CODE:rovs:0}}

Choisissez la variante Heavy avec `namespace:=bluerov2_heavy`. Vérifiez la connexion de l'autopilote dans les messages de démarrage et `/mavros/state`. Le modèle avec sonar nécessite la [configuration CUDA](sonar-tuning.md).

## Commander au clavier

Dans un terminal interactif du bureau, utilisez `gui:=true headless:=false use_teleop:=true`.

| Touche | Action |
|---|---|
| `c` / `x` | Armer / désarmer |
| `w` / `s` | Avancer / reculer |
| `a` / `d` | Tourner à gauche / droite |
| `r` / `f` | Monter / descendre |
| `h` / `j` | ALT_HOLD / STABILIZE |
| Espace | Consigne neutre |

## Utiliser le joystick du navigateur

Servez la page installée depuis un autre terminal préparé :

{{CODE:rovs:1}}

Lancez BlueROV2 avec la téléopération :

{{CODE:rovs:2}}

Gardez `use_teleop:=true` : ce lancement contient le pont WebSocket. Si le navigateur ne s'ouvre pas, ouvrez l'URL sur le même bureau Linux. Les images Docker incluent Firefox et QGC. La page [QGroundControl](qgroundcontrol.md) explique son installation, les joysticks virtuels et la vidéo en direct.

Pour les abonnements et commandes depuis vos programmes, passez à [ROS](ros.md).

## Voir le robot en mouvement

{{MEDIA:rovs}}

Images et vidéos : POSIM Notion Wiki. L'attribution à DAVE figure dans [Citations et licences](citation.md). Utilisez les commandes et topics de cette page.
