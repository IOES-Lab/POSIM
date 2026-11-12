# Ambiente Docker

Le immagini POSIM includono Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, ArduRover, MAVROS e QGroundControl. Parti dalla [scelta dell’ambiente](requirements.md) per configurare schermo e driver in base al sistema operativo.

## Scaricare un’immagine pubblicata

Le immagini sono su [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags). Docker seleziona automaticamente la variante AMD64 o ARM64.

{{CODE:docker:0}}

| Tag | Utilizzo |
|---|---|
| `latest` | Build pubblicata del ramo principale, disponibile per entrambe le architetture |
| `sha-<12-character-commit>` | Una revisione precisa del codice; copia un tag già presente su Docker Hub |
| Tag SHA con suffisso `-amd64` o `-arm64` | Scelta esplicita dell’architettura |
| Numero di release, se pubblicato | Una versione contrassegnata da un tag di rilascio |

L’etichetta dell’immagine riporta la revisione completa. Conserva anche il digest per ripetere un esperimento con la stessa identica immagine.

{{CODE:docker:1}}

Consulta la documentazione relativa alla revisione scelta. Scaricare una nuova immagine non aggiorna i container già esistenti.

## Creare l’immagine dai sorgenti

Sul computer host:

{{CODE:docker:2}}

Per AMD64:

{{CODE:docker:3}}

Per ARM64:

{{CODE:docker:4}}

Nella guida d’installazione, sostituisci `ioeslab/posim:latest` con il tag locale. Entrambe le immagini compilano i sorgenti presenti e la revisione fissata della libreria delle onde. La variante ARM64 include anche un desktop XFCE e un server RDP.

## Aprire l’ambiente

- [Linux con Docker](install-ubuntu-docker.md): usa lo schermo del desktop Linux.
- [Windows con WSL e Docker](install-windows-docker.md): mostra le finestre tramite WSLg.
- [macOS con Docker e RDP](install-macos.md): accede al desktop Linux remoto.

Ogni guida crea un container chiamato `posim`. [Preparare i terminali](terminals.md) spiega dove inserire i comandi e caricare il workspace.

## Conservare i file

Il workspace si trova in `/opt/posim_ws` su AMD64 e in `/home/docker/posim_ws` su ARM64. Le variabili `POSIM_WS` o `POSIM_UNDERLAY` ne indicano il percorso.

Salva modelli SDF e registrazioni in `/work`, collegato al volume `posim-data`. Gli altri file sopravvivono all’arresto del container, ma vengono eliminati insieme al container stesso. Prima di sostituirlo, registra le modifiche ai sorgenti in Git o copiale all’esterno. Il volume rimane disponibile finché non lo rimuovi esplicitamente.

## Arrestare, riprendere e aggiornare {#update-an-existing-container}

Ferma prima la simulazione con **Ctrl+C**, poi esegui sull’host:

{{CODE:docker:5}}

Per aggiornare, salva il lavoro, arresta il container, scarica l’immagine desiderata e rimuovi il container fermo. Ripeti il comando di creazione della tua guida, riutilizzando `posim-data`.

## Grafica e GPU {#graphics-and-gpu-access}

Su Linux configura il driver dell’host e [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). Su Windows segui la [configurazione GPU di Docker Desktop con WSL2](https://docs.docker.com/desktop/features/gpu/). Il sonar CUDA richiede anche le librerie nell’immagine: vedi [Configurare il sonar](sonar-tuning.md). Le istruzioni iniziali usano il rendering software, che può essere più lento.
