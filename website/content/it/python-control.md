# Controllare con Python

Mantieni REXROV attivo con la [prima simulazione](quickstart.md). In un altro [terminale preparato](terminals.md), leggi la posizione e applica un breve impulso a un propulsore.

## Sottoscrivere la posizione

Salva questo programma come `observe.py` ed esegui `python3 observe.py` nell'ambiente ROS caricato.

{{CODE:python-control:0}}

La posizione compare circa una volta al secondo. Ctrl+C arresta l'osservatore lasciando attivo il simulatore.

## Inviare un comando limitato

Salva il programma seguente come `pulse_thruster.py` ed esegui `python3 pulse_thruster.py`. Attende un subscriber, applica **5 N al propulsore 1** per circa due secondi misurati sul computer host, poi invia zero. L'asse di montaggio determina forza e momento risultanti.

{{CODE:python-control:1}}

L'impostazione dei segnali permette a Ctrl+C di eseguire il blocco di pulizia mentre il publisher ROS è ancora disponibile. Se termini forzatamente il processo, arresta la simulazione oppure invia comandi neutri seguendo [Controllo dei propulsori](robot-control.md).

## Osservare la risposta

In un altro terminale preparato, osserva la velocità angolare durante l'impulso:

{{CODE:python-control:2}}

La velocità deve rispondere e tornare verso zero. Confronta l'odometria per distinguere la risposta dell'attuatore dal movimento del veicolo. Ctrl+C chiude il visualizzatore.

## Sviluppare un controllore

Distribuisci le forze tra gli otto propulsori di REXROV usando lo stato misurato e la tua legge di controllo. Mantieni coerenti topic, assi e unità. Usa il tempo simulato per la logica dell'esperimento e riporta al neutro tutti gli attuatori comandati quando termini.

BlueROV2 usa il percorso [MAVROS/ArduSub](robot-control.md). Per confrontare le prove, [registra le osservazioni](visualization.md).
