# Funzionalità ed esempi

POSIM offre sistemi Gazebo, modelli di robot e interfacce ROS. L'SDF del mondo e i file di avvio selezionano i sistemi usati dalla simulazione.

## Ambiente e rendering

| Funzione | Implementazione | Guida |
|---|---|---|
| Foschia subacquea, assorbimento RGB e attenuazione della luce solare | `gazebo/posim_gz_rendering` | [Effetti subacquei](underwater-rendering.md) |
| Caustiche sul fondale e particelle in sospensione | `gazebo/posim_gz_rendering` | [Attivare gli effetti](underwater-rendering.md) |
| Texture del fondale, griglia e curve di profondità | `gazebo/posim_gz_rendering`; mappe del terreno | [Materiali e distanze](underwater-rendering.md) |
| Onde spettrali e idrodinamica del WAM-V | Wave Sim esterno; integrazione `extras/surface` | [Veicoli di superficie](surface.md) |
| Correnti uniformi, dipendenti dalla profondità e di marea | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [Correnti](currents.md) |
| Origine geografica e conversione delle coordinate | `posim_ros_gz_plugins` | [Coordinate sferiche](coordinates.md) |
| Fondali, oggetti e scenari operativi | `models/posim_worlds`, `posim_object_models` | [Mondi](worlds.md), [oggetti](objects.md), [terreni](heightmaps.md) |

Gli effetti ottici subacquei modificano le superfici PBR e conservano i valori numerici di profondità. La superficie oceanica usa un materiale proprio. La neve marina è un'approssimazione visiva procedurale, inizialmente disattivata. La [guida al rendering](underwater-rendering.md) descrive l'ipotesi sul livello del mare e il comportamento delle telecamere.

## Sensori, robot e controllo

| Funzione | Implementazione | Guida |
|---|---|---|
| GNSS marino, perdita in immersione, riacquisizione e latenza | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| Attenuazione delle immagini subacquee basata su RGBD | `posim_gz_sensor_plugins/UnderwaterCamera` | [Telecamera](camera.md) |
| Misure DVL e conversione ROS | DVL Gazebo; `posim_ros_gz_plugins` | [DVL](dvl.md) |
| Pressione idrostatica e profondità calcolata | `posim_gz_sensor_plugins` | [Pressione](pressure.md) |
| Interrogazione acustica e posizionamento USBL | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| Sonar multibeam CUDA | `gazebo/posim_gz_multibeam_sonar` | [Sonar](sonar.md), [configurazione e prestazioni](sonar-tuning.md) |
| REXROV, varianti BlueROV2 e aliante Slocum | `models/posim_robot_models` | [ROV](rovs.md), [aliante](gliders.md), [aggiungere un robot](custom-robots.md) |
| WAM-V con ArduRover e controllo ROS | `extras/surface`, configurazioni di avvio, `posim_interfaces` | [Veicolo di superficie](surface.md), [ROS 2](ros.md) |

I sensori disponibili dipendono dai plugin installati e dal modello. I profili GNSS descrivono i montaggi: durante l'integrazione aggiungi il link e il ricevitore. Il sonar CUDA richiede GPU e strumenti compatibili. Usa l'avvio rapido di ciascun riferimento per verificare le misure durante la simulazione.

## Directory del codice sorgente

I percorsi sono relativi al repository POSIM.

| Directory | Contenuto |
|---|---|
| `models/posim_worlds` | Mondi e risorse ambientali |
| `models/posim_robot_models` | Meccanica, sensori e configurazione ROS di ogni robot |
| `models/posim_sensor_models` | Descrizioni ed esempi di sensori indipendenti |
| `gazebo/` | Plugin dell'ambiente marino e dei sensori |
| `posim_interfaces` | Definizioni di messaggi e servizi POSIM |
| `examples/posim_demos` | File di avvio che collegano mondi, modelli e ROS |

[Prima simulazione](quickstart.md) avvia questi componenti insieme. Per modificarli, consulta [Aggiungere un robot](custom-robots.md) e [Creare un mondo](custom-worlds.md).

## Libreria Ogre alternativa per macOS

Lo [strumento di compilazione Ogre](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) genera un core Ogre isolato per macOS arm64 con Ogre-Next 2.3.3. Segui il profilo SDK supportato e i controlli sulle librerie caricate indicati nella guida.

Consulta le [licenze dei componenti](citation.md).
