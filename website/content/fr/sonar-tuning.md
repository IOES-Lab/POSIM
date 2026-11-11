# Configurer et régler le sonar

Choisissez le moteur de calcul, puis comparez portée, détail et fréquence de mise à jour. La [référence du sonar multifaisceaux](sonar.md) présente le démarrage rapide, le SDF et les paramètres.

## Choisir l'environnement

Utilisez les [terminaux POSIM préparés](terminals.md). Modifiez, compilez et exécutez dans la même installation Ubuntu, WSL ou Docker ; `POSIM_WS` désigne son espace de travail. Tous les moteurs de calcul ont besoin des capteurs de rendu de Gazebo.

| Environnement | Possibilités |
| --- | --- |
| Linux natif | CPU ; WGPU avec adaptateur Vulkan utilisable ; CUDA avec GPU NVIDIA et toolkit compatibles |
| WSL 2 ou conteneur Linux | CPU ; le calcul GPU exige un adaptateur et un pilote accessibles **dans** cet environnement |
| Docker/RDP sous macOS | Commencez par CPU. Le conteneur Linux n'expose pas le périphérique Metal du Mac. |
| macOS natif | WGPU peut utiliser Metal avec une chaîne ROS/Gazebo native préparée séparément ; le [guide macOS](install-macos.md) décrit Docker/RDP. |

`auto` essaie CUDA → WGPU → CPU. Pour une comparaison contrôlée, choisissez explicitement `cpu`, `wgpu` ou `cuda`. Si ce choix est indisponible, le capteur ne s'initialise pas. Voir le [tableau des choix](sonar.md#compute-backend).

## Compiler les paquets sonar {#build-the-sonar-packages}

Partez d'une [installation depuis les sources](install.md) avec ses dépendances. `wgpu_vendor` compile la bibliothèque Rust, même pour utiliser CPU. Vérifiez Rust et Cargo ; au besoin, suivez leur [installation officielle](https://www.rust-lang.org/tools/install).

{{CODE:sonar-tuning:0}}

Dans l'espace de travail dont l'environnement est chargé, compilez la bibliothèque, le capteur, le système du monde et l'exemple :

{{CODE:sonar-tuning:1}}

CUDA est facultatif. Pour ignorer un toolkit installé et compiler CPU/WGPU, ajoutez `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`. `Cargo.lock` fixe les dépendances Rust ; leur premier téléchargement nécessite Internet.

{{CODE:sonar-tuning:2}}

Vérifiez les quatre paquets, puis lancez l'[exemple](sonar.md#run-a-sonar-example) et recevez images et données brutes. La présence d'un paquet ne prouve pas que le capteur se charge. Vérifiez les paquets dans le conteneur ; s'ils manquent, compilez les sources sans supposer qu'une étiquette d'image les inclut.

## Choisir l'adaptateur WGPU

Sous Linux, exécutez `vulkaninfo --summary` dans l'environnement de Gazebo, avec le pilote et les outils Vulkan du système. Un adaptateur logiciel tel que llvmpipe peut exécuter Vulkan sans accélération matérielle. WGPU affiche l'adaptateur retenu au démarrage.

{{CODE:sonar-tuning:3}}

| Variable | Usage |
| --- | --- |
| `WGPU_BACKEND` | `vulkan` sous Linux, `metal` dans un processus macOS natif ; omettez-la pour les valeurs par défaut de la plateforme |
| `WGPU_ADAPTER_NAME` | Premier adaptateur dont le nom contient le texte, sans distinction de casse ; reprenez un nom affiché par le pilote ou au démarrage |
| `WGPU_POWER_PREF` | `high`, `low` ou `none` ; `high` par défaut si aucun nom n'est donné |

Définissez les variables avant le lancement. Arrêtez puis relancez la simulation après modification : l'initialisation est partagée dans le processus. Le projet [wgpu](https://github.com/gfx-rs/wgpu) décrit les API. En cas d'échec d'initialisation ou de contrôle des ressources, examinez l'adaptateur et ses limites, puis choisissez `cpu` ou réduisez les dimensions du capteur.

## CUDA, en option

Il faut un pilote NVIDIA, le compilateur/runtime CUDA, cuFFT et cuBLAS. Vérifiez l'environnement d'exécution :

{{CODE:sonar-tuning:4}}

Pour les conteneurs Linux, configurez [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) sur l'hôte. Sous Windows, suivez [CUDA sous WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) et, si nécessaire, la [configuration GPU de Docker Desktop](https://docs.docker.com/desktop/features/gpu/). Installez le pilote d'affichage Windows, pas un pilote Linux dans WSL. Le conteneur doit aussi disposer des bibliothèques CUDA.

Choisissez une architecture compatible. L'exemple utilise SM 75 : remplacez `75` par la valeur de votre GPU. `OFF` réactive la recherche du toolkit après une compilation CPU/WGPU seule.

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` indique les cibles disponibles. Sans choix explicite, le compilateur fournit sa valeur par défaut. Avec plusieurs toolkits, utilisez `-DCUDAToolkit_ROOT=/path/to/cuda` et ajoutez son `bin` au `PATH`. Lancez l'exemple avec `compute_backend:=cuda`.

## Mesurer une référence

Gardez la scène, la distance à la cible et la pose du capteur fixes. Dans le terminal B :

{{CODE:sonar-tuning:6}}

Arrêtez avec Ctrl+C, puis examinez la vitesse de simulation :

{{CODE:sonar-tuning:7}}

L'entité du monde s'appelle `default`. Relevez `real_time_factor` avec la fréquence reçue : demander 30 Hz en temps simulé ne garantit pas 30 images par seconde réelle. Quittez avec Ctrl+C.

## Régler l'échantillonnage et la portée

Modifiez un seul paramètre dans une copie du [SDF](sonar.md#sensor-configuration), recompilez, rechargez l'environnement et relancez avec la même pose.

| Expérience | Comparaison | Observation |
|---|---|---|
| Détail angulaire | 512 contre 256 faisceaux horizontaux, champ constant | Séparation des cibles et fréquence |
| Échantillonnage vertical | 300 contre 150 rayons | Continuité des cibles et temps de calcul |
| Couverture | Modifier ensemble portée des rayons et `maxDistance` | Cible la plus éloignée et échelle de distance |
| Actualisation | 30 contre 10 Hz | Fréquence reçue et facteur de temps réel |
| Saut de rayons | `raySkips` à 10 puis 5 | Détail vertical et coût de calcul |

Le pas de distance vaut `soundSpeed / (2 × bandwidth)` ; la grille compte `ceil(maxDistance / pas) + 1` cellules. Avec 1500 m/s, 29.9 kHz et 15 m, on obtient 599 cellules. La dernière coordonnée peut dépasser légèrement `maxDistance`, mais les échos hors portée sont exclus. Lisez les distances et le nombre de faisceaux dans le message brut ou le CSV.

WGPU conserve la longueur demandée, même hors des puissances de deux, et choisit automatiquement les étapes FFT. Portée et bande passante plus grandes augmentent mémoire et calcul. Dans le mode de grille par défaut, CPU place les échos dans la cellule de distance la plus proche ; GPU travaille dans le domaine fréquentiel. Comparez aussi l'amplitude et la réponse en distance entre moteurs.

## Conserver un essai reproductible

Laissez le rendu se stabiliser. Désactivez `writeLog` et `debugFlag` pour mesurer le débit ; enregistrez séparément en [CSV](sonar.md) ou avec [ROS](visualization.md). `blazingSonarImage=false` fixe la graine à 1234.

Conservez SDF, révision POSIM, moteur/adaptateur, versions pilote/toolkit, temps par image, fréquence reçue, facteur de temps réel et mémoire. Comparez les données brutes d'intensité/distance en plus des captures. Voir les [références du modèle](citation.md).

## Traitement GPU

{{MEDIA:sonar-tuning}}

Le traitement comprend somme des rayons, fenêtrage/correction et FFT. Mesurez séparément calcul et transferts mémoire. Figure : POSIM Notion Wiki.

Avec le SDK macOS de Gazebo Sim 10.5 testé, les mondes dont le sonar est le seul capteur peuvent planter à l’initialisation d’Ogre2. Avec ce SDK, ajoutez une petite caméra intégrée au monde.

## Transport ROS sous macOS natif {#native-macos-ros-transport}

Avec Fast DDS sous macOS natif, utilisez le profil local installé si de grands messages sonar best-effort ne sont pas reçus. Il prévoit un segment de mémoire partagée de 16 MiB et des tampons de messages de transport de 1 MiB. UDP sur localhost utilise des datagrammes de 8192 octets et des tampons de socket d’émission et de réception de 2 MiB chacun. Définissez cette variable dans chaque terminal du simulateur, du pont et de l’abonné avant de lancer ces processus :

{{CODE:sonar-tuning:8}}

Ce profil est facultatif et limité aux processus d’un même Mac. Pour communiquer avec d’autres ordinateurs, utilisez un profil réseau adapté. Si `FASTDDS_DEFAULT_PROFILES_FILE` est déjà défini, intégrez les paramètres de mémoire partagée à ce profil. Ces paramètres de transport ROS s’appliquent aux sonars CPU et WGPU.
