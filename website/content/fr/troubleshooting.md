# Dépannage

Commencez par la première erreur du journal de lancement. Vérifiez dans l'ordre l'environnement, les ressources du modèle, le temps de simulation, les plugins et la réception des données. Conservez l'erreur et les lignes voisines utiles pour la reproduire.

## Un paquet est introuvable

{{CODE:troubleshooting:0}}

Vous devez obtenir `lyrical` et le chemin de l'espace de travail choisi. Préparez le [terminal](terminals.md) selon votre [méthode d'installation](requirements.md) et distinguez les chemins de l'hôte de ceux du conteneur.

## Des modèles ou des maillages manquent

Repérez la première URI introuvable. Vérifiez que les ressources du paquet sont installées et que vous avez chargé le bon environnement. Gazebo a besoin d'un accès réseau pour les ressources Fuel. Le premier téléchargement peut prendre du temps : suivez sa progression avant de relancer une simulation.

## L'image est absente ou le rendu ne démarre pas

Vérifiez pilotes et connexion d’affichage dans le [guide d’installation](requirements.md). Gazebo et les caméras ont besoin d’un rendu fonctionnel. Pour le sonar, vérifiez le [moteur choisi et les paquets installés](sonar-tuning.md).

Pour les [effets de scène sous-marine](underwater-rendering.md), vérifiez :

- Le service `/posim/appearance/<session_nonce>/state`, les chemins du moteur et des shaders, ainsi que le nombre de caméras actives.
- La révision appliquée et l'état de disponibilité après le rendu.
- L'activation explicite des particules en suspension, désactivées par défaut. L'exemple d'atténuation de caméra ne charge pas à lui seul le système d'effets de scène.

## Le GNSS ne fournit pas de position valide

Consultez l'[état de validité GNSS](gnss.md), la hauteur de l'antenne dans le monde, la hauteur locale de l'eau et l'horodatage d'acquisition. Vérifiez les coordonnées sphériques du monde, une antenne émergée et sans obstruction, et un temps de simulation suffisant pour la réacquisition et la latence.

Si un topic de vagues est configuré, il doit fournir des hauteurs locales compatibles et récentes. Installer le récepteur ne l'ajoute pas au modèle et ne fournit pas d'adaptateur ROS ou de pilote automatique.

## Aucun message n'arrive

Vérifiez d'abord que le temps avance, puis recherchez le topic et son type :

{{CODE:troubleshooting:1}}

Utilisez le bon transport et le même environnement préparé. Vérifiez la QoS, le domaine et les erreurs du pont ou du plugin. Pour le lancement d'un capteur, `namespace` doit désigner un descripteur existant. L'[USBL](usbl.md) est déjà intégré à son monde d'exemple et se lance avec celui-ci.

## Les topics d'image existent, mais les images n'arrivent pas {#image-topics-appear-but-frames-stall}

Si Gazebo produit des images et que ROS découvre le publisher, mais reçoit peu d'images ou aucune, examinez le transport DDS. Les gros datagrammes UDP de Cyclone DDS peuvent atteindre les limites de fragmentation IP. Réduire leur taille peut aider sur un réseau dont la MTU est de 1500 octets. Consultez les [réglages de fragmentation de Cyclone DDS](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation).

### Préparer le profil

Avec `rmw_cyclonedds_cpp` installé, créez ce profil une seule fois. Si vous utilisez déjà un profil Cyclone DDS, ajoutez les deux tailles à sa section `General`, en conservant les réglages de découverte et d'interfaces.

{{CODE:troubleshooting:2}}

### Relancer avec les mêmes réglages

Arrêtez la simulation et les outils de visualisation. Chargez l'environnement POSIM, puis sélectionnez le profil dans chaque terminal avant de les relancer. Si vous avez modifié un profil existant, utilisez son chemin.

{{CODE:troubleshooting:3}}

Si le démon de la CLI ROS tourne déjà dans ce domaine, arrêtez-le une fois afin que les prochaines commandes prennent en compte la nouvelle configuration :

{{CODE:troubleshooting:4}}

Refaites les [contrôles d'image et de cadence](camera.md#run-the-example), en examinant aussi les horodatages d'acquisition. Les petits paquets augmentent la surcharge ; une MTU plus basse exige des tailles encore plus petites.

## Le véhicule ne bouge pas

Vérifiez l'état du contrôleur, l'armement, la validité de navigation, la réception des commandes et le mode choisi. Contrôlez les noms d'articulations et les axes des propulseurs. Gardez une seule source de commandes active. Une commande neutre, un monde en pause ou un pilote automatique désarmé peuvent expliquer l'immobilité.

## La simulation ne s'arrête pas

Arrêtez les outils de visualisation, appuyez une fois sur Ctrl+C dans le terminal de lancement et laissez les processus se terminer. Conservez les messages pertinents d'abandon, d'erreur de segmentation ou d'arrêt forcé. Pour [signaler le problème](https://github.com/IOES-Lab/POSIM/issues), indiquez la commande, la révision, le système, l'architecture et le moteur de rendu.

### RViz plante après Discard

L'installateur POSIM pour Ubuntu/WSL et les images Docker AMD64/ARM64 incluent un [overlay RViz isolé](https://github.com/IOES-Lab/POSIM/tree/main/extras/rviz) pour RViz 15.2.6. Il arrête le minuteur de mise à jour une fois la fermeture confirmée et libère les ressources ROS détenues par les vues lors de l'arrêt. Le choix **Cancel** laisse les mises à jour actives. Les paquets RViz du système restent inchangés. Après avoir chargé l'environnement POSIM, `ros2 pkg prefix rviz_common` doit afficher `/opt/posim/rviz-close/install`. Pour une installation plus ancienne, suivez les instructions de compilation et d'activation du lien.

Sans l'overlay, RViz 15.2.6 sur Ubuntu 26.04 peut parfois planter à la fermeture de sa fenêtre après le choix **Discard**, même si les vues des capteurs fonctionnent. L'overlay corrige des problèmes précis de minuteur et de libération des ressources, sans couvrir tous les moteurs de rendu ni tous les chemins d'arrêt. Si un problème persiste, précisez la version de RViz, les chemins des bibliothèques chargées et l'action de fermeture dans le rapport.

Si RViz continue à fonctionner après la fermeture de sa fenêtre, il peut consommer trop de mémoire, notamment avec le rendu logiciel. Avant de relancer la simulation, appuyez sur Ctrl+C dans le terminal de lancement pour arrêter les processus restants. Si RViz ne s'arrête pas normalement, ROS launch force l'arrêt du processus. Pour exécuter le sonar sans visualiseur, ajoutez `rviz:=false` à la commande `multibeam_sonar_demo.launch.py`. Les images restent disponibles dans ROS.

## Recréer une caméra de profondeur avec un SDK macOS natif

Pour le SDK **macOS arm64 / Ogre-Next 2.3.3**, l'[environnement Ogre isolé](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) fournit la correction de durée de vie des matériaux lors de la destruction et de la recréation des caméras de profondeur.

Respectez les versions de SDK et de sources indiquées, utilisez l'outil de compilation, puis transmettez votre commande Gazebo à `run_overlay.py`. La compilation seule ne sélectionne pas cette bibliothèque. Vérifiez les chemins effectivement chargés et conservez le résultat de validation. L'installation standard n'active pas cet environnement ; il ne remplace pas les bibliothèques système et ne configure pas les conteneurs Linux. Les autres versions ou plateformes nécessitent leur propre profil compatible.

## Fournir un cas reproductible

Indiquez le SHA du code ou le digest de l'image, les versions des dépendances, la commande et ses arguments, la première erreur, le résultat attendu, les types de topics concernés et un exemple minimal. Distinguez un topic simplement découvert d'un topic qui transmet réellement des données.

## MAVROS 2.16.0 s'arrête avec SIGSEGV

Le programme d'installation de POSIM et les builds Docker officiels intègrent le
[correctif de durée de vie du Router](https://github.com/IOES-Lab/POSIM/tree/main/extras/mavros)
pour MAVROS 2.16.0. Après avoir chargé l'environnement POSIM, `ros2 pkg prefix mavros`
doit afficher `/opt/posim/mavros-router/install`. Pour une installation existante,
suivez les instructions de compilation séparée indiquées dans ce lien. Si le
plantage persiste, joignez au rapport le journal de MAVROS et les chemins des
bibliothèques effectivement chargées.

## QGC est ouvert sous Windows, mais POSIM fonctionne dans WSL {#qgc-on-windows-host}

Fermez l’application Windows, puis lancez la version Linux de QGC dans la même distribution Ubuntu que POSIM ou dans le conteneur POSIM. Une fenêtre WSLg apparaît sur le bureau Windows, mais reste un processus Linux. La [configuration de QGC](qgroundcontrol.md) conserve la télémétrie et la vidéo dans cet environnement.
