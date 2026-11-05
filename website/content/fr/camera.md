# Caméra sous-marine

`UnderwaterCamera` utilise la profondeur pour atténuer les couleurs RVB avec la distance. Ajustez l'atténuation et la couleur de fond selon l'eau à représenter. Pour des effets communs à toute la scène, consultez le [rendu sous-marin](underwater-rendering.md).

<a id="run-the-example"></a>
## Exécuter l'exemple

Cet exemple utilise le filtre de caméra fondé sur la profondeur. Le moteur de scène a sa propre [compilation et configuration](underwater-rendering.md). Dans le [terminal A préparé pour votre installation](terminals.md) :

{{CODE:camera:0}}

Dans le terminal B préparé :

{{CODE:camera:1}}

Ajoutez un affichage Image dans RViz. Si le topic apparaît mais que les images n'arrivent plus, vérifiez le [transport des grandes images](troubleshooting.md#image-topics-appear-but-frames-stall).

## Configurer le plugin

Placez ce capteur RGBD dans un `<link>`. Le monde doit charger `gz-sim-sensors-system` avec `<render_engine>ogre2</render_engine>`, comme le [monde de caméra](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world). Le plugin lit couleur/profondeur Gazebo et publie directement le topic ROS `/underwater_camera/simulated_image`.

{{CODE:camera:2}}

| Paramètre SDF | Signification | Défaut |
|---|---|---|
| `attenuationR` | Atténuation rouge par mètre, finie et ≥ 0 | `1/30` |
| `attenuationG` | Atténuation verte par mètre, finie et ≥ 0 | `1/30` |
| `attenuationB` | Atténuation bleue par mètre, finie et ≥ 0 | `1/30` |
| `backgroundR` | Fond rouge, entier de 0 à 255 | `0` |
| `backgroundG` | Fond vert, entier de 0 à 255 | `0` |
| `backgroundB` | Fond bleu, entier de 0 à 255 | `0` |

## Topics et géométrie de caméra

`<topic>` doit produire un nom ROS valide. Sans cette balise, le plugin utilise le chemin Gazebo `/world/<world>/model/<model>/link/<link>/sensor/<sensor>`, auquel il ajoute `/simulated_image` pour ROS. Les modèles imbriqués ajoutent les noms de leurs modèles parents.

La conversion profondeur-distance privilégie une projection de lentille explicite, puis les paramètres intrinsèques explicites, enfin la taille de l'image et le champ de vision horizontal. Les coordonnées du point principal peuvent être nulles. Les focales doivent être positives, les valeurs de lentille finies et les plans de coupe respecter `0 < near < far`, en mètres. Une configuration invalide produit une erreur et désactive la sortie.

## Synchronisation et valeurs d'exemple

Couleur et profondeur sont associées par leur horodatage d'acquisition, quel que soit l'ordre d'arrivée. ROS conserve l'horodatage et l'identifiant de repère de l'image couleur. Un tampon plein élimine les images sans correspondance ; la remise à zéro du temps vide les images en attente et autorise un nouveau départ.

L'exemple demande **320 × 240 à 10 Hz**, une atténuation RVB **0.8/0.5/0.2** et un fond **85/107/47**. Le SDF remplace les valeurs par défaut du code.

## Régler et comparer

Placez un objet reconnaissable à des distances connues. Gardez pose, éclairage, matériaux et caméra fixes en modifiant un canal. Vérifiez aussi les horodatages et la fréquence reçue.

L'atténuation suit une loi exponentielle par canal, avec mélange du fond. Pour une comparaison sans atténuation, mettez les **six paramètres** du tableau à zéro. Les balises omises reprennent les valeurs par défaut.

Le [moteur de scène](underwater-rendering.md) applique des effets communs aux surfaces PBR et aux caméras. Quand ce système natif fonctionne, `POSIM_UNDERWATER_PLUGIN` transmet les pixels déjà colorés pour éviter un second filtrage. Le [modèle](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) décrit connexions et topics.

## Particules en suspension

Le moteur peut afficher des particules dans la couleur RGB/RGBD. Désactivées au départ, elles s'activent par le [service d'apparence](underwater-rendering.md#turn-on-marine-snow).

- Leur position dans le monde est commune aux caméras. Elles dérivent uniformément avec le temps simulé, indépendamment des courants.
- Les surfaces opaques les masquent. Elles sont visibles de **0.05 à 3 m**, dans les limites de la scène et du niveau moyen de l'eau.
- La profondeur numérique et les rayons acoustiques conservent leurs valeurs.

Ces effets optiques sont procéduraux. Une étude quantitative de la diffusion exige un modèle optique adapté. En navigation visuelle, traitez les particules comme des observations mobiles et conservez les réglages entre comparaisons.

## Comparer les images sous-marines

{{MEDIA:camera}}

Images : wiki Notion POSIM ; attribution DAVE dans [Citations et licences](citation.md). Utilisez les commandes et topics des blocs de cette page.
