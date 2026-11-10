# Terminali e topic ROS

Il simulatore gira in A; B serve a leggere i dati o inviare comandi. Entrambi devono caricare la stessa distribuzione ROS e lo stesso workspace POSIM.

## Seguire il flusso dei dati

<div class="docs-flow"><div><strong>Programmi e comandi ROS</strong><span>Sottoscrizioni, servizi e comandi</span></div><div><strong>Interfaccia ROS</strong><span>Bridge ros_gz o publisher del plugin</span></div><div><strong>Gazebo e plugin POSIM</strong><span>Fisica, stato e misure</span></div></div>

Le misure arrivano al programma ROS; i comandi vanno al simulatore. Un topic Gazebo compare in ROS quando viene pubblicato da un bridge o da un plugin con interfaccia ROS. Il lancio del robot carica la relativa configurazione dei bridge.

## Preparare A e B

Segui la [preparazione dei terminali per la tua installazione](terminals.md). Mantieni la [prima simulazione](quickstart.md) in A ed esegui i comandi seguenti in B.

## Esaminare le connessioni

`--no-daemon` interroga direttamente ROS, senza dipendere dalla cache del demone della CLI.

{{CODE:ros:0}}

`ros2 topic list` elenca i topic ROS; `gz topic -l` quelli Gazebo Transport. Controlla tipo di messaggio, publisher, subscriber e QoS prima di collegare un programma.

## Osservare una misura

{{CODE:ros:1}}

La posizione può restare quasi costante, purché arrivino nuove misure con timestamp aggiornati. **Ctrl+C in B** chiude il visualizzatore; A continua a simulare. Usa `--once` per un solo messaggio. I nodi che utilizzano il tempo simulato devono impostare `use_sim_time:=true` e ricevere `/clock`.

## Collegare un altro computer ROS

Usa pacchetti ROS e messaggi compatibili, lo stesso dominio e una rete che permetta discovery e scambio dati DDS. Installa `posim_interfaces` per i messaggi specifici di POSIM. Se ci sono publisher ma non arrivano misure, consulta la [risoluzione dei problemi](troubleshooting.md).

Prosegui con [Python](python-control.md), [propulsori e joystick](robot-control.md) oppure [visualizzazione e registrazione](visualization.md).
