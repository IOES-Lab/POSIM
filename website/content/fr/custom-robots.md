# Ajouter un robot

Partez d'un robot POSIM de disposition similaire. Le SDF décrit la mécanique et les systèmes ; la configuration ROS relie mesures et commandes.

## Environnement de travail

Utilisez les [terminaux Linux préparés](terminals.md) sous Ubuntu natif, WSL ou Docker. Sur macOS, travaillez dans le terminal du bureau RDP. Modifiez et compilez dans cet environnement ; `POSIM_WS` est le chemin de son espace de travail.

## 1. Copier un modèle de référence

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) utilise des propulseurs Gazebo. [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) intègre ArduSub/MAVROS. [Slocum](gliders.md) présente une autre disposition.

Depuis `$POSIM_WS`, copiez entièrement REXROV pour conserver maillages, configuration du pont et noms des topics :

{{CODE:custom-robots:0}}

Enregistrez le programme suivant dans `copy_robot.py`, puis exécutez `python3 copy_robot.py`. Il s'arrête si `my_robot` existe déjà.

{{CODE:custom-robots:1}}

La structure obtenue comprend :

{{CODE:custom-robots:2}}

`my_robot` est l'identifiant choisi. Le lancement recherche `description/<namespace>/model.sdf` et la configuration correspondante.

## 2. Définir mécanique et ressources

Conservez la mécanique copiée pour le premier lancement. Lors d'une modification de coque, vérifiez ensemble :

| Chemin SDF | Unité | Vérification |
|---|---|---|
| `model/@name`, noms des liens et articulations | identifiant | Concordance avec plugins et configuration ROS |
| `link/inertial/mass` | kg | Masse positive de chaque corps mobile |
| `link/inertial/pose` | m, rad | Centre de masse et orientation du repère d'inertie |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | Moments autour des axes de ce repère |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | Termes croisés ; tenseur complet physiquement valide |
| `link/visual`, `link/collision` | m, rad | Échelle et alignement des géométries |
| `joint/axis`, paramètres des propulseurs | selon le plugin | Sens d'actionnement et répartition des forces |

STL fournit la géométrie. Définissez masse et dynamique dans le SDF ; vérifiez unités et échelle du maillage. Reprenez les URI installées des exemples. Préférez une collision simple si le détail visuel rend le calcul du contact inutilement coûteux.

## 3. Relier systèmes et ROS

Adaptez hydrodynamique, flottabilité, axes de propulsion et poses des capteurs au véhicule. Les coefficients doivent correspondre à ses propriétés physiques.

Consultez [Courants](currents.md), [Ajouter des capteurs](custom-sensors.md) et les références de plugins associées. Pour latitude et longitude, définissez l'origine avec les [coordonnées sphériques](coordinates.md).

Adaptez `config/<namespace>/robot_config.py` en mettant à jour ensemble noms d'entités et d'articulations, topics, repères et ports. Le paquet installe déjà `description`, `meshes`, `config` et les réglages de recherche des ressources. Modifiez les sources, pas les fichiers générés dans `install/`.

## 4. Compiler et essayer

Depuis l'espace de travail dont l'environnement est chargé :

{{CODE:custom-robots:3}}

Essayez chaque axe avec de petites commandes. Vérifiez aussi un lancement hors du répertoire source :

- Chemins des ressources installées.
- Masse, inertie et pose d'équilibre.
- Géométrie de collision et réaction au contact.
- Repères et contenu des messages.

Dans un autre terminal préparé, lisez le modèle renommé :

{{CODE:custom-robots:4}}

Vous devez recevoir position et vitesse. Faites fonctionner la copie, puis modifiez la mécanique, enfin les capteurs ou la propulsion. Comparez les essais avec [Données des capteurs](visualization.md) et consultez le [dépannage](troubleshooting.md) pour les ressources manquantes.
