# Prima simulazione

Avvia un REXROV nell'oceano, ricevi le sue misure e chiudi la simulazione. Per questa prima prova, mantieni il modello e il mondo indicati.

## Preparare i terminali

Completa l'[installazione](requirements.md), poi apri [due terminali con ROS e POSIM caricati](terminals.md). Il terminale A esegue il simulatore; B legge i dati. Con Docker, Gazebo appare nel desktop Linux, WSLg o RDP configurato durante l'installazione.

## 1. Avviare robot e mondo

Nel **terminale A**:

{{CODE:quickstart:0}}

REXROV viene posizionato cinque metri sotto z = 0 e si avviano i suoi bridge ROS. Il comando avvia anche il mondo: eseguilo come un'unica simulazione e lascia aperto A.

## 2. Leggere la posizione

Nel **terminale B**:

{{CODE:quickstart:1}}

Le misure devono arrivare continuamente. Valori quasi costanti sono normali quando il robot si stabilizza. Premi **Ctrl+C in B** per interrompere la visualizzazione.

## 3. Verificare il tempo simulato

{{CODE:quickstart:2}}

L'orologio deve avanzare. Premi Ctrl+C per chiudere la visualizzazione. Se il tempo è fermo o non arrivano messaggi, consulta la [risoluzione dei problemi](troubleshooting.md).

## 4. Terminare la sessione

Premi **Ctrl+C in A** e attendi la chiusura dei processi. Chiudere il visualizzatore di un topic non arresta il simulatore. Ferma questa simulazione prima di avviarne un'altra.

## Proseguire

- [Visualizzare e registrare i sensori](visualization.md): immagini e osservazioni salvate.
- [Controllare con Python](python-control.md): leggere lo stato e azionare un propulsore.
- [ROV e BlueROV2](rovs.md): ArduSub, tastiera e joystick.
- [Libreria dei mondi](worlds.md): scegliere un altro ambiente.
