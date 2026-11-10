# Configurare e regolare il sonar

Scegli il backend di calcolo, poi confronta copertura, dettaglio e frequenza di aggiornamento. Il [riferimento del sonar multibeam](sonar.md) contiene avvio rapido, SDF e parametri.

## Scegliere l'ambiente

Usa i [terminali POSIM preparati](terminals.md). Modifica, compila ed esegui nella stessa installazione Ubuntu, WSL o Docker; `POSIM_WS` ne indica lo spazio di lavoro. Tutti i backend richiedono i sensori di rendering di Gazebo.

| Ambiente | Opzioni |
| --- | --- |
| Linux nativo | CPU; WGPU con un adattatore Vulkan utilizzabile; CUDA con GPU NVIDIA e toolkit compatibili |
| WSL 2 o container Linux | CPU; per il calcolo GPU, adattatore e driver devono essere accessibili **dentro** l'ambiente |
| Docker/RDP su macOS | Inizia con CPU. Il container Linux non espone il dispositivo Metal del Mac. |
| macOS nativo | WGPU può usare Metal con una toolchain ROS/Gazebo nativa preparata a parte; la [guida macOS](install-macos.md) descrive Docker/RDP. |

`auto` prova CUDA → WGPU → CPU. Per confronti controllati, seleziona esplicitamente `cpu`, `wgpu` o `cuda`. Se il backend scelto non è disponibile, il sensore non si inizializza. Consulta la [tabella delle opzioni](sonar.md#compute-backend).

## Compilare i pacchetti del sonar {#build-the-sonar-packages}

Parti dall'[installazione dai sorgenti](install.md) con le dipendenze già installate. `wgpu_vendor` compila la libreria Rust anche per usare CPU. Verifica Rust e Cargo; se mancano, segui l'[installazione ufficiale](https://www.rust-lang.org/tools/install).

{{CODE:sonar-tuning:0}}

Dallo spazio di lavoro con l'ambiente caricato, compila libreria, sensore, sistema del mondo ed esempio:

{{CODE:sonar-tuning:1}}

CUDA è facoltativo. Per ignorare un toolkit installato e compilare CPU/WGPU, aggiungi `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`. `Cargo.lock` fissa le dipendenze Rust; il primo download richiede Internet.

{{CODE:sonar-tuning:2}}

Controlla tutti e quattro i pacchetti, poi esegui l'[esempio](sonar.md#run-a-sonar-example) e ricevi immagini e dati grezzi. La presenza di un pacchetto non conferma che il sensore si carichi. Verifica i pacchetti dentro il container; se mancano, compila i sorgenti senza presumere che un tag dell'immagine li includa.

## Scegliere l'adattatore WGPU

Su Linux, esegui `vulkaninfo --summary` nell'ambiente di Gazebo, con driver e strumenti Vulkan del sistema. Un adattatore software come llvmpipe può eseguire Vulkan senza accelerazione hardware. WGPU mostra all'avvio l'adattatore selezionato.

{{CODE:sonar-tuning:3}}

| Variabile | Uso |
| --- | --- |
| `WGPU_BACKEND` | `vulkan` su Linux, `metal` in un processo macOS nativo; omettila per le impostazioni della piattaforma |
| `WGPU_ADAPTER_NAME` | Primo adattatore il cui nome contiene il testo, senza distinguere maiuscole; usa un nome mostrato dal driver o all'avvio |
| `WGPU_POWER_PREF` | `high`, `low` o `none`; predefinito `high` quando non è indicato un nome |

Imposta le variabili prima dell'avvio. Dopo averle cambiate, arresta e rilancia la simulazione: l'inizializzazione è condivisa nel processo. Il progetto [wgpu](https://github.com/gfx-rs/wgpu) descrive le API. Se l'inizializzazione o il controllo delle risorse falliscono, esamina adattatore e limiti, poi scegli `cpu` o riduci le dimensioni del sensore.

## Configurare CUDA, se necessario

Servono driver NVIDIA, compilatore/runtime CUDA, cuFFT e cuBLAS. Verifica l'ambiente di esecuzione:

{{CODE:sonar-tuning:4}}

Per i container Linux configura [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) sull'host. Su Windows segui [CUDA su WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) e, se necessario, la [configurazione GPU di Docker Desktop](https://docs.docker.com/desktop/features/gpu/). Installa il driver video Windows, non un driver Linux dentro WSL. Anche il container deve avere le librerie CUDA.

Scegli un'architettura compatibile. L'esempio usa SM 75: sostituisci `75` con il valore della tua GPU. `OFF` riattiva la ricerca del toolkit dopo una compilazione solo CPU/WGPU.

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` elenca i target disponibili. Senza scelta esplicita vale il valore predefinito del compilatore. Con più toolkit, usa `-DCUDAToolkit_ROOT=/path/to/cuda` e aggiungi il relativo `bin` a `PATH`. Avvia l'esempio con `compute_backend:=cuda`.

## Misurare un riferimento

Mantieni fissi scena, distanza dal bersaglio e posa del sensore. Nel terminale B:

{{CODE:sonar-tuning:6}}

Interrompi con Ctrl+C, poi controlla la velocità della simulazione:

{{CODE:sonar-tuning:7}}

L'entità del mondo si chiama `default`. Registra `real_time_factor` insieme alla frequenza ricevuta: richiedere 30 Hz nel tempo simulato non garantisce 30 immagini al secondo reale. Esci con Ctrl+C.

## Regolare campionamento e portata

Modifica un valore alla volta in una copia del [SDF](sonar.md#sensor-configuration), ricompila, carica l'ambiente e avvia con la stessa posa.

| Esperimento | Confronto | Cosa osservare |
|---|---|---|
| Dettaglio angolare | 512 contro 256 fasci orizzontali, stesso campo visivo | Separazione dei bersagli e frequenza |
| Campionamento verticale | 300 contro 150 raggi | Continuità dei bersagli e tempo di calcolo |
| Copertura | Cambiare insieme portata dei raggi e `maxDistance` | Bersaglio più lontano e scala delle distanze |
| Aggiornamento | 30 contro 10 Hz | Frequenza ricevuta e fattore di tempo reale |
| Salto dei raggi | `raySkips` 10 contro 5 | Dettaglio verticale e costo di calcolo |

Il passo in distanza è `soundSpeed / (2 × bandwidth)`; la griglia ha `ceil(maxDistance / passo) + 1` celle. Con 1500 m/s, 29.9 kHz e 15 m sono 599. L'ultima coordinata può superare leggermente `maxDistance`, ma gli echi oltre portata vengono esclusi. Leggi distanze e numero di fasci dal messaggio grezzo o dal CSV.

WGPU conserva la lunghezza richiesta, anche se non è una potenza di due, e sceglie automaticamente gli stadi FFT. Portata e larghezza di banda maggiori aumentano memoria e lavoro. CPU assegna gli echi alla cella di distanza più vicina; GPU elabora nel dominio delle frequenze. Confronta anche ampiezza e risposta in distanza tra backend.

## Conservare un confronto riproducibile

Attendi che il rendering si stabilizzi. Disattiva `writeLog` e `debugFlag` per misurare il throughput; registra separatamente con [CSV](sonar.md) o [ROS](visualization.md). `blazingSonarImage=false` fissa il seme a 1234.

Salva SDF, revisione POSIM, backend/adattatore, versioni driver/toolkit, tempo per immagine, frequenza ricevuta, fattore di tempo reale e memoria. Confronta dati grezzi di intensità/distanza oltre alle schermate. Le [citazioni](citation.md) riportano il modello scientifico.

## Elaborazione GPU

{{MEDIA:sonar-tuning}}

Le fasi comprendono somma dei raggi, finestratura/correzione e FFT. Misura separatamente calcolo e trasferimenti di memoria. Figura: POSIM Notion Wiki.
