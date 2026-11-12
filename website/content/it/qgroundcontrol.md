# Controllare con QGroundControl

Collega QGroundControl (QGC) al BlueROV2 simulato, usa i joystick virtuali e osserva la telecamera anteriore. Completa l’[installazione](requirements.md) e prepara i [terminali A e B](terminals.md).

## 1. Aprire QGC nell'ambiente POSIM

**Windows/WSL:** per questi tutorial usa la versione Linux di QGC. Non avviare l’applicazione installata su Windows. Avvia QGC nella stessa distribuzione Ubuntu di POSIM oppure, con Docker, nello stesso contenitore POSIM. WSLg può mostrare la finestra Linux sul desktop di Windows; l’applicazione continua a essere eseguita in Linux.

Gli indirizzi `127.0.0.1` di questa guida si riferiscono all’ambiente in cui viene eseguito QGC. Non configurano l’applicazione Windows per ricevere telemetria o video da WSL o Docker.

| Installazione | Dove eseguire QGC |
|---|---|
| Ubuntu nativo | Installa e avvia l'applicazione Linux in Ubuntu. |
| Windows con WSL nativo | Installa e avvia l'applicazione Linux nella stessa distribuzione Ubuntu. |
| Linux o Windows con Docker | Avvia l'applicazione inclusa nel container POSIM. |
| macOS con Docker e RDP | Avvia l'applicazione da un terminale del desktop RDP. |

Questa guida usa QGC **5.1.5**, incluso nelle immagini Docker di POSIM. Per un’installazione nativa, segui la [guida ufficiale all’installazione](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html) e scarica l’AppImage Linux per la tua architettura dalla [pagina della versione 5.1.5](https://github.com/mavlink/qgroundcontrol/releases/tag/v5.1.5). Nel container, usa il terminale B:

{{CODE:qgroundcontrol:0}}

Il simbolo `&` restituisce il prompt mentre QGC rimane aperto. Se hai avviato l'applicazione da un altro terminale, lascialo aperto. Se compaiono le preferenze iniziali, scegli **Metric System**.

QGC, MAVROS e Gazebo condividono lo stesso ambiente Linux. La telemetria usa **UDP 14550**, il video **UDP 5600**. Questa configurazione non richiede di pubblicare altre porte Docker. Su macOS, accedi all'ambiente tramite RDP.

## 2. Preparare i joystick

Apri **Application Settings → Fly View → Virtual Joystick**:

- Attiva **Virtual Joystick**.
- Attiva **Auto-Center Throttle** per riportare il comando al centro quando rilasci la leva.
- Lascia **Left-Handed Mode** disattivato per mantenere la disposizione usata nell'esempio.

## 3. Configurare il video

Apri **Application Settings → Video**. Seleziona prima la sorgente video, poi inserisci l'indirizzo UDP e gli altri valori della tabella.

| Impostazione | Valore |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`, proporzioni automatiche |
| Disable Video Stream When Disarmed | Disattivato |
| Low Latency Mode | Attivo |
| Auto-reconnect on stream loss | Attivato |

## 4. Avviare robot e telecamera

Arresta la simulazione precedente. Nel terminale A preparato:

{{CODE:qgroundcontrol:1}}

Gazebo apre la scena del fondale e QGC si collega ad ArduSub. Attendi il completamento dell'inizializzazione. Nel terminale B:

{{CODE:qgroundcontrol:2}}

Torna a **Fly View** e fai clic sull'anteprima video per ingrandirla.

{{MEDIA:qgroundcontrol}}

La telecamera produce immagini di **640 × 480 a 5 Hz nel tempo di simulazione**. Il rendering software può ridurre la frequenza effettiva. BlueROV2 ha un assetto a galleggiabilità positiva: anche disarmato, può risalire e portare gli oggetti fuori dall'inquadratura.

`qgc_video:=true` aggiunge il trasmettitore H.264. Il video RGB include gli [effetti subacquei del mondo](underwater-rendering.md). Il [filtro della telecamera](camera.md) indipendente ha una propria uscita ROS. GNSS perde la posizione quando l'antenna si immerge e la riacquisisce dopo l'emersione; telemetria, video e controllo manuale restano connessi. Usa Manual sott'acqua. La lente è più bassa dell'antenna, quindi l'immagine può essere subacquea anche con una posizione GNSS valida.

## 5. Armare, muovere e fermare

1. Attendi **Ready** e scegli **Manual**.
2. Apri il menu di stato del veicolo e tieni premuto **Arm** fino al completamento.
3. Sposta brevemente la leva destra in avanti e indietro.
4. Rilasciala per tornare al comando neutro. Correnti e inerzia possono continuare a muovere il robot.
5. Al termine, tieni premuto **Disarm** nel menu di stato.

Mantieni disattivati il controllo da tastiera e il joystick web per evitare comandi concorrenti. Ferma il video dal terminale B:

{{CODE:qgroundcontrol:3}}

Premi **Ctrl+C** in A e chiudi QGC.

MAVROS attende fino a 60 secondi senza ricevere un heartbeat dal veicolo prima di segnalare la perdita della connessione. Questo margine tiene conto del rendering software lento; una reale interruzione degli heartbeat può quindi richiedere fino a 60 secondi per apparire in `/mavros/state`.

## Risolvere problemi di connessione

Le opzioni `qgc_video` e `qgc_video_port` qui sotto si applicano all’avvio separato del passaggio 4. Le [prime simulazioni](quickstart.md) includono già un trasmettitore video; usa il loro comando `/tutorial/camera/enable_streaming`. Se il carico del sonar allunga gli intervalli tra i fotogrammi, QGC può mostrare temporaneamente **Waiting for video**. Mantieni **Low Latency Mode** attivo e prova il tutorial subacqueo senza sonar.

- **Veicolo assente:** verifica che QGC sia nello stesso ambiente Linux di POSIM e che un'altra istanza non occupi UDP 14550.
- **Waiting for video:** controlla sorgente H.264, indirizzo UDP e `qgc_video:=true`. Attiva l'invio da B e attendi qualche fotogramma.
- **Porta diversa:** usa `qgc_video_port:=5602` e imposta `127.0.0.1:5602` in QGC.
- **Opzione `qgc_video` assente:** aggiorna POSIM oppure [scarica l'immagine e ricrea il container](docker.md#update-an-existing-container).
- **Avvisi su firmware o porta seriale:** la simulazione usa ArduSub incluso e una connessione di rete; non richiede aggiornamenti firmware né una configurazione seriale.

Consulta le [impostazioni video ufficiali](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html) e le [interfacce ROS dei propulsori](robot-control.md).
