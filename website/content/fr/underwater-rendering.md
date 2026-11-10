# Effets de scène sous-marine

`posim_gz_rendering` applique voile sous-marin, absorption RVB, lumière solaire dépendant de la profondeur, caustiques et particules en suspension aux surfaces PBR d'Ogre2. Les particules évoquent la neige marine et sont **désactivées** par défaut.

Scène Gazebo, caméras RGB et couleur RGBD partagent ces effets. La profondeur numérique et les collisions restent inchangées. Le filtre indépendant fondé sur la profondeur est décrit dans [Caméra sous-marine](camera.md).

## Compiler et intégrer le système

Préparez un environnement de développement Gazebo Jetty/Ogre2 correspondant au serveur. Dans le [terminal A préparé](terminals.md), compilez depuis les sources POSIM ; sur macOS, utilisez le terminal Linux du bureau RDP :

{{CODE:underwater-rendering:0}}

CMake exige les paquets de développement Gazebo et les entrées pkg-config `OGRE-Next` et `OGRE-Next-Hlms`. La commande inclut le chemin du paquet ROS `gz_ogre_next_vendor` sous Ubuntu. Utilisez bibliothèques, en-têtes et shaders d'une même compilation.

Ajoutez ce plugin dans `<world>` et remplacez `/absolute/underwater` par le préfixe d'installation précédent. Chargez une seule instance par processus Gazebo, avec un `session_nonce` propre à chaque session :

{{CODE:underwater-rendering:1}}

Linux utilise `.so`, y compris dans Docker/RDP sur macOS. Les bibliothèques compilées directement pour macOS utilisent `.dylib`. Une caméra active dans la scène est nécessaire pour produire les images.

Si vous utilisez aussi `UnderwaterCamera`, définissez `POSIM_UNDERWATER_PLUGIN` dans l'environnement du serveur avec le chemin du système chargé. Les pixels déjà colorés passent alors sans seconde atténuation. Ne le définissez qu'une fois le système de scène installé et chargé.

## Effets et valeurs par défaut

| Réglage | Effet | Défaut |
|---|---|---|
| Optique de l'eau | Interrupteur général des effets optiques | Activée |
| Voile sous-marin | Mélange de couleur selon le trajet sous l'eau | Activé |
| Absorption RVB | Perte de couleur selon la longueur d'onde | Activée |
| Atténuation solaire | Lumière directionnelle plus faible en profondeur | Activée |
| Caustiques | Motifs lumineux animés sur les surfaces immergées | Activées |
| Particules / neige marine | Points lumineux procéduraux clairsemés | Désactivées |
| Texture du fond | Détails des cartes PBR du fond | Activée |
| Grille du fond | Espacement 10 m, réglable de 1 à 500 m | Désactivée |
| Isobathes | Espacement 5 m, réglable de 1 à 50 m | Désactivées |

Couper l'optique conserve les choix de texture, grille et isobathes. Les superpositions reconnaissent les visuels `seabed` ou `shallow`, y compris leurs noms complets Gazebo. Fournissez les textures dans les ressources du terrain ; voir [Terrain par carte de hauteur](heightmaps.md).

<a id="turn-on-marine-snow"></a>
## Activer la neige marine

Une fois le monde démarré, lisez l'état dans le terminal B préparé :

{{CODE:underwater-rendering:2}}

Les 13 premières valeurs suivent cet ordre ; les indices commencent à zéro :

| Indice | Valeur |
|---|---|
| 0–1 | Schéma `1`, révision |
| 2–5 | Optique, texture du fond, grille, isobathes |
| 6–10 | Voile, absorption, atténuation solaire, caustiques, particules |
| 11–12 | Espacement de la grille et des isobathes, m |

Sur un système qui vient de démarrer à la révision `0`, cette requête active les particules en conservant les autres valeurs par défaut :

{{CODE:underwater-rendering:3}}

Pour les modifications suivantes, conservez les réglages voulus et augmentez la révision de un. Les booléens acceptent `0` ou `1`. Une requête identique peut être répétée.

Après rendu, relisez `/state` : `ready`, nombre de caméras et `renderer_failed` sont ajoutés aux indices 13–15. Vérifiez la révision demandée, `ready = 1` et `renderer_failed = 0`. La réponse de `/set` confirme seulement la réception.

Pour activer les particules dès le démarrage, ajoutez cette balise au plugin à la place de la première requête :

{{CODE:underwater-rendering:4}}

WWW-POSIM utilise ce moteur public et ajoute son panneau d'apparence, sa gestion de sessions et ses préférences.

## Limites du modèle et comparaison

Le calcul du trajet optique sous-marin utilise un plan moyen à **z = 0 m**, sans suivre chaque crête. Caustiques et particules sont des approximations visuelles ; elles ne simulent pas de concentrations étalonnées ni le bruit optique du capteur.

Les lampes locales conservent leur éclairage direct. Les rayons du ciel subissent l'atténuation sous-marine ; les matériaux non éclairés et les shaders propres à la surface océanique gardent leur rendu. Le dessous des vagues dépend donc du matériau de surface.

Comparez les vues au-dessus/sous l'eau, RGB et couleur RGBD, puis chaque interrupteur. Gardez pose et lumière constantes. Vérifiez la profondeur identique octet par octet, la pause des effets avec le temps simulé et le signalement des pannes. Le [paquet public](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering) fournit des cas de vérification RGB/RGBD, les shaders et les détails de compilation.
