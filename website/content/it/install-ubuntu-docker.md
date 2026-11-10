# Linux: installazione con Docker

Usa l’immagine pubblicata di POSIM se il desktop esegue un’altra versione di Ubuntu o una distribuzione diversa. Questa guida riguarda **AMD64 su un computer x86-64**, con X11 o XWayland. Su Ubuntu 26.04 consigliamo l’[installazione nativa](install.md).

## 1. Preparare Docker e lo schermo

Installa [Docker Engine per la tua distribuzione](https://docs.docker.com/engine/install/). Per Ubuntu sono disponibili [istruzioni dedicate](https://docs.docker.com/engine/install/ubuntu/). Se l’account non può eseguire `docker`, completa la [configurazione dopo l’installazione](https://docs.docker.com/engine/install/linux-postinstall/).

Apri un terminale nella sessione desktop e installa `xauth`. Su Ubuntu:

{{CODE:install-ubuntu-docker:0}}

Verifica l’indirizzo del display e la versione del server:

{{CODE:install-ubuntu-docker:1}}

## 2. Scaricare POSIM

{{CODE:install-ubuntu-docker:2}}

La [guida Docker](docker.md) spiega come scegliere una revisione o compilare le proprie modifiche. Il primo avvio usa il rendering software e può essere più lento di un’installazione nativa con GPU.

## 3. Autorizzare l’accesso al display

Copia l’autorizzazione X11 della sessione in un file leggibile solo dal tuo account:

{{CODE:install-ubuntu-docker:3}}

L’ultimo comando verifica che il file contenga dati. Se fallisce, controlla che il terminale appartenga alla sessione grafica e che XWayland sia disponibile. Risolvi il problema prima di continuare.

## 4. Creare il container

{{CODE:install-ubuntu-docker:4}}

Salva modelli e registrazioni in `/work`, nel volume `posim-data`. Usa `docker stop posim` e `docker start posim` per arrestare e riprendere il container. Se il display non funziona dopo un nuovo accesso a Linux, aggiorna il file di autorizzazione.

Per gli esercizi, [prepara due terminali](terminals.md) collegati allo stesso container. Prima di abilitare accelerazione grafica o sonar CUDA, consulta la [configurazione GPU](docker.md#graphics-and-gpu-access).

## 5. Avviare il mondo di esempio

Entra nel container:

{{CODE:install-ubuntu-docker:5}}

Al suo interno, esegui la verifica finale:

{{CODE:install-ubuntu-docker:6}}

Gazebo deve aprire il mondo oceanico. Lascia il terminale aperto durante l’esecuzione e premi **Ctrl+C** per fermarla. Prosegui con [la prima simulazione](quickstart.md).
