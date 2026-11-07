# Veicoli di superficie e onde

Simula un WAM-V con le onde di Wave Sim e ArduRover in modalità imbarcazione. L'installatore POSIM e le ricette Docker compilano le librerie esterne per le onde.

## Preparare il WAM-V

Questa immagine opzionale usa la struttura desktop ARM64. Su un Mac Apple Silicon, prepara [Docker e RDP](install-macos.md), poi esegui i comandi nel Terminale macOS da una copia del repository POSIM:

{{CODE:surface:0}}

Collega il client RDP a **127.0.0.1:3390** con l'account `docker`. Apri un terminale nel desktop Linux:

{{CODE:surface:1}}

L'immagine contiene eseguibili separati di ArduSub e ArduRover. L'esempio seleziona ArduRover con `FRAME_CLASS=2`. Esegui **una sola istanza per ambiente**: nome del veicolo, file generati e porte del controllore sono fissi. Più veicoli richiedono file, nomi, identificativi e porte distinti, oppure ambienti isolati.

Il controllore attende la creazione del veicolo e i dati della sua IMU. Se la creazione fallisce o supera 180 s, oppure non arriva una misura IMU nei successivi 180 s, l'avvio termina con errore indicando la fase. Controlla i messaggi Gazebo e del sensore prima di riprovare.

## Modello, controllo e navigazione

`extras/surface/wamv.py` prepara il modello esterno. Le eliche di poppa usano le uscite 1 e 3; il veicolo parte in superficie.

L'adattatore di navigazione esterna invia a MAVROS **posa e velocità simulate da Gazebo**. Verifica stato del controllore, validità della navigazione e direzione della spinta prima di inviare una missione.

Per studiare rumore, contatto con l'acqua e riacquisizione del ricevitore, integra il [GNSS marino](gnss.md), l'antenna e un adattatore di navigazione adatto. L'adattatore di posa esterna di questo esempio è un ingresso distinto.

## Regolare le onde

I comandi usano `gz.msgs.Param` su `/world/<world_name>/waves`. Il nome deriva da `<world name="…">`. Elenca i topic attivi:

{{CODE:surface:2}}

Il [file di avvio WAM-V](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py) usa normalmente `posim_surface`, quindi `/world/posim_surface/waves`. Nel contenitore preparato puoi cambiare nome:

{{CODE:surface:3}}

Il topic diventa `/world/harbour/waves`; l'IMU ROS resta su `/model/wamv/imu`. `world_name` assegna il nome al mondo generato: non seleziona un terreno né isola una seconda istanza.

Il fondale è piano, a z = −10 m. Per modificarlo, consulta [Terreni da mappe di elevazione](heightmaps.md).

| Parametro (`double`) | Unità | Funzione |
|---|---|---|
| `wind_speed` | m/s | Velocità del vento nel modello delle onde |
| `wind_angle` | gradi | Direzione del vento |
| `steepness` | adimensionale | Ripidità delle onde |

Mantieni coerenti le impostazioni visive e idrodinamiche. Le [correnti](currents.md) si configurano separatamente. Il mondo deve includere e configurare i sistemi delle onde, oltre ad avere le librerie installate.

## Limiti e dipendenze

Il Dockerfile opzionale supporta la struttura ARM64. Calibra scafo, propulsione e resistenza per l'imbarcazione e le condizioni d'uso. Alla chiusura, MAVROS o Gazebo possono richiedere la terminazione da parte del supervisore; verifica che entrambi siano usciti prima di riavviare.

`extras/surface/dependency.json` fissa la revisione esterna. L'installatore compila in `/opt/waves` e conserva i sorgenti in `/opt/asv_wave_sim`: è una dipendenza di compilazione, non un sottomodulo Git. Vedi la [guida all'integrazione](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) e le [licenze](citation.md).
