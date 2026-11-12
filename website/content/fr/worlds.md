# Bibliothèque de mondes

Les mondes se trouvent dans `models/posim_worlds/worlds`. Sélectionnez-les par leur nom de fichier, sans l'extension `.world`.

Les mondes océaniques incluent `model://calm_waves` : vagues FFT de Wave Sim avec un vent d’entrée de 3 m/s, une tuile périodique de 256 m et une zone visible de 768 × 768 m. La surface est visible du dessus et du dessous. Les effets sous-marins natifs s’appliquent aux images RGB ; le tutoriel en eau peu profonde les active aussi dans la vue du monde Gazebo. Les installations standard et les images incluent Wave Sim. La flottabilité des véhicules sous-marins, le GNSS marin et la limite optique utilisent le niveau moyen z = 0, indépendamment de la hauteur des vagues affichées. WAM-V utilise l’hydrodynamique Wave Sim couplée aux vagues. La démonstration USBL sans eau et les mondes de test sonar isolés conservent leur propre géométrie.

## Lancer un monde

Utilisez le [terminal POSIM de votre installation](terminals.md), sous Ubuntu natif, WSL, Docker ou dans le bureau RDP.

{{CODE:worlds:0}}

Le nom du fichier peut différer du nom interne : `posim_ocean_waves.world` déclare `oceans_waves`. Ce dernier apparaît dans les topics et services Gazebo. Vérifiez `<world name="…">` avant de construire un chemin.

## Choisir selon l'expérience

| Expérience | Monde |
|---|---|
| Premier véhicule | `posim_ocean_waves` |
| Effets de caméra sous-marine | `camera_tutorial` |
| Mesures DVL | `dvl_world` |
| Interrogation USBL | `usbl_tutorial` |
| Configuration des courants | `ocean_current_plugin` |
| Scène côtière et géographique | `posim_Santorini` |
| Sonar | `posim_multibeam_sonar` et mondes océaniques dont le nom contient sonar |
| Manipulation | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

Les mondes sonar nécessitent l'[environnement correspondant](sonar-tuning.md). Les références Fuel peuvent télécharger des modèles au premier lancement.

## Fichiers SDF

Ouvrez un fichier pour examiner ses systèmes, modèles, origine, éclairages et caméras.

| SDF du monde |
|---|
{{WORLD_CATALOG}}

## Composer une autre scène

Copiez un SDF adapté dans `models/posim_worlds/worlds` et donnez de nouveaux noms au fichier et au monde. Employez des URI résolvant les ressources installées. Recompilez, rechargez l'environnement, puis lancez le nouveau fichier. Consultez les [terrains par carte de hauteur](heightmaps.md) et les [objets](objects.md).

## Attendre les téléchargements Fuel

`posim_integrated` et `posim_ocean_models` utilisent des modèles distants. Attendez la fin du premier téléchargement et évitez les lancements répétés pendant le chargement. `verbose:=true` affiche les messages liés aux ressources.

Dans un autre terminal du même environnement, demandez la scène chargée :

{{CODE:worlds:1}}

Vérifiez les modèles dans la réponse, l'avancement de l'horloge et les mesures. Si le téléchargement se bloque, examinez le message de chargement et la connexion réseau de l'ordinateur ou du conteneur exécutant Gazebo.

## Consulter les arguments de lancement

{{CODE:worlds:2}}

| Argument | Rôle | Valeur initiale du lancement de capteurs |
|---|---|---|
| `namespace` | Répertoire du modèle installé | Vide ; à préciser |
| `world_name` | Fichier du monde | `empty.sdf` |
| `paused` | Démarrer en pause | `true` |
| `gui` / `headless` | Afficher / masquer l'interface Gazebo | `true` / `false` |
| `use_sim_time` | Utiliser l'horloge simulée dans ROS | `true` |
| `x`, `y`, `z` | Position initiale, m | `0.0` chacun |
| `roll`, `pitch`, `yaw` | Orientation initiale, rad | `0.0` chacun |
| `debug` / `verbosity_level` | Journal détaillé / niveau | `false` / `1` |
| `use_ned_frame` | Publier la transformation `world_ned` (ENU vers NED) | `false` |

Utilisez `paused:=false` pour produire des données de capteurs. Consultez `--show-args` sur le lancement concerné pour les options des robots et des mondes.
