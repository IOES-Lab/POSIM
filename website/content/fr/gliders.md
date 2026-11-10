# Planeur Slocum

Le modèle `glider_slocum` comprend le corps, un propulseur, l'hydrodynamique ainsi que les interfaces IMU, navigation et pression. Utilisez-le pour une expérience de planeur ou comme base d'un autre véhicule.

## Lancer dans l'océan

Dans le [terminal POSIM préparé pour votre installation](terminals.md) :

{{CODE:gliders:0}}

Pour examiner le modèle dans un monde vide, utilisez `world_name:=empty.sdf z:=0.2`.

## Lire les observations

{{CODE:gliders:1}}

Les topics Gazebo IMU et NavSat se trouvent sous `/model/glider_slocum/`. Consultez la configuration des ponts pour choisir leurs topics et types ROS. Le modèle charge aussi le [plugin de pression](pressure.md) et la publication d'odométrie de Gazebo.

Une fois le véhicule et le monde lancés, connectez votre nœud de commande pour une plongée ou une mission de vol plané sous-marin.

## Adapter le modèle

Lisez le [SDF de Slocum](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) et sa configuration de ponts. Modifiez de manière cohérente masse, inertie, géométrie de collision et coefficients hydrodynamiques. Vérifiez les conventions de repères avant de connecter un estimateur ou un contrôleur.

Gardez le modèle d'origine pour les comparaisons. [Ajouter un robot](custom-robots.md) explique comment créer une autre description ; [ROS](ros.md), comment l'observer et la commander.

## Voir le planeur en mouvement

{{MEDIA:gliders}}

Images et vidéos : POSIM Notion Wiki. L'attribution de la documentation DAVE figure dans [Citations et licences](citation.md). Les commandes et topics à utiliser sont ceux de cette page.
