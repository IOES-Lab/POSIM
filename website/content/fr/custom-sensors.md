# Ajouter et configurer des capteurs

Un capteur nécessite sa définition dans le modèle, le système qui le met à jour et un chemin jusqu'à l'application réceptrice. Ajoutez-en un à la fois et vérifiez ses mesures.

## Environnement de travail

Utilisez les [terminaux Linux préparés](terminals.md) sous Ubuntu natif, WSL ou Docker. Sur macOS, travaillez dans le bureau RDP. Modifiez et compilez dans cet environnement ; `POSIM_WS` désigne son espace de travail.

## Choisir le point d'intégration

| Mesure | Emplacement dans le modèle | Système du monde / connexion ROS |
|---|---|---|
| IMU | `link/sensor type="imu"` | Système IMU ; pont de `gz.msgs.IMU` vers ROS `Imu` |
| [Caméra sous-marine](camera.md) | Capteur RGBD et plugin d'effets | Système Sensors avec moteur de rendu et topics caméra |
| [DVL](dvl.md) | Capteur DVL et faisceaux | Système DVL et pont de messages acoustiques fourni |
| [Pression](pressure.md) | Plugin directement sous `model` | Publication ROS directe de pression et profondeur |
| [GNSS marin](gnss.md) | Plugin du récepteur sur le modèle | Sorties ROS configurées du récepteur |
| [USBL](usbl.md) | Plugins du transcepteur et du transpondeur | Identifiants acoustiques concordants et interfaces ROS |
| [Sonar multifaisceaux](sonar.md) | Capteur personnalisé sous un lien | Rendu Sensors, système multifaisceaux du monde et CPU/WGPU/CUDA |

Consultez le SDF et les paramètres de chaque référence. Respectez leur emplacement : les systèmes du monde appartiennent au monde, tandis que les plugins de modèle nécessitent l'entité sur laquelle ils agissent.

## Ajouter une IMU au monde d'exemple

Ouvrez `calm_water.sdf` de [Créer un monde](custom-worlds.md). Ajoutez ce système directement dans `<world>`, à côté du système physique :

{{CODE:custom-sensors:0}}

Dans `<link name="base_link">` du modèle `pressure_station`, ajoutez :

{{CODE:custom-sensors:1}}

La pose est relative au lien, en mètres et radians. La fréquence est exprimée en temps simulé. La station étant fixe, la vitesse angulaire doit rester proche de zéro ; l'accélération inclut la réponse à la gravité.

## Relier et observer

Arrêtez le monde précédent. Dans le terminal A, validez et relancez :

{{CODE:custom-sensors:2}}

Dans le **terminal B**, avec l'environnement chargé, démarrez le pont et laissez-le tourner :

{{CODE:custom-sensors:3}}

Dans le **terminal C**, préparé de la même façon, lisez une mesure :

{{CODE:custom-sensors:4}}

Vérifiez horodatage, identifiant de repère, vitesse angulaire et accélération. Retirez `--once` pour une lecture continue ; Ctrl+C arrête l'affichage. À la fin, arrêtez pont et simulateur. [Terminaux et topics ROS](ros.md) explique comment partager l'environnement.

## Modifier la fréquence

Passez `update_rate` de `20` à `5`, puis redémarrez monde et pont. Les intervalles d'horodatage doivent passer d'environ **0,05 s à 0,2 s** de temps simulé. La fréquence reçue en temps réel dépend aussi du facteur de temps réel et du transport.

Pour une caméra, vérifiez ensemble résolution, champ de vision et distances proche et lointaine. Pour un capteur à balayage, vérifiez faisceaux et limites de portée. Avant de déplacer un capteur, confirmez son repère et son orientation.

## Vérifier l'intégration

- Modèle : pose valide, paramètres et topic unique.
- Monde : système requis et moteur de rendu pour les capteurs d'image.
- ROS : sens du pont, type de message, repère et QoS.
- Données : horodatages récents et réaction à une modification contrôlée.

Vérifiez le fond avec les [terrains](heightmaps.md). Pour comparer des sonars, utilisez [Configuration et performances](sonar-tuning.md) avec la [référence des paramètres](sonar.md).
