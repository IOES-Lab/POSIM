# Capteur de vitesse Doppler (DVL)

Un DVL estime la vitesse par rapport au fond ou à l'eau environnante. POSIM associe le système DVL de Gazebo à un pont ROS produisant `posim_interfaces/msg/DVL`.

## Exécuter le capteur

Dans le [terminal A préparé pour votre installation](terminals.md) :

{{CODE:dvl:0}}

Dans le terminal B préparé, consultez Gazebo, arrêtez cette lecture avec **Ctrl+C**, puis consultez ROS :

{{CODE:dvl:1}}

Pour un autre capteur, utilisez le topic défini dans son modèle.

## Intégration SDF

Ajoutez ces systèmes directement dans `<world>` :

{{CODE:dvl:2}}

Le pont POSIM se place dans le `<model>` du capteur. Son topic doit correspondre au `<topic>` du DVL situé dans `<link>` :

{{CODE:dvl:3}}

Le [SDF complet du DVL500-300](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf) définit le capteur `type="custom" gz:type="dvl"`, quatre faisceaux et les modes de suivi. Le [monde](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world) fournit la géométrie du fond.

## Paramètres et unités

`DVLBridge` accepte un seul paramètre : `topic`, par défaut `/dvl/velocity`. Il reçoit les messages Gazebo et publie les messages ROS sur ce chemin. Les autres champs appartiennent au capteur Gazebo ; le tableau reprend l'exemple DVL500-300.

| Champ SDF du capteur | Signification | Exemple |
|---|---|---|
| `topic`, `update_rate` | Chemin et fréquence demandée, Hz | `/dvl/velocity`, 8 |
| `gz:dvl/type` | Disposition des transducteurs | `phased_array` |
| `arrangement` : `aperture`, `rotation`, `tilt` | Angles des faisceaux ; `degrees="true"` utilise les degrés | Ouverture 3°, rotations −135/135/45/−45°, inclinaison 25° |
| `minimum_range`, `maximum_range` | Portée acoustique, m | 0.3, 200 |
| `reference_frame` | Position en m, orientation en rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | Sélection de la référence de mesure | Tous deux `best` |
| Suivi : `noise/stddev` | Écart-type du bruit de vitesse, m/s | Fond 0.005 ; eau 0.015 |
| Masse d'eau : `water_velocity` | Composantes x/y/z, m/s | 0/0/0 |
| Masse d'eau : `boundaries/near`, `boundaries/far`, `bins` | Bornes en m et nombre de cellules | 10, 100, 10 |
| Suivi : `visualize` | Afficher la géométrie d'échantillonnage | true |

L'[implémentation du pont](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc) décrit la conversion. Le capteur de profondeur utilise le rendu Gazebo : terminez la [configuration de l'environnement](requirements.md).

## Modèles disponibles

| Description | Famille |
|---|---|
| `nortek_dvl500_300`, `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300`, `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000`, `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL avec [sonar CUDA](sonar-tuning.md) |

Vérifiez l'échantillonnage et la géométrie dans chaque SDF.

## Repère et covariance

L'exemple convertit ENU vers son repère avant/tribord/bas via `<reference_frame>`. Le pont conserve le nom de repère Gazebo en remplaçant `::` par `/`. Utilisez le même repère dans l'estimateur et distinguez les vitesses par rapport au fond et à l'eau.

La vitesse globale et chaque faisceau portent une covariance ROS de mouvement 6×6. Le bloc supérieur gauche 3×3 contient la covariance de vitesse linéaire, en m²/s². Les autres cases sont nulles parce que le DVL n'estime pas la vitesse angulaire ; elles ne représentent pas une incertitude mesurée nulle.

## Examiner les observations

Consultez `ros2 interface show posim_interfaces/msg/DVL`. Vérifiez vitesse, repère et état à l'arrêt, puis pendant un mouvement contrôlé. Fond, portée et intersections des faisceaux influencent la mesure. Activez `visualize` dans le modèle pour inspecter les faisceaux.

## Faisceaux et sortie DVL

{{MEDIA:dvl}}

Images : wiki Notion POSIM ; attribution DAVE dans [Citations et licences](citation.md). Utilisez les commandes et topics de cette page.
