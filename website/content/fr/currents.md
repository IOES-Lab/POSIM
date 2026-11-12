# Courants marins

POSIM génère les courants du monde, calcule l'écoulement au niveau de chaque robot et expose des réglages ROS. Le courant uniforme et le profil qui varie avec la profondeur utilisent des topics et des paramètres distincts.

## Lancer l'exemple

Préparez les [terminaux A et B](terminals.md) dans votre environnement POSIM : Ubuntu natif, WSL, Docker ou le bureau distant de macOS. Lancez l'exemple dans A :

{{CODE:currents:0}}

Dans B, consultez Gazebo, puis ROS. Arrêtez le premier affichage avec Ctrl+C avant d'exécuter la seconde commande :

{{CODE:currents:1}}

{{CODE:currents:2}}

## Configurer le courant du monde

Placez les deux plugins directement dans `<world>`, celui du courant avant celui de ROS. Le [monde d'exemple](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world) fournit la configuration complète.

{{CODE:currents:3}}

Utilisez le même `namespace`, non vide, dans les deux plugins. Les blocs `constant_current` et `transient_current` sont obligatoires ; une configuration manquante désactive la publication et produit une erreur. Des modèles de Gauss–Markov distincts décrivent la vitesse et les angles horizontal et vertical du courant uniforme.

| Paramètre | Rôle et valeur par défaut |
| --- | --- |
| `namespace` des deux plugins | Espace de noms commun ; `hydrodynamics` dans l'exemple |
| `constant_current/use_constant_current` | Publier le courant uniforme ; `false` |
| `constant_current/topic` | Topic Gazebo du courant uniforme ; `ocean_current` |
| `transient_current/topic_stratified` | Topic Gazebo des couches dans cet espace de noms ; `stratified_current_velocity` |
| `transient_current/databasefileName` | CSV installé dans `posim_worlds/worlds` ; `transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | Chemin CSV facultatif, prioritaire sur le nom de fichier |
| `mean` de chaque modèle stochastique | Moyenne initiale de vitesse (m/s) ou d'angle (rad) ; 0 |
| `min`, `max` | Bornes ; −1 et 1. Garder `min < max` et la moyenne dans l'intervalle |
| `noiseAmp` | Amplitude de l'incrément aléatoire ; 0 |
| `mu` | Coefficient de décroissance de Gauss–Markov ; 0 |
| `tidal_oscillation` | Bloc facultatif des données de marée |

## Préparer un profil selon la profondeur

Le [CSV d'exemple](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv) contient, dans cet ordre, la vitesse selon l'axe X du monde (m/s), la vitesse selon l'axe Y du monde (m/s) et la profondeur (m). Les noms historiques `velocity_north`, `velocity_east` et `velocity_down` correspondent directement aux axes X, Y et Z du monde.

- Conservez les trois premières lignes de description et d'en-tête.
- Fournissez au moins deux couches, avec exactement trois nombres finis par ligne.
- Les profondeurs doivent être positives ou nulles et strictement croissantes.
- Un profil absent ou mal formé désactive la publication du courant.

Le plugin ROS du monde transmet toute la base de données. Le plugin du modèle interpole le courant à la profondeur du véhicule. L'hydrodynamique tient compte de l'écoulement relatif à la vitesse du robot.

## Appliquer le profil au robot

L'exemple REXROV utilise d'abord le courant uniforme `/ocean_current`. Pour utiliser le profil, activez une seule fois le bloc commenté du [modèle REXROV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf), directement dans `<model>` :

{{CODE:currents:4}}

Ajoutez aussi `<namespace>rexrov</namespace>` dans le plugin existant `gz-sim-hydrodynamics-system`, en conservant son lien, sa masse volumique et ses coefficients de traînée. Recompilez l'espace de travail, puis chargez son environnement.

| Paramètre du modèle | Rôle et valeur par défaut |
| --- | --- |
| `namespace` | Obligatoire ; doit correspondre à celui de l'hydrodynamique, ici `rexrov` |
| `flow_velocity_topic` | Suffixe sous `/model/<namespace>/` ; `ocean_current` |
| `transient_current/topic_stratified` | Entrée ROS de la base de données ; `hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | Réglages stochastiques facultatifs par composante ; sans réglage, aucun bruit ajouté |
| `mean` de chaque composante | Vitesse initiale, m/s ; 0. Remplacée par le profil interpolé à réception des données |
| `mu` | Décroissance entre 0 et 1, appliquée selon les secondes de simulation écoulées ; 0 |
| `noiseAmp` | Écart autorisé autour de la vitesse interpolée, m/s ; 0 |
| `noiseFreq` | Amplitude de l'incrément aléatoire par mise à jour ; 0. Ce n'est pas une fréquence en Hz |
| `tide_oscillation` | Appliquer les données de marée au modèle ; `false` |

La sortie `/model/rexrov/ocean_current` existe dans Gazebo (`gz.msgs.Vector3d`) et ROS (`geometry_msgs/msg/TwistStamped`). Contrôlez cette sortie et la réponse du véhicule.

La profondeur vient de la position du modèle dans le monde ; elle vaut zéro au-dessus de l'eau. Le courant est interpolé entre les couches et prend la valeur de la couche la plus proche hors du profil. La pause ou un horodatage de simulation répété ne fait pas avancer le calcul. Un message de base de données invalide est rejeté entièrement ; le dernier profil valide reste actif.

## Systèmes et topics

| Système | Rôle |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | Générer le courant du monde et les données par profondeur |
| Plugin de courant du modèle | Interpoler le courant à la position du robot |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | Publier les données ROS et fournir les services de réglage |

| Topic de l'exemple | Transport et type |
| --- | --- |
| `/ocean_current` | Gazebo, `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo, données par couche |
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS, `posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS, `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS, `posim_interfaces/msg/StratifiedCurrentDatabase` |

Vérifiez les noms dans le SDF et reliez l'hydrodynamique à la source de courant voulue.

## Régler le courant avec ROS

Dans B, listez les services et demandez un courant horizontal de 0.3 m/s :

{{CODE:currents:5}}

La vitesse est en m/s et les angles en radians. Vérifiez `success` et les données publiées. Pour examiner l'interface d'une couche :

{{CODE:currents:6}}

Lisez le modèle, modifiez ses bornes, puis réglez la première couche :

{{CODE:currents:7}}

| Service sous `/hydrodynamics` | Interface | Champs |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | Requête vide |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

### Contraintes des requêtes

- Une modification est appliquée entièrement ou renvoie `success: false` sans changer le courant.
- Les valeurs doivent être finies et respecter les bornes configurées. Les bornes de vitesse sont positives ou nulles ; les angles restent dans ±π horizontalement et ±π/2 verticalement.
- Les réglages concernent le modèle global et toutes les couches, en conservant leurs moyennes. Les nouvelles bornes doivent aussi contenir ces moyennes.
- `layer` commence à 0. Le champ ROS `noise` correspond à `noiseAmp` dans le SDF.

## Fournir des données de marée

Configurez `<tidal_oscillation>` avec un CSV ou des composantes harmoniques, par exemple M2/S2/N2, les directions du flot et du jusant et une date de départ GMT. Dans les exemples harmoniques, l'amplitude est en mètres, la phase en degrés et la vitesse angulaire en degrés/heure. Ces unités diffèrent des radians des services de courant.

En mode marée, le signal harmonique calculé, ou la vitesse du CSV divisée par 100, est multiplié par la première colonne du profil de profondeur. Les amplitudes de hauteur de l'exemple servent de facteurs numériques ; le modèle ne convertit pas la hauteur de marée en vitesse du courant. Calibrez conjointement le signal et le profil pour obtenir la vitesse voulue.

Le CSV comporte une ligne d'en-tête, puis des lignes `YYYY-MM-DD HH:MM,event,speed` :

- Événements : `flood`, `ebb` ou `slack`. La vitesse doit être finie et exprimée en cm/s ; `-` est accepté pour les lignes `slack` ignorées.
- Les dates doivent être valides et strictement croissantes.
- Le lecteur conserve les pics positifs et négatifs alternés, avec au moins deux pics. Hors de l'intervalle, il conserve le premier ou le dernier.
- Un fichier absent, des réglages incomplets ou des données mal formées désactivent le plugin de courant du monde.

Ces données sont des entrées d'expérience. Conservez leurs unités, la plage d'interpolation, l'origine du monde et la référence temporelle. Comparez le courant et la trajectoire du véhicule à partir de la même pose initiale.

## Écoulement et réponse du véhicule

{{MEDIA:currents}}

Images et vidéos : POSIM Notion Wiki. Consultez les [références et licences](citation.md) pour l'attribution de DAVE. Utilisez les commandes et les topics de cette page.
