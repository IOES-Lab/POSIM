# Véhicules de surface et vagues

Simulez un WAM-V avec les vagues de Wave Sim et ArduRover en mode bateau. L'installateur POSIM et ses recettes Docker compilent les bibliothèques externes de vagues.

## Tutoriel standard

L’installation standard inclut un parcours de bouées WAM-V avec caméra, lidar et vue RViz préconfigurée. L’image optionnelle ci-dessous n’est pas nécessaire. Dans le [terminal POSIM préparé](terminals.md), exécutez :

{{CODE:quickstart:4}}

Suivez [Première simulation](quickstart.md) pour la scène, l’affichage lidar, la vidéo et le pilotage avec QGroundControl.

## Préparer le WAM-V

Cette image facultative utilise l'organisation du bureau ARM64. Sur un Mac Apple Silicon, préparez [Docker et RDP](install-macos.md), puis exécutez ces commandes depuis une copie du dépôt POSIM dans le Terminal macOS :

{{CODE:surface:0}}

Connectez-vous par RDP à **127.0.0.1:3390** avec le compte `docker`. Dans un terminal du bureau Linux :

{{CODE:surface:1}}

L'image contient des binaires ArduSub et ArduRover distincts. L'exemple choisit ArduRover avec `FRAME_CLASS=2`. Lancez **une instance par environnement** : nom du véhicule, fichiers générés et ports du contrôleur sont fixes. Plusieurs véhicules nécessitent des fichiers, noms, identifiants et ports séparés, ou des environnements isolés.

Le contrôleur attend l'apparition du véhicule et les données de son IMU. Si la création échoue ou dépasse 180 s, ou qu'aucune mesure IMU n'arrive dans les 180 s suivantes, le lancement s'arrête en erreur en précisant l'étape. Examinez les messages Gazebo et du capteur avant de réessayer.

## Modèle, commande et navigation

`extras/surface/wamv.py` prépare le modèle externe. Les hélices arrière utilisent les sorties 1 et 3 ; le véhicule démarre à la surface.

L'adaptateur de navigation externe transmet à MAVROS la **pose et la vitesse simulées par Gazebo**. Vérifiez l'état du contrôleur, la validité de la navigation et le sens de propulsion avant d'envoyer une mission.

Pour étudier le bruit, le contact avec l'eau ou la réacquisition du récepteur, intégrez le [GNSS marin](gnss.md), son antenne et un adaptateur de navigation adapté. L'adaptateur de pose externe de cet exemple constitue une autre entrée.

## Régler les vagues

Les commandes utilisent `gz.msgs.Param` sur `/world/<world_name>/waves`. Le nom vient de `<world name="…">`. Listez les topics actifs :

{{CODE:surface:2}}

Le [lancement WAM-V](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py) utilise par défaut `posim_surface`, donc `/world/posim_surface/waves`. Changez le nom dans le conteneur préparé :

{{CODE:surface:3}}

Le topic devient `/world/harbour/waves` ; l'IMU ROS reste sur `/model/wamv/imu`. `world_name` nomme le monde généré : il ne choisit pas un fichier de terrain et n'isole pas une seconde instance.

Le fond est plat, à z = −10 m. Consultez les [terrains par carte de hauteur](heightmaps.md) pour le remplacer.

| Paramètre (`double`) | Unité | Rôle |
|---|---|---|
| `wind_speed` | m/s | Vitesse du vent du modèle de vagues |
| `wind_angle` | degrés | Direction du vent |
| `steepness` | sans dimension | Cambrure des vagues |

Gardez les réglages visuels et hydrodynamiques cohérents. Les [courants](currents.md) se configurent séparément. Outre l'installation des bibliothèques, le monde doit inclure et configurer les systèmes de vagues.

## Limites et dépendances

Le Dockerfile facultatif prend en charge l'organisation ARM64. Calibrez coque, propulsion et traînée pour le bateau et ses conditions d'utilisation. À l'arrêt, MAVROS ou Gazebo peuvent demander une terminaison par le superviseur ; vérifiez leur sortie avant de relancer.

`extras/surface/dependency.json` fixe la révision externe. L'installateur compile dans `/opt/waves` et conserve les sources dans `/opt/asv_wave_sim` ; il s'agit d'une dépendance de compilation, pas d'un sous-module Git. Consultez le [guide d'intégration](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) et les [licences](citation.md).
