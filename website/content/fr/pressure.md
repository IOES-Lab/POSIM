# Pression hydrostatique

Le plugin calcule la pression à partir de la profondeur du modèle et publie des messages ROS `FluidPressure`. Il peut aussi publier une profondeur estimée à partir de cette pression.

## Observer REXROV

Lancez le robot dans le [terminal A préparé pour votre installation](terminals.md) :

{{CODE:pressure:0}}

Dans le terminal B préparé :

{{CODE:pressure:1}}

La pression est en **Pa**, la variance en **Pa²** et la profondeur en **m**.

## Intégrer le plugin

Placez le bloc directement dans le `<model>` à mesurer. `namespace` identifie ses topics :

{{CODE:pressure:2}}

La sortie est `/model/<namespace>/<topic>` ; celle de profondeur ajoute `_depth`. Le plugin mesure le modèle auquel il est attaché. Le namespace peut différer du nom du modèle.

## Paramètres

| Paramètre | Signification et valeur par défaut |
|---|---|
| `namespace` | Obligatoire ; préfixe de topic ROS valide |
| `topic` | Nom de sortie ; `sea_pressure` |
| `standard_pressure` | Pression de surface finie et positive ou nulle, kPa ; `101.325` |
| `kPa_per_meter` | Augmentation hydrostatique finie et strictement positive, kPa/m ; `9.80638` |
| `estimate_depth_on` | Publier la profondeur calculée ; `true` |
| `update_rate` | Fréquence finie, Hz ; `0`. Une valeur non positive publie à chaque avancée physique |
| `saturation` | Valeur acceptée, `3000` ; cette implémentation ne borne pas la pression avec elle |

La profondeur vaut `max(0, -z)` dans le repère du monde, transformations parentes des modèles imbriqués comprises. La référence est le plan z = 0, indépendamment des vagues. Le calcul est en kPa, puis la publication convertit en Pa.

## Temps et validité

Pression ROS, profondeur et pression Gazebo partagent le même horodatage simulé. Une pause ou un horodatage répété ne produit pas de nouvelle mesure. La publication reprend lorsque le temps avance ou revient en arrière.

Une configuration invalide laisse le plugin inactif. Aucune mesure n'est publiée si la pose manque, n'est pas finie ou provoque un dépassement numérique.

La variance publiée est fixe : **9 000 000 Pa²**. Aucun bruit aléatoire n'est ajouté à la pression. Tenez-en compte dans l'estimateur. Le [code du plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc) détaille les calculs.

## Comparer deux profondeurs

Mesurez deux positions z connues sans changer la pente de pression. La différence doit être l'augmentation par mètre multipliée par le changement de profondeur. La profondeur dérivée utilise le même calcul ; elle n'est donc pas une mesure indépendante. Accordez unités et pression de référence de votre estimateur.

## Sorties de pression et profondeur

{{MEDIA:pressure}}

Images : wiki Notion POSIM ; attribution DAVE dans [Citations et licences](citation.md). Utilisez les commandes et topics des blocs de cette page.
