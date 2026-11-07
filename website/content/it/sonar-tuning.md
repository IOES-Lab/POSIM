# Configurare e regolare il sonar

Scegli copertura, dettaglio e frequenza per l'esperimento. Il [riferimento del sonar multibeam](sonar.md) contiene avvio rapido, SDF e parametri; questa guida spiega come confrontare configurazioni e preparare CUDA.

## Ambiente necessario

Usa i [terminali Linux preparati](terminals.md) su Ubuntu o WSL 2 compatibile con GPU NVIDIA, oppure in un contenitore che vi acceda. Modifica e compila lì; `POSIM_WS` è il relativo spazio di lavoro. Docker/RDP su macOS non espone una GPU CUDA: esegui questo sonar su un host Linux o WSL 2 con NVIDIA.

## Raccogliere una misura di riferimento

Avvia l'[esempio BlueView P900](sonar.md#run-a-sonar-example). Mantieni fissi scena, distanza dal bersaglio e posa del sensore. Verifica immagini e dati grezzi prima di misurare. Se manca CUDA, prepara prima gli strumenti.

In un altro terminale preparato, misura la frequenza ricevuta:

{{CODE:sonar-tuning:0}}

Dopo un campione stabile, ferma con Ctrl+C. Controlla separatamente la velocità di simulazione:

{{CODE:sonar-tuning:1}}

Il nome interno del mondo è `default`. Registra `real_time_factor`: richiedere 30 Hz simulati non garantisce 30 immagini per secondo reale. Ctrl+C interrompe la lettura.

## Confrontare una variabile

Modifica un'impostazione in una copia del [descrittore](sonar.md#sensor-configuration), ricompila, carica l'ambiente e riavvia alla stessa posa.

| Esperimento | Confronto | Cosa osservare |
|---|---|---|
| Dettaglio angolare | 512 contro 256 fasci orizzontali, stesso campo visivo | Separazione dei bersagli e frequenza |
| Campionamento verticale | 300 contro 150 raggi | Continuità dei bersagli e tempo di calcolo |
| Copertura | Cambiare insieme portata dei raggi e `maxDistance` | Bersaglio più lontano e scala delle distanze |
| Aggiornamento | 30 contro 10 Hz | Frequenza ricevuta e fattore di tempo reale |
| Salto dei raggi | `raySkips` 10 contro 5 | Dettaglio verticale e costo di calcolo |

Misura il risultato di ogni confronto. Disattiva `writeLog` e `debugFlag` per misurare il rendimento; attivali separatamente per esaminare ritorni o tempi. Conserva SDF, revisione POSIM, versioni GPU/CUDA e osservazioni. [guida ai dati dei sensori](visualization.md) spiega la registrazione dei topic.

## Preparare gli strumenti

Il [plugin](sonar.md) richiede driver NVIDIA, compilatore e runtime CUDA, cuFFT, cuBLAS e rendering Gazebo. Nell'ambiente che eseguirà il sonar:

{{CODE:sonar-tuning:2}}

Verifica GPU e compilatore separatamente. Con Docker Engine su Linux configura [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) sull'host. Su Windows segui la [configurazione GPU Docker Desktop/WSL 2](https://docs.docker.com/desktop/features/gpu/). L'immagine richiede anche le librerie CUDA.

## Scegliere l'architettura CUDA

Imposta `CMAKE_CUDA_ARCHITECTURES` su una versione SM supportata da GPU e toolkit. L'esempio usa SM 75: sostituisci `75` con la tua architettura e compila dallo spazio di lavoro preparato.

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` elenca i target supportati. Senza una scelta esplicita, CMake usa il valore predefinito del compilatore. Con più installazioni CUDA, seleziona quella adatta con `-DCUDAToolkit_ROOT=/path/to/cuda` e aggiungi la relativa directory `bin` a `PATH`.

## Verificare i componenti installati

{{CODE:sonar-tuning:4}}

I pacchetti possono risultare registrati anche se i target CUDA sono stati saltati. Controlla le librerie:

{{CODE:sonar-tuning:5}}

Ogni comando deve terminare con codice 0. Controlla target saltati o errori di caricamento, poi ricevi i dati descritti in [Sonar multibeam](sonar.md).

## Interpretare le misure

Lascia stabilizzare il renderer prima di misurare. A parità di scena e posa, registra tempo per fotogramma, frequenza ricevuta, fattore di tempo reale, utilizzo CPU/GPU e memoria. Distingui fisica, rendering, calcolo CUDA, pubblicazione e registrazione.

`blazingSonarImage` sceglie fra semi casuali variabili e il seme fisso `1234`: confronta la ripetibilità. Conserva dati di riferimento quando modifichi i kernel numerici e confronta intensità e distanza oltre alle immagini. Le [citazioni](citation.md) riportano la ricerca originale.

## Elaborazione GPU

{{MEDIA:sonar-tuning}}

L'elaborazione comprende somma dei raggi, finestratura/correzione e FFT. Nel profiling, separa questi tempi dai trasferimenti di memoria GPU. Immagine: wiki Notion POSIM.
