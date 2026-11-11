# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![Compilazione e pubblicazione delle immagini Docker di POSIM](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM è una libreria open source per la simulazione nella robotica marina. L’[IOES Lab. della Korea Maritime & Ocean University (KMOU)](https://lab.wschoi.com) ne cura la manutenzione.
Integra ambienti oceanici, robot subacquei e di superficie, sensori simulati e interfacce di controllo ROS 2 per Gazebo Sim.

## Documentazione e primi passi

**[Leggi la documentazione in italiano](https://ioes-lab.github.io/POSIM/it/)**

[Registro delle modifiche](CHANGELOG.md)

POSIM utilizza ROS 2 Lyrical e Gazebo Sim Jetty. Scegli il percorso di installazione adatto al tuo sistema operativo:

- **Ubuntu 26.04:** si consiglia l’[installazione nativa](https://ioes-lab.github.io/POSIM/it/install.html). Su Linux è disponibile anche l’[installazione con Docker](https://ioes-lab.github.io/POSIM/it/install-ubuntu-docker.html).
- **Windows:** [WSL](https://ioes-lab.github.io/POSIM/it/install-windows.html) oppure [WSL + Docker](https://ioes-lab.github.io/POSIM/it/install-windows-docker.html).
- **macOS:** [Docker + RDP](https://ioes-lab.github.io/POSIM/it/install-macos.html).

Confronta le alternative in [Scegliere l’ambiente](https://ioes-lab.github.io/POSIM/it/requirements.html),
[prepara i terminali](https://ioes-lab.github.io/POSIM/it/terminals.html)
e avvia la tua [prima simulazione](https://ioes-lab.github.io/POSIM/it/quickstart.html).

Il [catalogo di funzionalità ed esempi](https://ioes-lab.github.io/POSIM/it/features.html) raccoglie i collegamenti alle guide
su robot, mondi, sensori e controllo. Comprende anche gli [effetti subacquei e la neve marina](https://ioes-lab.github.io/POSIM/it/underwater-rendering.html)
e il [GNSS in ambiente marino](https://ioes-lab.github.io/POSIM/it/gnss.html).

## Immagini Docker

Scarica l’immagine attuale:

```bash
docker pull ioeslab/posim:latest
```

Docker seleziona la variante AMD64 o ARM64 adatta al computer. Per riprodurre un esperimento con la stessa versione,
scegli su [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags) un tag `sha-<12-character-commit>`,
che contiene i primi 12 caratteri dell’identificativo del commit.
Prima di creare il container, consulta [Scegliere l’ambiente](https://ioes-lab.github.io/POSIM/it/requirements.html)
per configurare la visualizzazione su Linux, Windows WSL o macOS.

## Applicazione correlata

[WWW-POSIM](https://www-posim.vercel.app/) combina POSIM con librerie per il terreno geografico, la pianificazione dei percorsi
e il controllo dei veicoli, aggiungendo un’interfaccia grafica e applicazioni web e desktop.
La documentazione di WWW-POSIM spiega come utilizzare queste applicazioni.

## Contribuire

Puoi segnalare problemi o proporre modifiche in [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
La [guida ai contributi](https://ioes-lab.github.io/POSIM/it/contributing.html) descrive i controlli da eseguire durante lo sviluppo
e le modalità di aggiornamento della documentazione.

## Licenza e attribuzione

POSIM include codice di [Project DAVE](https://github.com/Field-Robotics-Lab/dave), conservandone le indicazioni sugli autori e sul copyright.
Il codice scritto per POSIM è distribuito con [licenza Apache 2.0](LICENSE).
Il codice e le risorse di terze parti mantengono le condizioni originali.
La dipendenza esterna Wave Sim viene compilata dal proprio repository upstream ed è soggetta alla GPL;
consulta le [note su Wave Sim e sulla distribuzione](extras/surface/README.md#licensing).
