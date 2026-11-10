# Sonar multifaisceaux

Le sonar multifaisceaux de POSIM utilise un modèle de diffuseurs ponctuels pour produire des données d'intensité en fonction de la distance et des images acoustiques. Le même plugin propose le calcul sur CPU, WGPU ou CUDA. WGPU s'appuie sur Vulkan sous Linux ou Metal dans un environnement macOS natif pour utiliser des GPU de différents fabricants. Consultez la [configuration du calcul](sonar-tuning.md).

## Lancer un exemple {#run-a-sonar-example}

Préparez les [terminaux A et B](terminals.md) et [compilez les paquets sonar](sonar-tuning.md#build-the-sonar-packages) s'ils ne sont pas installés. Lancez la simulation dans A et consultez les données dans B. La première commande choisit automatiquement le moteur de calcul ; l'exemple dédié choisit WGPU et ouvre aussi RViz.

{{CODE:sonar:0}}

Le paquet `posim_multibeam_sonar_demo` propose aussi un exemple autonome :

{{CODE:sonar:1}}

Lancez un seul exemple à la fois. Remplacez `wgpu` par `cpu`, `cuda` ou `auto` pour changer de moteur de calcul. Pour un sonar embarqué, sélectionnez `bluerov2_heavy_multibeam_sonar` avec un monde adapté.

## Fonctionnement du capteur

Gazebo fournit la géométrie de la scène. Le modèle estime les normales de surface, calcule la rétrodiffusion acoustique, applique la réponse de réception et comprime les échos dans les cellules de distance. Les données acoustiques et l'image représentent la même observation. Utilisez les données brutes pour l'analyse et conservez les réglages de gain et d'affichage.

Une valeur positive de `spec/receiveBeamWidth` active le mode physique, qui utilise les coordonnées XYZ mesurées par une caméra de profondeur couvrant tout le champ du sonar. Les échantillons géométriques, les directions de réception et les sorties en distance sont indépendants. Les cellules dont le centre se trouve dans le champ se répartissent son angle solide complet ; leurs surfaces projetées et la réponse de réception signée déterminent les contributions aux faisceaux. Le modèle utilise une impulsion de Hamming en bande de base, de bande passante fixe et de crête unitaire. L'intervalle de calcul interne dépasse l'intervalle de distance publié. Le pas de sortie vaut `soundSpeed / (2 × bandwidth)` ; la longueur de calcul ne détermine pas le gain de l'impulsion. L'intervalle de garde interne n'est pas publié et ne modifie pas ce pas.

`ray/scan/horizontal/samples` définit les colonnes géométriques, `ray/scan/vertical/rays` les lignes de l'image et `beams` les directions de réception. Le mode physique exige un champ symétrique inférieur à 180° sur chaque axe. Les limites de distance sont radiales ; les pixels hors champ ou hors limites ont des XYZ invalides dans le nuage organisé. `spec/receiveBeamWidth` indique la largeur totale à mi-puissance en radians ; `spec/receiveGain` règle le gain linéaire de pression (1 par défaut). Une largeur absente ou nulle sélectionne le mode de grille par défaut. Le mode physique prend en charge CPU et WGPU, mais pas CUDA. Il n'y a pas de plafond fixe de 4096 échantillons de fréquence ; les grandes grilles restent limitées par les ressources du moteur et les dimensions représentables. Les coefficients de diffusion sont générés par image et cellule géométrique. Le modèle décrit des échos en bande de base, sans propagation de phase porteuse ni étalonnage de l'amplitude du matériel.

## Examiner les sorties

Dans le terminal B :

{{CODE:sonar:2}}

- Les images utilisent une QoS `reliable`. Choisissez aussi cette fiabilité dans l'affichage Image de RViz.
- Ajoutez PointCloud2 pour `/sensor/multibeam_sonar/point_cloud`.
- Les échos bruts utilisent `marine_acoustic_msgs/msg/ProjectedSonarImage`.
- Le lancement BlueView transmet l'horloge de Gazebo à ROS `/clock`. Pour les nœuds avec `use_sim_time`, vérifiez que la simulation tourne et que l'horloge avance.

## Configuration du capteur {#sensor-configuration}

Placez ces systèmes directement dans `<world>` :

{{CODE:sonar:3}}

Placez le capteur P900 dans un `<link>` du modèle :

{{CODE:sonar:4}}

L'exemple demande 512 faisceaux horizontaux sur environ 130°, 300 rayons verticaux sur environ 12° et une portée de 0.1 à 10 m. Il utilise une fréquence de 900 kHz, une bande passante de 29.9 kHz et une célérité du son de 1500 m/s. Les 30 Hz indiqués sont une consigne ; la cadence reçue dépend du calcul et du rendu. Consultez le [modèle complet](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf).

## Paramètres de `<spec>`

| Paramètre | Rôle | Valeur par défaut | Exemple P900 |
| --- | --- | --- | --- |
| `verticalFOV` | Ouverture acoustique verticale, degrés | 10 | 12 |
| `sonarFreq` | Fréquence acoustique, Hz | 900000 | 900000 |
| `bandwidth` | Bande passante, Hz | 29500000 | 29900 |
| `soundSpeed` | Célérité du son, m/s | 1500 | 1500 |
| `sourceLevel` | Niveau de la source, dB | 220 | 220 |
| `maxDistance` | Portée du traitement, m ; à accorder à celle des rayons | 60 | 10 |
| `raySkips` | Pas d'échantillonnage vertical ; entier positif | 10 | 10 |
| `sensorGain` | Gain d’amplitude des échos dans la grille CPU par défaut ; inutilisé en mode physique et par WGPU/CUDA | 0.02 | 0.02 |
| `blazingSonarImage` | Graine aléatoire issue de l'horloge réelle ; `false` fixe la graine à 1234 | false | true |
| `pointCloudTopicName` | Suffixe du topic Gazebo du nuage de points | `point_cloud` | Identique |
| `sonarImageRawTopicName` | Suffixe du topic ROS des échos bruts | `sonar_image_raw` | Identique |
| `sonarImageTopicName` | Suffixe du topic ROS de l'image | `sonar_image` | Identique |
| `frameName` | Identifiant du repère optique | `forward_sonar_optical_link` | Identique |
| `writeLog` | Écrire les CSV dans un répertoire propre à l'exécution | false | false |
| `writeFrameInterval` | Nombre d'images entre deux écritures ; entier positif | 10 | 5 |
| `debugFlag` | Messages de diagnostic dans la console | false | false |
| `plotScaler` | Lu et affiché, mais sans effet sur la génération de l'image | 10 | Non renseigné |

### Topics et repères

Les suffixes s'ajoutent au `<topic>` du capteur, ici `/sensor/multibeam_sonar`. Le nuage de points Gazebo est de type `gz.msgs.PointCloudPacked` ; le pont ROS fourni le transmet en `sensor_msgs/msg/PointCloud2`. Faites correspondre `pointCloudTopicName` à `sensor_config.py` et renseignez le repère optique dans `frameName`.

### Géométrie des rayons

Par défaut, `<ray>` définit 256 faisceaux horizontaux, 3 rayons verticaux, des angles horizontaux de ±π/4, verticaux de ±π/8 et une portée de 0.1 à 5 m. Les angles sont en radians, sauf avec `degrees="true"`. Le `<reference_frame>` facultatif dans `<gz:multibeam_sonar>` utilise une pose nulle par défaut.

Conservez les blocs `<ray>` et `<spec>` complets : les valeurs par défaut concernent leurs champs internes. Accordez la portée des rayons et `maxDistance`. L'[implémentation](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc) précise la lecture des paramètres et les publications.

### Comparer des essais

Désactivez l'enregistrement pour mesurer le débit, sauf s'il fait partie de l'expérience. Consultez le guide [Configurer et régler le sonar](sonar-tuning.md). Conservez les paramètres acoustiques et la valeur de `blazingSonarImage` pour comparer les essais.

### Moteur de calcul {#compute-backend}

| Choix | Comportement |
| --- | --- |
| `auto` | Essaie CUDA, puis WGPU, puis CPU ; choix par défaut |
| `wgpu` | Utilise un adaptateur Vulkan/Metal compatible ; son absence empêche l'initialisation du capteur |
| `cuda` | Requiert un GPU NVIDIA et le backend CUDA compilé et installé ; échoue s'ils sont indisponibles |
| `cpu` | Calcule les échos sur le CPU ; le rendu Gazebo reste nécessaire |

Le lancement dédié accepte `compute_backend`. Les autres lancements lisent `POSIM_SONAR_COMPUTE_BACKEND`. Vérifiez la ligne `Using sonar compute backend:` au démarrage. Le guide [Configurer et régler le sonar](sonar-tuning.md) détaille le choix de l'adaptateur, les prérequis et les différences numériques.

## Ajouter un capteur

1. Copiez `models/posim_sensor_models/description/blueview_p900/` sous un nouvel identifiant.
2. Préparez le `config/<identifiant>/sensor_config.py` correspondant. Accordez les noms de modèle, de liens, de repères et les topics du pont.
3. Réglez les échantillons, les angles et la portée. Chargez dans le monde les deux systèmes présentés plus haut ; consultez le [SDF du monde sonar](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world).
4. Préparez le moteur de calcul et le rendu, recompilez l'espace de travail et chargez son environnement. Sélectionnez le capteur avec les [arguments de lancement](quickstart.md).

## Enregistrer et tracer les données brutes

Avec `writeLog=true`, chaque capteur crée `SonarRawData_<run>_<suffix>/` dans le répertoire de travail du simulateur et en affiche le chemin. Il y place des CSV numérotés et `SonarRawData_beam_angles.csv`, sans écraser les essais précédents. Un intervalle inférieur à 1 est ramené à 1. Si le répertoire ne peut pas être créé, l'enregistrement est désactivé et une erreur est affichée.

Dans le répertoire indiqué, ouvrez la première image :

{{CODE:sonar:6}}

Pour enregistrer les figures en PNG :

{{CODE:sonar:7}}

L'[outil de tracé](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py) s'installe aussi sans CUDA. Il produit une vue sonar et cinq profils de faisceaux régulièrement espacés, ou tous les profils s'il y en a moins de cinq.

| Option | Effet |
| --- | --- |
| `--range-max` | Distance maximale affichée, m |
| `--beam-step` | Afficher un faisceau sur N |
| `--dynamic-range` | Plage sous le maximum, dB ; valeur par défaut : 60 |

Les couleurs sont relatives au maximum de chaque image. Comparez les valeurs CSV pour confronter plusieurs essais. La première colonne contient la distance ; les suivantes, les échos complexes de chaque faisceau. Gardez le fichier d'angles avec les données et surveillez l'espace disque.

## Scènes et observations

{{MEDIA:sonar}}

Images et vidéos : POSIM Notion Wiki. Les [références et licences](citation.md) donnent les attributions de DAVE et du modèle sonar. Utilisez les commandes et les noms de topics des blocs de cette page.
