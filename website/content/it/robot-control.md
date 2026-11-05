# Propulsori e controllo con joystick

Scegli l'interfaccia adatta al robot. Verifica prima i sensori, poi attiva un solo controllore alla volta.

| Veicolo | Percorso di comando | Guida |
|---|---|---|
| REXROV | Forze ROS → propulsori Gazebo | [Python](python-control.md) |
| BlueROV2 | ROS/joystick → MAVROS → ArduSub | [QGC e video](qgroundcontrol.md) · [ROS/joystick](rovs.md) |
| WAM-V | Integrazione di superficie e controllore del veicolo | [Robot di superficie](surface.md) |

## Propulsori di REXROV

Nei [terminali preparati](terminals.md), mantieni REXROV in A e invia i comandi da B. La configurazione collega otto topic `cmd_thrust` con tipo `std_msgs/msg/Float64`. I valori esprimono la spinta in newton. Per portare al neutro il primo propulsore:

{{CODE:robot-control:0}}

Distribuisci le forze secondo posizione e asse di ciascun propulsore. Il controllo della velocità del veicolo richiede di coordinarli. Al termine, invia zero a tutti i propulsori utilizzati.

## BlueROV2 con ArduSub

Avvia l'[esempio BlueROV2](rovs.md#start-bluerov2), attendi la connessione dell'autopilota e controlla le interfacce:

{{CODE:robot-control:1}}

La durabilità `volatile` seleziona i nuovi messaggi di stato; il filtro attende `connected: true` dopo l'apertura della connessione. Se l'attesa continua, verifica l'output di avvio e la configurazione MAVROS/ArduSub.

L'adattatore converte `/joy` e `/keyboard/joy` in comandi MAVROS:

- Frequenza di pubblicazione: **20 Hz**.
- Scadenza dell'ingresso: **0,3 s**.

Usa una sola sorgente di comandi e controlla armamento e stato di navigazione. La pagina [ROV e BlueROV2](rovs.md) descrive tastiera e bridge WebSocket.
