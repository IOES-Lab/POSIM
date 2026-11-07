# Aggiungere e configurare sensori

Un sensore richiede la definizione nel modello, il sistema che lo aggiorna e un collegamento all'applicazione ricevente. Aggiungine uno alla volta e verifica le misure.

## Ambiente di lavoro

Usa i [terminali Linux preparati](terminals.md) su Ubuntu nativo, WSL o Docker. Su macOS lavora nel desktop RDP. Modifica e compila in quell'ambiente; `POSIM_WS` ne indica lo spazio di lavoro.

## Scegliere il punto d'integrazione

| Misura | Posizione nel modello | Sistema del mondo / collegamento ROS |
|---|---|---|
| IMU | `link/sensor type="imu"` | Sistema IMU; bridge da `gz.msgs.IMU` a ROS `Imu` |
| [Telecamera subacquea](camera.md) | Sensore RGBD e plugin degli effetti | Sistema Sensors con renderer e topic della telecamera |
| [DVL](dvl.md) | Sensore DVL e configurazione dei fasci | Sistema DVL e bridge dei messaggi acustici incluso |
| [Pressione](pressure.md) | Plugin direttamente sotto `model` | Pubblicazione ROS diretta di pressione e profondità |
| [GNSS marino](gnss.md) | Plugin del ricevitore sul modello | Uscite ROS configurate del ricevitore |
| [USBL](usbl.md) | Plugin di ricetrasmettitore e transponder | Identificativi acustici corrispondenti e interfacce ROS |
| [Sonar multibeam](sonar.md) | Sensore personalizzato sotto un link | Renderer Sensors, sistema multibeam del mondo e CUDA |

Consulta l'SDF e la tabella dei parametri di ogni riferimento. Rispetta la posizione: i sistemi del mondo vanno nel mondo, mentre i plugin di modello richiedono l'entità su cui agiscono.

## Aggiungere un'IMU al mondo di esempio

Apri `calm_water.sdf` da [Creare un mondo](custom-worlds.md). Aggiungi questo sistema direttamente in `<world>`, accanto al sistema fisico:

{{CODE:custom-sensors:0}}

Dentro `<link name="base_link">` del modello `pressure_station`, aggiungi:

{{CODE:custom-sensors:1}}

La posa è relativa al link, in metri e radianti. La frequenza usa il tempo simulato. La stazione è fissa: la velocità angolare dovrebbe restare vicina a zero, mentre l'accelerazione include la risposta alla gravità.

## Collegare e osservare

Ferma il mondo precedente. Nel terminale A, valida e riavvia:

{{CODE:custom-sensors:2}}

Nel **terminale B**, con l'ambiente caricato, avvia il bridge e lascialo in esecuzione:

{{CODE:custom-sensors:3}}

Nel **terminale C**, preparato nello stesso modo, leggi una misura:

{{CODE:custom-sensors:4}}

Controlla timestamp, identificativo del sistema di riferimento, velocità angolare e accelerazione. Togli `--once` per leggere continuamente; Ctrl+C ferma la visualizzazione. Alla fine arresta bridge e simulatore. [Terminali e topic ROS](ros.md) spiega come condividono l'ambiente.

## Cambiare la frequenza

Modifica `update_rate` da `20` a `5` e riavvia mondo e bridge. Gli intervalli tra timestamp dovrebbero passare da circa **0,05 s a 0,2 s** di tempo simulato. La frequenza ricevuta nel tempo reale dipende anche dal fattore di tempo reale e dal trasporto.

Per le telecamere, controlla insieme risoluzione, campo visivo e limiti vicino e lontano. Per i sensori a scansione, verifica fasci e portata. Prima di spostare un sensore, conferma sistema di riferimento e orientamento.

## Verificare l'integrazione

- Modello: posa valida, impostazioni e topic univoco.
- Mondo: sistema necessario e renderer per i sensori d'immagine.
- ROS: direzione del bridge, tipo di messaggio, riferimento e QoS.
- Dati: timestamp recenti e risposta a una modifica controllata.

Verifica il fondale con [Terreni](heightmaps.md). Per confrontare configurazioni sonar, usa [Configurazione e prestazioni](sonar-tuning.md) insieme al [riferimento dei parametri](sonar.md).
