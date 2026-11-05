# Correnti marine

POSIM genera le correnti del mondo, calcola il flusso in corrispondenza di ogni robot e offre servizi ROS per regolarlo. La corrente uniforme e il profilo che varia con la profondità usano topic e impostazioni distinti.

## Avviare l'esempio

Prepara i [terminali A e B](terminals.md) nel tuo ambiente POSIM: Ubuntu nativo, WSL, Docker o il desktop remoto di macOS. Avvia l'esempio in A:

{{CODE:currents:0}}

In B, controlla prima Gazebo e poi ROS. Premi Ctrl+C per interrompere la prima visualizzazione prima di eseguire il secondo comando:

{{CODE:currents:1}}

{{CODE:currents:2}}

## Configurare la corrente del mondo

Inserisci entrambi i plugin direttamente in `<world>`, quello della corrente prima di quello ROS. Il [mondo di esempio](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world) mostra la configurazione completa.

{{CODE:currents:3}}

Usa lo stesso `namespace`, non vuoto, nei due plugin. Sono necessari entrambi i blocchi `constant_current` e `transient_current`: se manca una configurazione obbligatoria, la pubblicazione viene disattivata con un errore. La corrente uniforme usa modelli di Gauss–Markov separati per velocità e angoli orizzontale e verticale.

| Parametro | Funzione e valore predefinito |
| --- | --- |
| `namespace` dei due plugin | Namespace condiviso; `hydrodynamics` nell'esempio |
| `constant_current/use_constant_current` | Pubblicare la corrente uniforme; `false` |
| `constant_current/topic` | Topic Gazebo della corrente uniforme; `ocean_current` |
| `transient_current/topic_stratified` | Topic Gazebo degli strati nel namespace; `stratified_current_velocity` |
| `transient_current/databasefileName` | CSV installato in `posim_worlds/worlds`; `transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | Percorso CSV facoltativo, con precedenza sul nome del file |
| `mean` di ogni modello stocastico | Media iniziale della velocità (m/s) o dell'angolo (rad); 0 |
| `min`, `max` | Limiti; −1 e 1. Mantieni `min < max` e la media nell'intervallo |
| `noiseAmp` | Ampiezza dell'incremento casuale; 0 |
| `mu` | Coefficiente di decadimento di Gauss–Markov; 0 |
| `tidal_oscillation` | Blocco facoltativo dei dati di marea |

## Preparare un profilo di profondità

Il [CSV di esempio](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv) contiene, nell'ordine, velocità verso nord (m/s), velocità verso est (m/s) e profondità (m).

- Conserva le prime tre righe descrittive e di intestazione.
- Inserisci almeno due strati, con esattamente tre numeri finiti per riga.
- Le profondità devono essere non negative e strettamente crescenti.
- Un profilo assente o non valido disattiva la pubblicazione della corrente.

Il plugin ROS del mondo trasmette l'intero database. Il plugin del modello interpola la corrente alla profondità del veicolo. La risposta idrodinamica dipende dal flusso relativo alla velocità del robot.

## Applicare il profilo al robot

L'esempio REXROV usa inizialmente la corrente uniforme `/ocean_current`. Per usare il profilo, attiva una sola volta il blocco commentato nel [modello REXROV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf), direttamente in `<model>`:

{{CODE:currents:4}}

Aggiungi anche `<namespace>rexrov</namespace>` al plugin esistente `gz-sim-hydrodynamics-system`, conservando link, densità e coefficienti di resistenza. Ricompila il workspace e caricane l'ambiente.

| Parametro del modello | Funzione e valore predefinito |
| --- | --- |
| `namespace` | Obbligatorio; deve coincidere con quello dell'idrodinamica, qui `rexrov` |
| `flow_velocity_topic` | Suffisso sotto `/model/<namespace>/`; `ocean_current` |
| `transient_current/topic_stratified` | Ingresso ROS del database; `hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | Impostazioni stocastiche facoltative per componente; le componenti omesse non aggiungono rumore |
| `mean` di ogni componente | Velocità iniziale, m/s; 0. Sostituita dal profilo interpolato quando arrivano i dati |
| `mu` | Decadimento da 0 a 1, applicato in base ai secondi di simulazione trascorsi; 0 |
| `noiseAmp` | Scostamento ammesso dalla velocità interpolata, m/s; 0 |
| `noiseFreq` | Ampiezza dell'incremento casuale per aggiornamento; 0. Non è una frequenza in Hz |
| `tide_oscillation` | Applicare al modello i dati di marea forniti; `false` |

Il modello pubblica `/model/rexrov/ocean_current` in Gazebo (`gz.msgs.Vector3d`) e ROS (`geometry_msgs/msg/TwistStamped`). Controlla sia il dato sia la risposta del veicolo.

La profondità deriva dalla posizione del modello nel mondo; sopra la superficie vale zero. La corrente viene interpolata tra gli strati e assume il valore dello strato più vicino fuori dall'intervallo. Una pausa o un timestamp ripetuto non fa avanzare il calcolo. Un messaggio di database non valido viene rifiutato interamente, mantenendo l'ultimo profilo valido.

## Sistemi e topic

| Sistema | Compito |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | Generare la corrente del mondo e i dati per profondità |
| Plugin di corrente del modello | Interpolare la corrente alla posizione del robot |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | Pubblicare dati ROS e offrire servizi di configurazione |

| Topic dell'esempio | Trasporto e tipo |
| --- | --- |
| `/ocean_current` | Gazebo, `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo, dati degli strati |
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS, `posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS, `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS, `posim_interfaces/msg/StratifiedCurrentDatabase` |

Verifica i nomi nel SDF e collega l'idrodinamica alla sorgente di corrente prevista.

## Regolare la corrente con ROS

In B, elenca i servizi e richiedi una corrente orizzontale di 0.3 m/s:

{{CODE:currents:5}}

La velocità è in m/s e gli angoli in radianti. Controlla `success` e i dati pubblicati. Per leggere l'interfaccia di uno strato:

{{CODE:currents:6}}

Leggi il modello, modifica i suoi limiti e regola il primo strato:

{{CODE:currents:7}}

| Servizio sotto `/hydrodynamics` | Interfaccia | Campi |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | Richiesta vuota |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

### Vincoli delle richieste

- Una modifica viene applicata interamente oppure restituisce `success: false` senza cambiare la corrente.
- I valori devono essere finiti e rispettare i limiti configurati. I limiti di velocità sono non negativi; quelli angolari rimangono entro ±π in orizzontale e ±π/2 in verticale.
- Le impostazioni valgono per il modello globale e per tutti gli strati, conservandone le medie. Anche queste medie devono rientrare nei nuovi limiti.
- `layer` parte da 0. Il campo ROS `noise` corrisponde a `noiseAmp` nel SDF.

## Usare dati di marea

Configura `<tidal_oscillation>` con un CSV o costituenti armoniche, come M2/S2/N2, le direzioni di flusso e riflusso e un istante iniziale GMT. Negli esempi armonici, l'ampiezza è in metri, la fase in gradi e la velocità angolare in gradi/ora. Queste unità sono diverse dai radianti dei servizi della corrente.

Il CSV contiene una riga di intestazione, poi righe `YYYY-MM-DD HH:MM,event,speed`:

- Eventi: `flood`, `ebb` o `slack`. La velocità deve essere finita e in cm/s; `-` è ammesso nelle righe `slack` che vengono saltate.
- Le date devono essere valide e strettamente crescenti.
- Il lettore conserva picchi positivi e negativi alternati, almeno due. Fuori dall'intervallo temporale mantiene il primo o l'ultimo picco.
- File mancanti, impostazioni incomplete o dati malformati disattivano il plugin di corrente del mondo.

Questi dati sono ingressi dell'esperimento. Conserva unità, intervallo di interpolazione, origine del mondo e riferimento temporale. Confronta corrente e movimento del veicolo a partire dalla stessa posa iniziale.

## Flusso e risposta del veicolo

{{MEDIA:currents}}

Immagini e video: POSIM Notion Wiki. Consulta [Citazioni e licenze](citation.md) per l'attribuzione di DAVE. Usa i comandi e i topic di questa pagina.
