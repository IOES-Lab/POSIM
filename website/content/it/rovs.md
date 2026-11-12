# ROV e BlueROV2

L'argomento `namespace` seleziona la directory di un modello installato. I topic ROS dipendono dalla configurazione del modello.

## Modelli disponibili

| Modello | Uso |
|---|---|
| `rexrov` | Propulsori Gazebo, idrodinamica e odometria |
| `bluerov2` | BlueROV2 con ArduSub SITL e MAVROS |
| `bluerov2_heavy` | Configurazione Heavy |
| `bluerov2_heavy_multibeam_sonar` | Heavy con sonar |

Usa il [terminale preparato per la tua installazione](terminals.md). Per REXROV, usa l’[esempio seguente](#start-rexrov).

## Avviare BlueROV2 {#start-bluerov2}

{{CODE:rovs:0}}

Seleziona la variante Heavy con `namespace:=bluerov2_heavy`. Controlla la connessione dell'autopilota nell'output di avvio e in `/mavros/state`. Il modello con sonar richiede la [configurazione ](sonar-tuning.md).

## Controllare dalla tastiera

In un terminale interattivo del desktop, usa `gui:=true headless:=false use_teleop:=true`.

| Tasto | Azione |
|---|---|
| `c` / `x` | Armare / disarmare |
| `w` / `s` | Avanti / indietro |
| `a` / `d` | Ruotare a sinistra / destra |
| `r` / `f` | Risalire / scendere |
| `h` / `j` | ALT_HOLD / STABILIZE |
| Spazio | Comando neutro |

## Usare il joystick nel browser

Servi la pagina installata da un altro terminale preparato:

{{CODE:rovs:1}}

Avvia BlueROV2 con la teleoperazione attiva:

{{CODE:rovs:2}}

Mantieni `use_teleop:=true`: questo lancio include il bridge WebSocket. Se il browser non si apre automaticamente, visita l'URL nello stesso desktop Linux. Le immagini Docker includono Firefox e QGC. [QGroundControl](qgroundcontrol.md) descrive installazione, joystick virtuali e video in diretta.

Per sottoscrizioni e comandi dai tuoi programmi, prosegui con [ROS](ros.md).

## Vedere il robot in movimento

{{MEDIA:rovs}}

Immagini e video: POSIM Notion Wiki. L'attribuzione a DAVE è in [Citazioni e licenze](citation.md). Usa i comandi e i topic di questa pagina.

## Avviare REXROV {#start-rexrov}

Nel [terminale A preparato](terminals.md), avvia questo esempio separato per le guide Python e di controllo dei propulsori. Avvia il mondo oceanico e i bridge ROS e colloca REXROV a z = −5 m. Mantieni la sessione attiva mentre segui le guide nel terminale B.

{{CODE:rovs:3}}
