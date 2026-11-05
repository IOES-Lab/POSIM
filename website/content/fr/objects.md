# Objets et scènes de travail

POSIM comprend des modèles de manipulation, d'inspection et d'interaction. `posim_object_models` installe leurs descriptions, maillages et configurations. Exécutez les commandes dans le [terminal POSIM de votre installation](terminals.md), y compris sous WSL, Docker ou RDP.

## Catalogue local

Les répertoires suivants proviennent de `models/posim_object_models/description`. Ouvrez-les pour examiner les SDF, liens, articulations et ressources. Les mondes d'exemple peuvent aussi charger des objets Gazebo Fuel.

{{OBJECT_CATALOG}}

## Commencer par une scène préparée

{{CODE:objects:0}}

Les autres scènes comprennent `posim_electrical_mating` et `posim_bimanual_example`. Elles utilisent notamment un panneau de bloc obturateur de puits, des connecteurs mâles et femelles et des vases immergés provenant de Fuel. Lisez leurs URI et positions dans le SDF avant de placer le robot ou d'étudier contacts et articulations.

## Ajouter un objet à un monde

Remplacez l'URI par le nom réel de la ressource choisie :

{{CODE:objects:1}}

Les positions sont en mètres et les angles en radians. Donnez un nom distinct à chaque instance. Choisissez délibérément le comportement `static` : une cible d'inspection fixe et un objet manipulé par contact demandent des dynamiques différentes.

La géométrie visuelle définit l'apparence ; la géométrie de collision détermine le contact. Vérifiez échelle, pose, masse, inertie, limites articulaires et frottement. Commencez par une interaction limitée et contrôlée, puis enregistrez les états avec [ROS 2](ros.md).

## Lancer un objet seul

Après avoir chargé l'environnement, lancez `mossy_cinder_block` :

{{CODE:objects:2}}

Ce lancement crée son propre monde. Pour le combiner avec un autre exemple, composez un seul monde avec des éléments `<include>`. Consultez [Ajouter un robot](custom-robots.md) et la [bibliothèque de mondes](worlds.md).

## Utiliser Fuel en local

Téléchargez et décompressez un modèle du [catalogue Fuel](https://app.gazebosim.org/fuel/models). Placez ensemble `model.sdf`, `model.config`, `meshes/` et `materials/`, par exemple dans `~/posim_models/my_object/`. Déclarez leur répertoire parent dans le terminal qui lance Gazebo :

{{CODE:objects:3}}

Ouvrez **Resource Spawner** dans Gazebo pour placer le modèle local. Réutilisez-le dans un SDF avec `<uri>model://my_object</uri>`. L'URI HTTPS du fragment SDF fourni par Fuel fonctionne également, après un premier téléchargement. Conservez les fichiers et leurs licences pour les expériences hors ligne.

## Publier sur Fuel

Connectez-vous à [Gazebo Fuel](https://app.gazebosim.org/home) et créez un modèle. Renseignez nom, description et licence, puis envoyez `model.sdf`, `model.config` et les maillages et matériaux référencés. Téléchargez le résultat et vérifiez l'absence de dépendances vers des chemins locaux externes. Ajoutez-le à une collection si nécessaire.

Choisissez les bibliothèques et classes de plugins présentes dans Jetty. [Ajouter un robot](custom-robots.md) explique les raccordements de flottabilité, hydrodynamique, propulsion et capteurs.

{{MEDIA:objects}}

Images : wiki Notion POSIM. L'attribution de la documentation DAVE figure dans [Citations et licences](citation.md). Utilisez les blocs de cette page pour les commandes et les noms de topics.
