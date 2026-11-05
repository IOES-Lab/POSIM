# Visualizzare e registrare i dati dei sensori

Mantieni il simulatore in esecuzione nel terminale A. Usa B per controllare le misure, aprire un visualizzatore o registrare una sequenza.

## Ricevere un'immagine

Prepara [entrambi i terminali nello stesso ambiente](terminals.md). Arresta la simulazione precedente con Ctrl+C in A, quindi avvia questa scena. Gazebo si apre nel desktop configurato durante l'installazione.

{{CODE:visualization:0}}

In B:

{{CODE:visualization:1}}

Controlla che larghezza e altezza siano maggiori di zero, quindi verifica codifica e timestamp. `--no-arr` nasconde l'array dei pixel. Per vedere l'immagine, aggiungi un display **Image** in RViz e seleziona questo topic. La [telecamera subacquea](camera.md) descrive i parametri di attenuazione.

## Scegliere il visualizzatore

In RViz, usa **Image** per le telecamere, **PointCloud2** per le nuvole di punti e i display **TF/pose** per i sistemi di riferimento e lo stato.

Ogni client ROS separato deve installare e caricare `posim_interfaces` per usare i messaggi specifici di POSIM. Trovi topic e tipi nelle pagine [telecamera](camera.md), [pressione](pressure.md), [DVL](dvl.md), [USBL](usbl.md) e [sonar](sonar.md). [Correnti](currents.md) e [coordinate sferiche](coordinates.md) offrono anche servizi ROS per modificare gli ingressi dell'esperimento.

## Registrare e riprodurre

{{CODE:visualization:2}}

Premi Ctrl+C per terminare. Scegli una cartella nuova per ogni registrazione e controlla il numero di messaggi:

{{CODE:visualization:3}}

Arresta il simulatore prima di riprodurre i dati sugli stessi topic:

{{CODE:visualization:4}}
