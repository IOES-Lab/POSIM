# Terrains par carte de hauteur

Une carte de hauteur représente le terrain par une grille régulière d'altitudes. Configurez ensemble les géométries visuelle et de collision pour que le fond visible corresponde à la surface de contact.

## Environnement de travail

Utilisez les [terminaux Linux préparés](terminals.md) sous Ubuntu natif, WSL ou Docker. Sur macOS, travaillez dans le bureau RDP. Modifiez et compilez dans cet environnement ; `POSIM_WS` désigne son espace de travail.

## Consulter les exemples

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) présente une scène géographique avec le modèle Fuel `Santorini Scaled`. Le [fond à niveaux](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world) fournit un autre exemple de composition.

Pour créer la géométrie, consultez le [tutoriel Gazebo](https://gazebosim.org/api/sim/10/heightmap_dem.html) et la [référence SDFormat](http://sdformat.org/spec?ver=1.12&elem=geometry).

## Conserver les ressources dans le paquet

Placez le monde dans `models/posim_worlds/worlds` et les données dans `models/posim_worlds/media`. Vérifiez les règles d'installation CMake et les chemins de ressources du paquet. Utilisez des URI portables de ressources installées.

Dans une géométrie visuelle ou de collision, la structure générale est :

{{CODE:heightmaps:0}}

Remplacez l'URI par votre propre ressource. Choisissez un format raster et un échantillonnage compatibles avec le moteur de rendu et le moteur physique. Vérifiez l'encodage de l'altitude avant de régler taille verticale et décalage.

## Préserver échelle et coordonnées

`size` donne l'étendue horizontale et l'échelle verticale en mètres ; `pos` ajoute un décalage. Une image normalisée n'est pas automatiquement géoréférencée. Notez étendue d'origine, plage d'altitudes, référence du niveau marin et toute reprojection ou tout rééchantillonnage.

Si les positions géographiques comptent, définissez `<spherical_coordinates>` dans le monde. Les [coordonnées sphériques](coordinates.md) expliquent la conversion entre latitude/longitude et coordonnées locales.

## Compiler et vérifier

Recompilez et chargez l'environnement après l'ajout de ressources. Lancez le monde hors du dépôt. Vérifiez :

- Reliefs identifiables et alignement visuel/collision.
- Profondeur mesurée par les capteurs.
- Contact avec un essai de chute contrôlé.
- Niveau marin et hauteur initiale du robot.

Sous Jetty, un `<heightmap><pos>` non nul peut décaler le terrain affiché sans déplacer sa collision de la même façon. Reporter ce déplacement sur la pose du modèle peut également laisser un désalignement. Testez les deux avec vos moteurs de rendu et de physique. Si l'écart persiste, utilisez une géométrie ou un moteur qui satisfait les contrôles de contact et de rendu avant les essais de navigation.
