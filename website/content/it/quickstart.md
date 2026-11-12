# Prima simulazione

Esplora una scena di ispezione in acque poco profonde con BlueROV2, poi un percorso tra boe con WAM-V. Ogni esempio apre Gazebo e una finestra RViz già configurata. Esegui una sola simulazione alla volta.

## Prima di iniziare

Completa un’[installazione](requirements.md) e carica lo stesso workspace ROS/POSIM nei [terminali A e B](terminals.md). A esegue il simulatore; B legge i dati e avvia il video. Con Docker, usa il desktop Linux, WSLg o RDP configurato durante l’installazione. Entrambi gli esempi fanno parte dell’installazione standard e non richiedono un’immagine separata per i veicoli di superficie.

## Ispezione subacquea con BlueROV2

Nel **terminale A**:

{{CODE:quickstart:0}}

BlueROV2 parte circa 3,1 m sotto la superficie media, sopra un fondale a 4 m, rivolto verso una scatola, un tubo, un pilastro e una sfera. Sono attive piccole onde, foschia subacquea, assorbimento, caustiche e neve marina. La zavorra è regolata per mantenere il veicolo disarmato vicino agli oggetti. Gazebo mostra la telecamera in diretta accanto alla scena.

Per aggiungere il sonar multibeam, aggiungi `sonar:=true` al comando. Viene selezionato BlueROV2 Heavy con sonar: Gazebo aggiunge l’immagine sonar e RViz mostra immagine e nuvola di punti. Il sonar richiede molte risorse di calcolo e **può ridurre la frequenza delle immagini e la reattività dei comandi**. In base all’host e al sensore, `auto` prova CUDA, poi WGPU e infine CPU. Il rendering mantiene le impostazioni abituali. Consulta i [requisiti del sonar](sonar-tuning.md).

Le immagini seguenti usano la variante con sonar. RViz carica automaticamente modello, telecamera e viste dei sensori. La griglia da 1 m è un riferimento di distanza nel sistema del veicolo, non un fondale misurato. **Move Camera** è selezionato: trascina per ruotare, usa il pulsante centrale per traslare e la rotellina per lo zoom. Le eliche visualizzate mantengono l’angolo di riposo; la dinamica simulata resta invariata.

{{MEDIA:quickstart:0}}

### Controllare i dati

Nel **terminale B**:

{{CODE:quickstart:1}}

Con il sonar, sostituisci `bluerov2` nel topic con `bluerov2_heavy_multibeam_sonar`. Le posizioni devono continuare ad arrivare anche se il veicolo è quasi fermo. Ferma la lettura con **Ctrl+C**, poi verifica che il tempo simulato avanzi:

{{CODE:quickstart:2}}

Premi Ctrl+C per terminare questa lettura. Se mancano messaggi o il tempo è fermo, consulta la [risoluzione dei problemi](troubleshooting.md).

### Video e controllo con QGC

Segui i passaggi 1–3 della [configurazione QGC](qgroundcontrol.md). Apri QGroundControl nello stesso ambiente Linux e abilita i joystick virtuali. Seleziona **UDP h.264 Video Stream**, indirizzo **127.0.0.1:5600**, e disattiva **Disable Video Stream When Disarmed**. Mantieni in esecuzione il tutorial e avvia il video nel terminale B:

{{CODE:quickstart:3}}

Fai clic sul piccolo pannello video di QGC per ingrandirlo. **No GPS Lock** è normale quando l’antenna GNSS marina è sommersa. Telemetria, video e controllo Manual restano disponibili. Attendi **Ready**, scegli **Manual**, arma e muovi brevemente il joystick. Rilascialo al centro e disarma. Consulta i [passaggi di controllo QGC](qgroundcontrol.md). Non avviare contemporaneamente un altro controllo da tastiera o joystick.

{{MEDIA:quickstart:1}}

## Sensori di superficie con WAM-V

Termina prima la sessione subacquea. Nel **terminale A**:

{{CODE:quickstart:4}}

WAM-V parte su piccole onde davanti a quattro boe ancorate. RViz apre la telecamera e una vista comune di veicolo e nuvola lidar, con griglia da 1 m e Move Camera selezionato. Il lidar esegue una scansione orizzontale: i punti delineano le boe all’altezza del sensore, senza ricostruire l’intero oggetto in 3D.

Nel pannello **Lidar** di Gazebo, premi aggiorna e seleziona `/tutorial/lidar`. Disattiva **Show Non Hitting Rays** per mostrare solo i raggi che colpiscono oggetti. Il pannello della telecamera resta accanto alla scena.

{{MEDIA:quickstart:2}}

La telecamera usa lo stesso topic `/tutorial/camera`, lo stesso comando di avvio e le stesse impostazioni video QGC dell’esempio subacqueo. WAM-V usa la modalità barca di ArduRover e posizione/velocità simulate come navigazione esterna; vedi [Robot di superficie e onde](surface.md). Questo ingresso è distinto dal ricevitore GNSS marino di BlueROV2.

In modalità Manual di QGC, il movimento verticale dello stick sinistro controlla la spinta di WAM-V. Arma, muovilo brevemente, rilascialo al centro e disarma.

{{MEDIA:quickstart:3}}

Leggi una scansione lidar nel **terminale B**:

{{CODE:quickstart:5}}

La nuvola di punti è `/tutorial/lidar/points`; l’odometria è `/model/wamv/odometry`.

## Terminare la sessione

Disarma, chiudi RViz e QGC, poi premi **Ctrl+C nel terminale A** e attendi la fine dei processi. Se RViz chiede di salvare le modifiche alla vista, scartale per mantenere la configurazione fornita. In caso di chiusura anomala, consulta i [problemi del visualizzatore](troubleshooting.md). Chiudere la lettura di un topic non ferma la simulazione. Termina la sessione prima di avviare un altro esempio.

## Proseguire

- [Visualizzare e registrare i sensori](visualization.md): esaminare immagini e salvare misure.
- [Controllare con Python](python-control.md): avviare l’esempio REXROV separato e provare un propulsore.
- [ROV e BlueROV2](rovs.md): usare ArduSub, tastiera o joystick locale.
- [Libreria di mondi](worlds.md): scegliere l’ambiente e consultare gli argomenti di avvio.
