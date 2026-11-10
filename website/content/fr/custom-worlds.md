# Créer un monde

Commencez par un petit monde avec un capteur de pression à une profondeur connue. Vous pourrez modifier l'environnement et mesurer le résultat avant d'ajouter terrain et robot mobile.

## Préparer l'environnement de travail

Utilisez les [terminaux Linux préparés](terminals.md) sous Ubuntu natif, WSL ou Docker. Sur macOS, ouvrez le terminal du bureau RDP. Modifiez et compilez dans cet environnement ; `POSIM_WS` désigne son espace de travail.

## Enregistrer le monde complet

Enregistrez ce contenu dans `calm_water.sdf`. Il utilise des formes simples et des plugins installés, sans maillage à télécharger.

{{CODE:custom-worlds:0}}

Ce monde contient :

- **Physique :** pas maximal de 0,001 s, facteur de temps réel visé de 1 et gravité vers le bas.
- **Éclairage et scène :** soleil directionnel, lumière ambiante et couleur de fond.
- **Fond marin :** boîte fixe à z = −20 m, avec géométries visuelle et de collision identiques.
- **Station de pression :** repère fixe à z = −2 m. Son plugin de [pression](pressure.md), placé au niveau du modèle, publie pression et profondeur estimée.

La référence de pression est le plan z = 0 du monde. Ajoutez vagues, flottabilité et mouvement après avoir vérifié ce cas simple.

## Valider et lancer

Dans le terminal A, depuis le répertoire du fichier :

{{CODE:custom-worlds:1}}

La validation doit réussir, puis Gazebo s'ouvre. `-r` démarre le temps simulé. Sélectionnez `pressure_station` dans l'arbre des entités et centrez la vue dessus.

Dans un autre terminal avec le même environnement chargé :

{{CODE:custom-worlds:2}}

Les valeurs attendues sont environ **120 938 Pa** et **2 m**. Le plugin publie directement dans ROS ; cet exemple n'exige pas de pont supplémentaire.

## Modifier la profondeur

1. Arrêtez le terminal A avec Ctrl+C.
2. Remplacez la pose `0 0 -2 0 0 0` par `0 0 -4 0 0 0`.
3. Validez et relancez le monde.
4. Répétez les deux lectures. Attendez environ **140 551 Pa** et **4 m**.

Gardez les coefficients de pression inchangés. L'augmentation attendue est **19 612,76 Pa**. Conservez les deux fichiers avec vos observations.

## Retrouver chaque réglage

| Chemin SDF | Unité | Rôle |
|---|---|---|
| `world/physics/max_step_size` | s | Pas maximal : `0.001` |
| `world/physics/real_time_factor` | — | Rapport visé entre temps simulé et réel : `1.0` |
| `world/gravity` | m/s² | Vecteur de gravité |
| `world/scene`, `world/light` | RGBA / direction | Fond et éclairage |
| `model/pose` | m, rad | Position et orientation |
| `model/static` | booléen | Maintenir station et fond fixes |
| `model/plugin` | selon le plugin | Paramètres de [pression](pressure.md) |

La vitesse obtenue dépend de la charge. La définition d'un capteur et son système dans le monde ont des portées différentes. [Ajouter des capteurs](custom-sensors.md) ajoute une IMU et son pont ROS à ce fichier.

## Enrichir le monde

- [Terrains](heightmaps.md) : remplacer le fond et vérifier les collisions.
- [Ajouter un robot](custom-robots.md) : mécanique, propulsion et ROS.
- [Courants](currents.md) : écoulement du monde et réaction du modèle.
- [Coordonnées sphériques](coordinates.md) : origine géographique.
- [Effets sous-marins](underwater-rendering.md) : rendu visuel.

Placez les mondes à distribuer dans `models/posim_worlds/worlds` et leurs ressources dans le même paquet. Recompilez, rechargez l'environnement et utilisez des URI de ressources installées pour les partager.
