# Coordonnées sphériques

Le plugin ROS `SphericalCoords` gère l'origine géographique du monde et convertit les positions cartésiennes locales en latitude, longitude et altitude, et inversement.

## Lancer un monde configuré

Dans le [terminal A préparé pour votre installation](terminals.md) :

{{CODE:coordinates:0}}

Dans le terminal B préparé :

{{CODE:coordinates:1}}

Le SDF d'exemple fixe la latitude à **35.074823°** et la longitude à **129.084798°**. Lisez la réponse pour connaître l'origine du monde en cours.

## Intégration SDF et paramètres

Placez ces deux blocs directement dans `<world>` :

{{CODE:coordinates:2}}

Le plugin lit les coordonnées du monde et ne possède pas de paramètre SDF propre. Ses quatre services ont des chemins fixes sous `/gz/`. Placé ailleurs que dans un monde, ou sans configuration géographique, il signale une erreur et laisse les services désactivés.

| Champ du monde | Signification | Exemple |
|---|---|---|
| `surface_model` | Ellipsoïde de référence | `EARTH_WGS84` |
| `world_frame_orientation` | Convention des axes | `ENU` |
| `latitude_deg`, `longitude_deg` | Origine, degrés | `35.074823`, `129.084798` |
| `elevation` | Altitude de l'origine, m | `0` |
| `heading_deg` | Décalage de cap, degrés | `0` |

Définissez une origine explicite. Consultez le [plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc) et le [monde d'exemple](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world).

## Services

Les types appartiennent à `posim_interfaces/srv`. Les angles sont en degrés ; les coordonnées locales et l'altitude sont en mètres.

| Suffixe du service sous `/gz/` | Type | Requête |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Vide |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Les valeurs doivent être finies. La latitude va de −90° à 90°, la longitude de −180° à 180°. Une mise à jour d'origine invalide renvoie `success: false` sans changer l'origine. Une transformation échouée renvoie des coordonnées `NaN` ; vérifiez-les avant de les transmettre à un contrôleur.

## Convertir une position locale

{{CODE:coordinates:3}}

La réponse contient latitude, longitude et altitude. Passez ces valeurs au service inverse et comparez `output` au vecteur initial pour vérifier origine et convention des axes.

## Changer l'origine

{{CODE:coordinates:4}}

Le changement modifie la correspondance entre positions locales et géographiques. Vérifiez la cohérence entre le terrain, l'orientation du monde, la référence d'altitude et les contrôleurs. Les conversions utilisent Gazebo ; tenez compte du cap et de l'orientation définis dans le SDF.
