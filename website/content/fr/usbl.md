# Positionnement USBL

Le positionnement acoustique à base ultracourte (USBL) localise des équipements sous l'eau. Cet exemple relie un émetteur-récepteur à deux transpondeurs. Vous pouvez interroger tous les transpondeurs ou en choisir un par ID.

## Lancer le monde d'exemple

Dans le [terminal A préparé pour votre installation](terminals.md) :

{{CODE:usbl:0}}

Le monde contient déjà les trois appareils. Dans le terminal B préparé, recevez leurs positions :

{{CODE:usbl:1}}

Les deux messages portent `transponder_id`, mais leurs champs n'ont pas le même sens :

| Sortie | x | y | z |
|---|---|---|---|
| `transponder_location` | Angle horizontal, degrés | Distance, m | Élévation, degrés |
| `transponder_location_cartesian` | x relatif, m | y relatif, m | z relatif, m |

Le calcul utilise la différence de positions dans le monde. L'angle horizontal vaut 0° vers +x et 90° vers +y ; l'élévation positive pointe vers +z. Tenez compte de ces axes avec des données exprimées dans le repère du véhicule.

## Intégration SDF

Placez l'émetteur-récepteur dans le modèle `box`. Chaque ID correspond au modèle à la même position dans la liste `transponder_attached_object` :

{{CODE:usbl:2}}

Placez ce transpondeur dans `sphere`, puis un autre dans `sphere2` avec `transponder_ID` égal à `2`. Nom, ID et namespace de l'émetteur-récepteur doivent correspondre aux deux extrémités :

{{CODE:usbl:3}}

## Paramètres

| Paramètre | Appareil | Signification et défaut |
|---|---|---|
| `namespace` | Les deux | Namespace obligatoire, par exemple `USBL` |
| `transceiver_device`, `transceiver_ID` | Les deux | Nom et ID obligatoires pour les chemins |
| `transponder_device`, `transponder_ID` | Les deux | Nom et ID obligatoires ; l'émetteur-récepteur accepte plusieurs IDs séparés par des virgules |
| `transponder_attached_object` | Émetteur-récepteur | Modèles obligatoires séparés par des virgules, dans l'ordre des IDs |
| `transceiver_model` | Transpondeur | Modèle obligatoire de l'émetteur-récepteur pour sa position |
| `sound_speed` | Les deux | Vitesse de référence à 10 °C et z = 0, m/s ; `1540`, finie et positive |
| `enable_ping_scheduler` | Émetteur-récepteur | Booléen obligatoire ; `false` utilise les pings manuels |
| `ping_frequency` | Émetteur-récepteur | Fréquence en temps simulé, Hz ; `1`, exemple `0.5` ; finie et positive |
| `interrogation_mode` | Émetteur-récepteur | `common` ou `individual` ; défaut `common` |
| `mu`, `sigma` | Transpondeur | Moyenne et écart-type du bruit de position, m ; `0` et `1`, exemple `0` et `0` ; finis et sigma ≥ 0 |

Utilisez des modèles existants, des IDs uniques et des listes IDs/modèles de même longueur. Les IDs sont des entiers décimaux de **0 à 2147483647**, sans signe ni zéro initial. Chaque plugin exige un modèle contenant un lien. Le [SDF complet](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) fournit géométrie et placement.

Une configuration invalide laisse le plugin inactif. Les canaux ou cibles de commande inconnus sont ignorés ; un canal inconnu conserve la sélection. Le premier transpondeur est le canal initial. Les pings programmés suivent les pauses et sont émis au plus une fois par mise à jour.

## Temps de propagation

Chaque interrogation comporte deux trajets : arrivée de la demande au transpondeur, puis retour de la position à l'émetteur-récepteur. Chaque retard est la distance divisée par la vitesse du son de l'appareil concerné.

- Les délais fractionnaires suivent le temps simulé, s'arrêtent avec le monde et sont annulés lors d'un retour en arrière du temps.
- Un seul échantillon par transpondeur peut attendre sur chaque trajet ; demandes ou réponses supplémentaires sont ignorées.
- Choisissez un intervalle entre pings supérieur au temps aller-retour attendu pour recevoir chaque interrogation.

La vitesse effective vaut `sound_speed + 0.017 × depth + 4 × (temperature − 10)`. Les unités sont m/s, profondeur en m sous z = 0 et température en °C. Au-dessus de l'eau, la profondeur vaut zéro. La température par défaut est 10 °C.

Publiez `std_msgs/msg/Float64` sur `/USBL/transceiver_manufacturer_168/temperature` ou `/USBL/transponder_manufacturer_1/temperature`. La modification s'applique aux trajets suivants. Les températures non finies et les changements donnant une vitesse non positive ou non finie sont ignorés.

Cette approximation suit la [présentation DOSITS de la vitesse du son](https://dosits.org/decision-makers/tutorials/science/sound-speed/). Les distances sont rectilignes, sans réfraction, salinité, trajets multiples ni interférences. Le retour utilise la position reçue, avec le bruit configuré.

## Interroger tous les transpondeurs

{{CODE:usbl:4}}

Le mode `common` diffuse une demande par ping programmé et peut renvoyer plusieurs IDs sur le même topic. Conservez l'ID de chaque réponse. L'émetteur-récepteur conserve le mode pour les transpondeurs qui se connectent plus tard.

## Choisir un transpondeur

{{CODE:usbl:5}}

Changer de canal sélectionne le mode `individual` ; chaque ping programmé vise cet ID. Revenez à `common` pour interroger tous les appareils.

## Disposition et modes d'interrogation

{{MEDIA:usbl}}

Images : wiki Notion POSIM ; attribution DAVE dans [Citations et licences](citation.md). Utilisez les commandes et topics de cette page.
