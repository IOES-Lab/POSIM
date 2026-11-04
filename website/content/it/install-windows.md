# Windows: installazione in WSL

Installa POSIM in Ubuntu 26.04 tramite Windows Subsystem for Linux 2 (WSL2). WSLg mostra Gazebo sul desktop di Windows. Per usare un’immagine già pronta, scegli [WSL con Docker](install-windows-docker.md).

## 1. Installare WSL e Ubuntu

Apri PowerShell come amministratore:

{{CODE:install-windows:0}}

Riavvia Windows se richiesto. Apri Ubuntu e crea il tuo account Linux. Da PowerShell verifica che la distribuzione utilizzi WSL 2:

{{CODE:install-windows:1}}

Se WSL è già installato o la procedura non riesce, consulta la [guida Microsoft](https://learn.microsoft.com/it-it/windows/wsl/install).

## 2. Preparare il driver grafico e WSLg

Installa o aggiorna il **driver Windows** della GPU Intel, AMD o NVIDIA. Il passaggio serve anche con NVIDIA. WSLg usa questo driver per l’accelerazione grafica; segui la [guida alle applicazioni grafiche](https://learn.microsoft.com/it-it/windows/wsl/tutorials/gui-apps).

Con NVIDIA consulta [CUDA su WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html): installa il driver Windows, **non un driver video Linux dentro WSL**. Per compilare il plugin sonar occorre anche il toolkit CUDA.

Aggiorna WSL da PowerShell:

{{CODE:install-windows:2}}

Salva il lavoro nei terminali WSL prima di un eventuale riavvio richiesto, quindi riapri Ubuntu.

## 3. Installare POSIM nel terminale Ubuntu

Usa il profilo **Ubuntu** di Windows Terminal per i comandi Linux. Verifica la distribuzione:

{{CODE:install-windows:3}}

Segui [Ubuntu: installazione nativa](install.md), dal download dei sorgenti alla compilazione. Crea il workspace in `~/posim_ws`, nel filesystem Linux. Il programma d’installazione usa `sudo` e aggiorna i pacchetti Ubuntu.

Se i topic delle immagini sono visibili ma i fotogrammi non arrivano, consulta la [risoluzione dei problemi](troubleshooting.md). [Preparare i terminali](terminals.md) spiega le due finestre degli esempi successivi.

## 4. Avviare il mondo dimostrativo

Esegui questi comandi nel terminale Ubuntu. Gazebo dovrebbe aprirsi sul desktop di Windows e mostrare la scena oceanica. Il primo avvio può scaricare modelli da Fuel. Ferma la simulazione con Ctrl+C prima di avviare un altro esempio.

{{CODE:install-windows:4}}
