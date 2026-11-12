# Sonar multibeam

Il sonar multibeam di POSIM usa un modello a diffusori puntiformi per generare dati di intensità e distanza e immagini acustiche. Lo stesso plugin offre il calcolo su CPU, WGPU e CUDA. WGPU usa Vulkan su Linux o Metal in un ambiente macOS nativo per lavorare con GPU di produttori diversi. Consulta la [configurazione del calcolo](sonar-tuning.md).

## Avviare un esempio {#run-a-sonar-example}

Prepara i [terminali A e B](terminals.md) e, se mancano, [compila i pacchetti del sonar](sonar-tuning.md#build-the-sonar-packages). Avvia la simulazione in A e osserva i dati in B. Il primo comando sceglie automaticamente il backend; l'esempio dedicato seleziona WGPU e apre anche RViz.

{{CODE:sonar:0}}

Il pacchetto `posim_multibeam_sonar_demo` offre anche un esempio autonomo:

{{CODE:sonar:1}}

{{MEDIA:sonar:0}}

Esegui un solo esempio alla volta. Sostituisci `wgpu` con `cpu`, `cuda` o `auto` per cambiare backend. Per il sonar a bordo, scegli `bluerov2_heavy_multibeam_sonar` con un mondo adatto.

## Come funziona il sensore

Gazebo fornisce la geometria della scena. Il modello stima le normali delle superfici, calcola la retrodiffusione acustica, applica la risposta di ricezione e comprime gli echi in celle di distanza. Dati acustici e immagine rappresentano la stessa osservazione. Usa i dati grezzi per l'analisi e conserva le impostazioni di guadagno e visualizzazione.

Un valore positivo di `spec/receiveBeamWidth` attiva la modalità fisica, che usa le coordinate XYZ misurate da una camera di profondità che copre tutto il campo del sonar. Campioni geometrici, direzioni riceventi e uscite in distanza sono indipendenti. Le celle con centro nel campo ne ripartiscono l'intero angolo solido; le loro aree di superficie proiettate e la risposta ricevente con segno determinano i contributi ai fasci. Il modello usa un impulso Hamming in banda base, con larghezza di banda fissa e picco unitario. L'intervallo di calcolo interno è più lungo dell'intervallo di distanza pubblicato. Il passo di uscita è `soundSpeed / (2 × bandwidth)`; la lunghezza di calcolo non determina il guadagno dell'impulso. L'intervallo di guardia interno non viene pubblicato e non cambia questo passo.

`ray/scan/horizontal/samples` definisce le colonne geometriche, `ray/scan/vertical/rays` le righe dell'immagine e `beams` le direzioni riceventi. La modalità fisica richiede un campo simmetrico inferiore a 180° su ogni asse. I limiti di distanza sono radiali; i pixel fuori dal campo o dai limiti hanno XYZ non valide nella nuvola organizzata. `spec/receiveBeamWidth` indica la larghezza totale a metà potenza in radianti; `spec/receiveGain` imposta il guadagno lineare di pressione (predefinito: 1). Una larghezza omessa o nulla seleziona la modalità a griglia predefinita. La modalità fisica supporta CPU e WGPU, ma non CUDA. Non esiste un limite fisso di 4096 campioni di frequenza; le griglie grandi restano soggette alle risorse del motore e alle dimensioni rappresentabili. I coefficienti di diffusione sono generati per immagine e cella geometrica. Il modello descrive echi in banda base, senza propagazione della fase portante né calibrazione dell'ampiezza dell'hardware.

## Controllare i dati

Nel terminale B:

{{CODE:sonar:2}}

- Le immagini usano QoS `reliable`: imposta la stessa affidabilità nella vista Image di RViz.
- Aggiungi PointCloud2 per `/sensor/multibeam_sonar/point_cloud`.
- Gli echi grezzi usano `marine_acoustic_msgs/msg/ProjectedSonarImage`.
- L'avvio del sensore BlueView collega l'orologio di Gazebo a ROS `/clock`. Per i nodi con `use_sim_time`, verifica che la simulazione sia in esecuzione e che l'orologio avanzi.

## Configurazione del sensore {#sensor-configuration}

Inserisci questi sistemi direttamente in `<world>`:

{{CODE:sonar:3}}

Inserisci il sensore P900 in un `<link>` del modello:

{{CODE:sonar:4}}

L'esempio richiede 512 fasci orizzontali su circa 130°, 300 raggi verticali su circa 12° e una portata di 0.1–10 m. Usa 900 kHz, una larghezza di banda di 29.9 kHz e una velocità del suono di 1500 m/s. I 30 Hz impostati sono la frequenza richiesta: quella ricevuta dipende dal calcolo e dal rendering. Consulta il [modello completo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf).

## Parametri di `<spec>`

| Parametro | Funzione | Valore predefinito | Esempio P900 |
| --- | --- | --- | --- |
| `verticalFOV` | Apertura acustica verticale, gradi | 10 | 12 |
| `sonarFreq` | Frequenza acustica, Hz | 900000 | 900000 |
| `bandwidth` | Larghezza di banda, Hz | 29500000 | 29900 |
| `soundSpeed` | Velocità del suono, m/s | 1500 | 1500 |
| `sourceLevel` | Livello della sorgente, dB | 220 | 220 |
| `maxDistance` | Distanza di elaborazione, m; da allineare alla portata dei raggi | 60 | 10 |
| `raySkips` | Passo di campionamento verticale; intero positivo | 10 | 10 |
| `sensorGain` | Guadagno di ampiezza degli echi nella griglia CPU predefinita; non usato in modalità fisica né da WGPU/CUDA | 0.02 | 0.02 |
| `blazingSonarImage` | Seme casuale basato sull'orologio reale; con `false` è fisso a 1234 | false | true |
| `pointCloudTopicName` | Suffisso del topic Gazebo della nuvola di punti | `point_cloud` | Uguale |
| `sonarImageRawTopicName` | Suffisso del topic ROS degli echi grezzi | `sonar_image_raw` | Uguale |
| `sonarImageTopicName` | Suffisso del topic ROS dell'immagine | `sonar_image` | Uguale |
| `frameName` | Identificatore del sistema di riferimento ottico | `forward_sonar_optical_link` | Uguale |
| `writeLog` | Scrivere CSV in una cartella distinta per ogni esecuzione | false | false |
| `writeFrameInterval` | Fotogrammi tra due scritture; intero positivo | 10 | 5 |
| `debugFlag` | Messaggi diagnostici nella console | false | false |
| `plotScaler` | Letto e stampato, ma non usato nella generazione dell'immagine | 10 | Non specificato |

### Topic e sistemi di riferimento

I suffissi si aggiungono al `<topic>` del sensore, qui `/sensor/multibeam_sonar`. La nuvola di punti Gazebo usa `gz.msgs.PointCloudPacked`; il bridge ROS incluso la rende disponibile come `sensor_msgs/msg/PointCloud2`. Allinea `pointCloudTopicName` a `sensor_config.py` e imposta il riferimento ottico del sonar in `frameName`.

I file di avvio del BlueView P900 e del sonar BlueROV2 pubblicano una trasformazione identità da `world` al sistema di riferimento del sonar per la visualizzazione in RViz. Le coordinate della nuvola di punti restano relative al sensore. Questa trasformazione statica non segue il movimento del veicolo e non tiene conto della posizione e dell'orientamento di montaggio del sonar. Per creare mappe in coordinate globali, sostituiscila con una catena di trasformazioni che includa entrambe le pose.

### Geometria dei raggi

I valori predefiniti di `<ray>` sono 256 fasci orizzontali, 3 raggi verticali, angoli orizzontali ±π/4, verticali ±π/8 e portata 0.1–5 m. Gli angoli sono in radianti; con `degrees="true"` si usano i gradi. Il `<reference_frame>` facoltativo in `<gz:multibeam_sonar>` ha posizione e orientamento nulli per impostazione predefinita.

Mantieni completi i blocchi `<ray>` e `<spec>`: i valori predefiniti si applicano ai singoli campi. La portata dei raggi e `maxDistance` devono essere coerenti. L'[implementazione](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc) definisce la lettura dei parametri e la pubblicazione dei dati.

### Confrontare le misure

Disattiva la registrazione quando misuri le prestazioni, a meno che faccia parte dell'esperimento. Segui la guida [Configurare e regolare il sonar](sonar-tuning.md) e conserva i parametri acustici e `blazingSonarImage` per confrontare le esecuzioni.

### Backend di calcolo {#compute-backend}

| Scelta | Comportamento |
| --- | --- |
| `auto` | Prova CUDA, poi WGPU e infine CPU; scelta predefinita |
| `wgpu` | Usa un adattatore Vulkan/Metal compatibile; se manca, il sensore non si inizializza |
| `cuda` | Richiede GPU NVIDIA e backend CUDA compilato e installato; l'inizializzazione fallisce se non sono disponibili |
| `cpu` | Calcola gli echi sulla CPU; il rendering di Gazebo resta necessario |

L'esempio dedicato accetta `compute_backend`; gli altri avvii leggono `POSIM_SONAR_COMPUTE_BACKEND`. Controlla la riga `Using sonar compute backend:` all'avvio. La guida [Configurare il sonar](sonar-tuning.md) spiega la scelta dell'adattatore, i requisiti e le differenze numeriche.

## Aggiungere un sensore

1. Copia `models/posim_sensor_models/description/blueview_p900/` con un nuovo identificatore.
2. Prepara il relativo `config/<identificatore>/sensor_config.py`. Allinea nomi del modello, link, sistemi di riferimento e topic del bridge.
3. Imposta campionamento, angoli e portata. Carica nel mondo i due sistemi mostrati sopra; consulta il [mondo sonar in SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world) per le altre impostazioni.
4. Prepara il backend di calcolo e il rendering, ricompila il workspace e caricane l'ambiente. Seleziona il sensore tramite gli [argomenti di avvio](worlds.md).

## Registrare e visualizzare i dati grezzi

Con `writeLog=true`, ogni sensore crea `SonarRawData_<run>_<suffix>/` nella cartella di lavoro del simulatore e ne stampa il percorso. Vi salva CSV numerati e `SonarRawData_beam_angles.csv`, conservando le registrazioni precedenti. Gli intervalli inferiori a 1 vengono portati a 1. Se la cartella non può essere creata, la registrazione viene disattivata con un messaggio di errore.

Dalla cartella indicata, apri il primo fotogramma:

{{CODE:sonar:6}}

Per salvare i grafici in PNG:

{{CODE:sonar:7}}

Lo [strumento di visualizzazione](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py) si installa anche senza CUDA. Produce una vista sonar e cinque profili di fasci scelti a intervalli regolari, oppure tutti i profili se i fasci sono meno di cinque.

| Opzione | Effetto |
| --- | --- |
| `--range-max` | Distanza massima visualizzata, m |
| `--beam-step` | Mostrare un fascio ogni N |
| `--dynamic-range` | Intervallo sotto il massimo, dB; valore predefinito: 60 |

I colori sono relativi al massimo di ciascun fotogramma: usa i valori CSV per confrontare esecuzioni diverse. La prima colonna contiene le distanze, le altre gli echi complessi dei fasci. Conserva il file degli angoli insieme ai dati e controlla lo spazio occupato su disco.

## Scene e osservazioni

{{MEDIA:sonar:1}}
{{MEDIA:sonar:2}}
{{MEDIA:sonar:3}}
{{MEDIA:sonar:4}}
{{MEDIA:sonar:5}}
{{MEDIA:sonar:6}}

Immagini e video: POSIM Notion Wiki. Consulta [Citazioni e licenze](citation.md) per l'attribuzione di DAVE e del modello sonar. Usa i comandi e i topic riportati nei blocchi di questa pagina.
