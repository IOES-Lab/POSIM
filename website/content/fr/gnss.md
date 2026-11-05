# GNSS maritime

`posim_gz_gnss` simule un récepteur satellitaire sur le lien d'antenne du modèle. Il convertit sa position dans le monde en coordonnées géographiques, ajoute des erreurs gaussiennes réglables et publie position, vitesse, horodatage et état dans un message unique. Il représente la perte de réception sous l'eau, le blocage explicite, la réacquisition et la latence.

## Compiler et monter le récepteur

Dans le [terminal A préparé pour votre installation](terminals.md) ; sur macOS, utilisez le terminal Linux du bureau RDP :

{{CODE:gnss:0}}

Le modèle doit avoir un lien physique d'antenne, généralement `gnss_link`, placé et fixé au véhicule. Le monde doit définir des [coordonnées WGS 84](coordinates.md). Ajoutez au SDF du modèle :

{{CODE:gnss:1}}

Adaptez lien et topic au robot. Le guide [Créer un robot](custom-robots.md) explique comment préparer et lancer le modèle.

Les [profils](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json) donnent des poses par rapport à `base_link` : WAM-V à z = 1.8 m, BlueROV2 à 0.3 m et REXROV à 0.8 m. WAM-V active le récepteur ; les profils submersibles le désactivent au départ. Ces profils sont des données de montage : incorporez lien et plugin au modèle. WAM-V exige aussi le [modèle externe Wave Sim](surface.md).

## Paramètres

| Paramètre SDF | Unité ou signification | Défaut |
|---|---|---|
| `update_rate` | Échantillons par seconde simulée ; >0 à 100 Hz | 5 |
| `latency` | Retard de livraison, secondes simulées ; 0–10 s | 0.2 |
| `reacquire_seconds` | Temps continu d'acquisition hors de l'eau, s | 3 |
| `dry_margin` | Dégagement au-dessus de l'eau pour acquérir, m | 0.05 |
| `horizontal_stddev` | Écart-type de position horizontale, m | 0.7 |
| `vertical_stddev` | Écart-type de position verticale, m | 1.2 |
| `velocity_stddev` | Écart-type de vitesse, m/s | 0.1 |
| `water_level` | z moyen de l'eau sans topic de vagues, m | 0 |
| `seed` | Graine aléatoire du récepteur | 42 |
| `enabled` | Récepteur actif | true |
| `link_name` | Lien d'antenne dans le modèle | `gnss_link` |
| `topic` | Topic Gazebo de mesure | `/model/<model-name>/gnss` |
| `wave_topic` | Topic facultatif de hauteur d'eau | Vide |

Les réglages numériques doivent être finis. Latence, temps d'acquisition, marge et écarts-types doivent être positifs ou nuls.

## Hauteur locale de l'eau

Sans `wave_topic`, le récepteur utilise le plan moyen de l'eau. Avec un topic configuré, il lui faut des hauteurs locales récentes ; des données absentes ou invalides produisent `surface unavailable`.

Le message `gz.msgs.Double_V` contient, dans l'ordre : schéma `1`, taille de grille, dimensions périodiques X/Y, vitesse et angle du vent, cambrure des vagues, temps simulé et hauteurs rangées par lignes.

- Taille de grille : entier de 2 à 256, puis son carré en valeurs de hauteur.
- Dimensions X/Y : finies et strictement positives.
- Horodatage : fini, âgé d'au plus **0.6 seconde simulée**, jamais futur.

Configurez séparément un publieur compatible. Les métadonnées du vent ne pilotent pas la réception.

## Lire les mesures et le blocage

Après avoir lancé le modèle configuré, lisez son topic dans le terminal B préparé :

{{CODE:gnss:2}}

Chaque message contient 15 nombres en double précision. Les indices commencent à zéro. Conservez l'horodatage d'acquisition même si la solution arrive avec retard.

| Indice | Champ et unité |
|---|---|
| 0–3 | Schéma `1`, époque de réinitialisation, acquisition en secondes simulées, état |
| 4–6 | Latitude en degrés, longitude en degrés, altitude en m |
| 7–9 | Vitesses est, nord et haut, m/s |
| 10–12 | Variances de position horizontale/verticale en m² et de vitesse en (m/s)² |
| 13–14 | z de l'antenne dans le monde et hauteur locale de l'eau, m |

États : `0 disabled` (désactivé), `1 submerged` (immergé), `2 acquiring` (acquisition), `3 fix` (solution valide), `4 blocked` (bloqué) et `5 surface unavailable` (surface indisponible).

Une mesure invalide contient des coordonnées et vitesses `NaN`. Le contact avec l'eau ou le blocage supprime les solutions en attente. La réacquisition demande un dégagement continu au-dessus de la marge. Une réinitialisation efface temporisation et mesures retardées, puis incrémente l'époque.

Pour bloquer l'antenne ; utilisez `false` pour la libérer :

{{CODE:gnss:3}}

## Interpréter et vérifier

Comparez une antenne fixe hors de l'eau, immergée, bloquée, puis rétablie. Vérifiez acquisition, retard, époque après réinitialisation et horodatages croissants. Le programme qui reçoit les données doit rejeter les solutions périmées ou futures et gérer les états invalides. La sortie utilise Gazebo ; pour produire ROS `NavSatFix` ou alimenter un autopilote ou un estimateur de navigation, ajoutez un adaptateur au format attendu.

Ce modèle repose sur la pose de l'antenne. Il ne calcule ni géométrie satellitaire, ni propagation RF, ni trajets multiples physiques, ni positionnement acoustique. La précision configurée est une donnée de simulation, pas une garantie fabricant. Consultez l'[implémentation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc) et le [guide du paquet](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss).
