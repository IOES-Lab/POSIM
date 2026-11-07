# Risoluzione dei problemi

Parti dal primo errore nel log di avvio. Controlla nell'ordine ambiente, risorse del modello, tempo di simulazione, plugin e ricezione dei dati. Conserva l'errore e le righe vicine utili a riprodurlo.

## Un pacchetto non viene trovato

{{CODE:troubleshooting:0}}

Il risultato deve mostrare `lyrical` e il percorso del workspace scelto. Prepara il [terminale](terminals.md) secondo il tuo [metodo di installazione](requirements.md) e distingui i percorsi dell'host da quelli del container.

## Mancano modelli o mesh

Individua il primo URI non risolto. Verifica che le risorse del pacchetto siano installate e che l'ambiente corretto sia stato caricato. Gazebo deve poter accedere alla rete per scaricare risorse Fuel. Il primo download può richiedere tempo: controllane l'avanzamento prima di avviare un'altra simulazione.

## Non compare un'immagine o il rendering non parte

Controlla driver grafici e collegamento al display nella tua [guida di installazione](requirements.md). Sia l'interfaccia di Gazebo sia le telecamere richiedono un renderer funzionante. Per il sonar servono anche le [librerie CUDA](sonar-tuning.md).

Per gli [effetti della scena subacquea](underwater-rendering.md), verifica:

- Il servizio `/posim/appearance/<session_nonce>/state`, i percorsi di renderer e shader e il numero di telecamere attive.
- La revisione applicata e lo stato di disponibilità dopo il rendering.
- L'attivazione esplicita delle particelle sospese, disabilitate per impostazione predefinita. L'esempio di attenuazione della telecamera non carica da solo il sistema degli effetti di scena.

## Il GNSS non fornisce una posizione valida

Controlla lo [stato di validità GNSS](gnss.md), la quota dell'antenna nel mondo, il livello locale dell'acqua e il timestamp di acquisizione. Verifica le coordinate sferiche del mondo, che l'antenna sia emersa e libera da ostacoli, e che trascorra abbastanza tempo di simulazione per riacquisizione e latenza.

Se è configurato un topic delle onde, deve fornire campioni locali compatibili e recenti. Installare il ricevitore non lo aggiunge al modello né fornisce un adattatore ROS o per l'autopilota.

## Non arrivano messaggi

Verifica prima che il tempo avanzi, poi cerca il topic e il tipo:

{{CODE:troubleshooting:1}}

Usa il trasporto corretto e lo stesso ambiente preparato. Controlla QoS, dominio ed errori di bridge e plugin. Per avviare un sensore, `namespace` deve indicare un descrittore esistente. L'[USBL](usbl.md) è già incluso nel mondo del tutorial e si avvia insieme al mondo.

## I topic delle immagini compaiono, ma i fotogrammi non arrivano {#image-topics-appear-but-frames-stall}

Se Gazebo genera immagini e ROS trova il publisher, ma arrivano pochi fotogrammi o nessuno, controlla il trasporto DDS. I datagrammi UDP grandi di Cyclone DDS possono incontrare limiti di frammentazione IP. Ridurne le dimensioni può aiutare su reti con MTU di 1500 byte. Consulta le [impostazioni di frammentazione di Cyclone DDS](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation).

### Preparare il profilo

Con `rmw_cyclonedds_cpp` installato, crea questo profilo una sola volta. Se usi già un profilo Cyclone DDS, aggiungi le due dimensioni alla sua sezione `General`, conservando le impostazioni di discovery e delle interfacce.

{{CODE:troubleshooting:2}}

### Riavviare con la stessa configurazione

Ferma simulazione e visualizzatori. Carica l'ambiente POSIM e seleziona il profilo in ogni terminale prima di riavviarli. Se hai modificato un profilo esistente, usa il suo percorso.

{{CODE:troubleshooting:3}}

Se il processo in background della CLI ROS è già attivo in quel dominio, fermalo una volta per applicare la nuova configurazione alle successive chiamate:

{{CODE:troubleshooting:4}}

Ripeti i [controlli delle immagini e della frequenza](camera.md#run-the-example), verificando anche i timestamp di acquisizione. I pacchetti piccoli aumentano il sovraccarico; reti con MTU inferiore richiedono dimensioni ancora più piccole.

## Il veicolo non si muove

Controlla stato del controllore, armamento, validità della navigazione, ricezione dei comandi e modalità selezionata. Verifica nomi dei giunti e assi dei propulsori. Mantieni una sola sorgente di comandi attiva. Un comando neutro, il mondo in pausa o l'autopilota disarmato possono spiegare l'immobilità.

## La simulazione non termina

Ferma i visualizzatori, premi Ctrl+C una volta nel terminale di avvio e attendi la chiusura dei processi. Conserva i messaggi pertinenti di arresto anomalo, errore di segmentazione o terminazione forzata. Nella [segnalazione](https://github.com/IOES-Lab/POSIM/issues), indica comando, revisione, sistema operativo, architettura e renderer.

### RViz si arresta dopo Discard

Su Ubuntu 26.04, RViz 15.2.6 può arrestarsi in modo intermittente quando chiudi la finestra e scegli **Discard**, anche se le viste dei sensori funzionano. Ferma i processi di avvio rimasti con Ctrl+C prima di ripartire. Indica versione di RViz e azione di chiusura nella segnalazione.

## Ricreare telecamere di profondità con un SDK macOS nativo

Per **macOS arm64 / Ogre-Next 2.3.3**, l'[ambiente Ogre isolato](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) fornisce la correzione del ciclo di vita dei materiali durante la distruzione e la ricreazione delle telecamere di profondità.

Rispetta le versioni di SDK e sorgenti previste, usa lo strumento di compilazione e passa il comando Gazebo a `run_overlay.py`. Compilare non seleziona automaticamente la libreria. Verifica i percorsi effettivamente caricati e conserva il risultato della validazione. L'installazione standard non attiva questo ambiente; non sostituisce librerie di sistema e non configura container Linux. Altre versioni o piattaforme richiedono un profilo compatibile dedicato.

## Preparare una segnalazione riproducibile

Includi SHA del codice o digest dell'immagine, versioni delle dipendenze, comando e argomenti, primo errore, risultato atteso, tipi dei topic interessati e un esempio minimo. Distingui un topic semplicemente trovato da uno che trasmette davvero dati.

## MAVROS 2.16.0 termina con SIGSEGV

Il programma di installazione di POSIM e le build Docker ufficiali includono la
[correzione del ciclo di vita del Router](https://github.com/IOES-Lab/POSIM/tree/main/extras/mavros)
per MAVROS 2.16.0. Dopo aver caricato l'ambiente POSIM, `ros2 pkg prefix mavros`
deve mostrare `/opt/posim/mavros-router/install`. Per un'installazione esistente,
segui le istruzioni per la compilazione separata disponibili al link. Se il crash
persiste, allega alla segnalazione il log di MAVROS e i percorsi delle librerie
effettivamente caricate.
