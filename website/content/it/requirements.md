# Scegliere un ambiente

Scegli il percorso adatto al sistema operativo del tuo computer. **Consigliamo Ubuntu 26.04 nativo** per accedere direttamente alla grafica Gazebo, agli strumenti ROS e ai dispositivi NVIDIA/CUDA.

## Percorsi di installazione

| Sistema operativo | Metodo | Guida |
| --- | --- | --- |
| Ubuntu 26.04 | Compilazione ed esecuzione diretta di POSIM | [Ubuntu nativo](install.md) |
| Ubuntu o altra distribuzione Linux supportata da Docker, x86-64 | Immagine Ubuntu di POSIM in Docker; finestre sul desktop Linux | [Linux con Docker](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | Ubuntu 26.04 in WSL2; compilazione nativa e finestre tramite WSLg | [Windows con WSL](install-windows.md) |
| Windows 10/11, x86-64 | Docker Desktop con WSL2; immagine POSIM e finestre tramite WSLg | [WSL e Docker](install-windows-docker.md) |
| macOS con Apple Silicon | Immagine ARM64 in Docker; desktop Linux tramite RDP | [macOS con Docker e RDP](install-macos.md) |

L’immagine fornisce il proprio ambiente Ubuntu. L’host Linux può quindi usare un’altra versione o distribuzione supportata da Docker. L’emulazione ARM64 sui Mac Intel non è trattata qui.

Per questa versione, usa Docker/RDP su macOS. L’installazione nativa su macOS non è ancora stata verificata nel suo insieme. I riferimenti al supporto Metal nativo riguardano singoli componenti e non confermano il supporto dell’intero ambiente del tutorial.

## Software e grafica

Tutti i percorsi utilizzano **ROS 2 Lyrical** e **Gazebo Sim — Gazebo Jetty**. L’installazione nativa li prepara in Ubuntu; le immagini li contengono già. Installa il driver grafico sull’host come indicato nella guida scelta.

Gazebo e le telecamere a colori/profondità richiedono un renderer funzionante. Inizia con un robot e i sensori predefiniti. Il sonar multibeam offre CPU, WGPU e CUDA: [scegli e prepara il backend](sonar-tuning.md). WGPU richiede Vulkan utilizzabile su Linux o Metal su macOS nativo. Docker/RDP su macOS esegue Linux senza accesso a Metal; inizia con CPU.

## Risorse e download

Il carico dipende dal numero di veicoli, dal dettaglio del terreno, dalla risoluzione e dalla frequenza dei sensori. Controlla memoria, GPU e fattore di tempo reale prima di aumentare questi valori. Prevedi spazio per sorgenti, compilazioni, immagini Docker, modelli Fuel e registrazioni.

L’installazione richiede accesso ai repository ROS/Gazebo, a GitHub e a Docker Hub. Alcuni mondi scaricano modelli Fuel al primo avvio. Quando il mondo dimostrativo funziona, [prepara i terminali](terminals.md).
