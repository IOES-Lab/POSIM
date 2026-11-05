# Fonctionnalités et exemples

POSIM fournit des systèmes Gazebo, des modèles de robots et des interfaces ROS. Le SDF du monde et les fichiers de lancement sélectionnent les systèmes de chaque simulation.

## Environnement et rendu

| Fonction | Implémentation | Guide |
|---|---|---|
| Brume sous-marine, absorption RVB et atténuation solaire | `gazebo/posim_gz_rendering` | [Effets sous-marins](underwater-rendering.md) |
| Caustiques du fond et particules en suspension | `gazebo/posim_gz_rendering` | [Activation des effets](underwater-rendering.md) |
| Texture du fond, grille et courbes de profondeur | `gazebo/posim_gz_rendering` ; cartes du terrain | [Matériaux et espacement](underwater-rendering.md) |
| Houle spectrale et hydrodynamique du WAM-V | Wave Sim externe ; intégration `extras/surface` | [Véhicules de surface](surface.md) |
| Courants uniformes, selon la profondeur et de marée | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [Courants](currents.md) |
| Origine géographique et conversion des coordonnées | `posim_ros_gz_plugins` | [Coordonnées sphériques](coordinates.md) |
| Fonds, objets et scènes de travail | `models/posim_worlds`, `posim_object_models` | [Mondes](worlds.md), [objets](objects.md), [terrains](heightmaps.md) |

Les effets optiques sous-marins s'appliquent aux surfaces PBR et préservent les valeurs numériques de profondeur. La surface océanique possède son propre matériau. La neige marine est une approximation visuelle procédurale, désactivée par défaut. Le [guide du rendu](underwater-rendering.md) précise l'hypothèse de niveau marin et le comportement des caméras.

## Capteurs, robots et commande

| Fonction | Implémentation | Guide |
|---|---|---|
| GNSS marin, perte en immersion, réacquisition et latence | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| Atténuation des images sous-marines à partir de RGBD | `posim_gz_sensor_plugins/UnderwaterCamera` | [Caméra](camera.md) |
| Mesures DVL et conversion ROS | DVL Gazebo ; `posim_ros_gz_plugins` | [DVL](dvl.md) |
| Pression hydrostatique et profondeur calculée | `posim_gz_sensor_plugins` | [Pression](pressure.md) |
| Interrogation acoustique et positionnement USBL | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| Sonar multifaisceaux CUDA | `gazebo/posim_gz_multibeam_sonar` | [Sonar](sonar.md), [configuration et performances](sonar-tuning.md) |
| REXROV, variantes BlueROV2 et planeur Slocum | `models/posim_robot_models` | [ROV](rovs.md), [planeur](gliders.md), [ajout d'un robot](custom-robots.md) |
| WAM-V avec ArduRover et commande ROS | `extras/surface`, configurations de lancement, `posim_interfaces` | [Véhicule de surface](surface.md), [ROS 2](ros.md) |

Les capteurs disponibles dépendent des plugins installés et du modèle. Les profils GNSS décrivent les montages : ajoutez le lien mécanique et le récepteur lors de l'intégration. Le sonar CUDA nécessite un GPU et des outils compatibles. Utilisez le démarrage rapide de chaque référence pour vérifier les mesures en cours de simulation.

## Répertoires du code source

Ces chemins sont relatifs au dépôt POSIM.

| Répertoire | Contenu |
|---|---|
| `models/posim_worlds` | Mondes et ressources de l'environnement |
| `models/posim_robot_models` | Mécanique, capteurs et configuration ROS de chaque robot |
| `models/posim_sensor_models` | Descriptions et exemples de capteurs autonomes |
| `gazebo/` | Plugins de l'environnement marin et des capteurs |
| `posim_interfaces` | Définitions des messages et services POSIM |
| `examples/posim_demos` | Lancements reliant mondes, modèles et ROS |

[Première simulation](quickstart.md) lance ces composants ensemble. Pour les adapter, consultez [Ajouter un robot](custom-robots.md) et [Créer un monde](custom-worlds.md).

## Bibliothèque Ogre de remplacement pour macOS

L'[outil de compilation Ogre](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) produit un cœur Ogre isolé pour macOS arm64 avec Ogre-Next 2.3.3. Suivez son profil SDK pris en charge et ses vérifications des bibliothèques chargées.

Consultez les [licences des composants](citation.md).
