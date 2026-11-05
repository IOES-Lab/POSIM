# Configurer et régler le sonar

Choisissez couverture, détail et fréquence pour votre expérience. La [référence du sonar multifaisceaux](sonar.md) fournit démarrage rapide, SDF et paramètres ; ce guide explique les comparaisons et la préparation de CUDA.

## Environnement nécessaire

Utilisez les [terminaux Linux préparés](terminals.md) sous Ubuntu ou WSL 2 compatible avec un GPU NVIDIA, ou dans un conteneur y ayant accès. Modifiez et compilez dans cet environnement ; `POSIM_WS` est son espace de travail. Docker/RDP sur macOS n'expose pas de GPU CUDA : exécutez ce sonar sur un hôte Linux ou WSL 2 équipé de NVIDIA.

## Mesurer une référence

Lancez l'[exemple BlueView P900](sonar.md#run-a-sonar-example). Gardez scène, distance de cible et pose du capteur fixes. Vérifiez l'image et les données brutes avant de mesurer. Installez d'abord CUDA si nécessaire.

Dans un autre terminal préparé, mesurez la fréquence reçue :

{{CODE:sonar-tuning:0}}

Après un échantillon stable, arrêtez avec Ctrl+C. Consultez séparément la vitesse de simulation :

{{CODE:sonar-tuning:1}}

Le nom interne du monde est `default`. Notez `real_time_factor` : demander 30 Hz simulés ne garantit pas 30 images par seconde réelle. Ctrl+C arrête la lecture.

## Comparer une variable

Modifiez un réglage dans une copie du [descripteur](sonar.md#sensor-configuration), recompilez, chargez l'environnement et redémarrez à la même pose.

| Expérience | Comparaison | Observation |
|---|---|---|
| Détail angulaire | 512 contre 256 faisceaux horizontaux, champ constant | Séparation des cibles et fréquence |
| Échantillonnage vertical | 300 contre 150 rayons | Continuité des cibles et temps de calcul |
| Couverture | Modifier ensemble portée des rayons et `maxDistance` | Cible la plus éloignée et échelle de distance |
| Actualisation | 30 contre 10 Hz | Fréquence reçue et facteur de temps réel |
| Saut de rayons | `raySkips` à 10 puis 5 | Détail vertical et coût de calcul |

Mesurez le résultat de chaque comparaison. Désactivez `writeLog` et `debugFlag` pour le débit ; activez-les séparément pour étudier retours ou chronométrage. Conservez SDF, révision POSIM, versions GPU/CUDA et observations. [ROS 2](ros.md#visualization-and-recording) explique l'enregistrement des topics.

## Préparer les outils

Le [plugin](sonar.md) nécessite pilote NVIDIA, compilateur et runtime CUDA, cuFFT, cuBLAS et rendu Gazebo. Dans l'environnement qui exécutera le sonar :

{{CODE:sonar-tuning:2}}

Vérifiez séparément GPU et compilateur. Avec Docker Engine sous Linux, configurez [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) sur l'hôte. Sous Windows, suivez la [configuration GPU Docker Desktop/WSL 2](https://docs.docker.com/desktop/features/gpu/). L'image doit aussi contenir les bibliothèques CUDA.

## Choisir l'architecture CUDA

Réglez `CMAKE_CUDA_ARCHITECTURES` sur une version SM compatible avec le GPU et le toolkit. Cet exemple utilise SM 75 : remplacez `75` par votre architecture, depuis l'espace de travail préparé.

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` liste les cibles prises en charge. Sans réglage explicite, CMake reprend le choix par défaut du compilateur. Avec plusieurs installations CUDA, utilisez `-DCUDAToolkit_ROOT=/path/to/cuda` et ajoutez son répertoire `bin` à `PATH`.

## Vérifier les composants installés

{{CODE:sonar-tuning:4}}

Un paquet peut être enregistré même si ses composants CUDA ont été omis. Vérifiez les bibliothèques :

{{CODE:sonar-tuning:5}}

Chaque commande doit renvoyer le code 0. Examinez les composants omis ou erreurs de chargement, puis recevez les données décrites dans [Sonar multifaisceaux](sonar.md).

## Interpréter les mesures

Laissez le rendu se stabiliser avant de chronométrer. À scène et pose identiques, notez temps par image, fréquence reçue, facteur de temps réel, utilisation CPU/GPU et mémoire. Distinguez physique, rendu, calcul CUDA, publication et journalisation.

`blazingSonarImage` choisit entre une graine aléatoire variable et la graine fixe `1234` ; comparez la répétabilité. Conservez des données de référence lors de changements numériques et comparez intensité et distance, en plus des images. Les [citations](citation.md) donnent les recherches d'origine.

## Traitement GPU

{{MEDIA:sonar-tuning}}

Le traitement comprend sommation des rayons, fenêtrage/correction et FFT. Lors du profilage, séparez ces étapes des transferts mémoire GPU. Image : wiki Notion POSIM.
