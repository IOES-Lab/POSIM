# Windows: WSL e Docker

Docker Desktop esegue POSIM e WSLg mostra Gazebo sul desktop di Windows. Esegui i comandi Docker di questa pagina nel **terminale Ubuntu all'interno di WSL**.

## 1. Preparare WSL e la grafica

Completa i primi due passaggi dell'[installazione su Windows](install-windows.md): installa Ubuntu 26.04 in WSL2, quindi aggiorna WSL e il driver grafico di **Windows**. Vale anche per le schede NVIDIA. Non installare un driver video Linux all'interno di WSL.

Installa [Docker Desktop con WSL2](https://docs.docker.com/desktop/features/wsl/) e attiva **Settings → Resources → WSL Integration → Ubuntu-26.04**. Usa il suo motore Docker; non serve un secondo motore in Ubuntu.

Verifica il server e il socket grafico:

{{CODE:install-windows-docker:0}}

Se il socket non esiste, completa la [configurazione di WSLg](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps).

## 2. Scaricare POSIM

{{CODE:install-windows-docker:1}}

La pagina [Ambiente Docker](docker.md) descrive versioni e compilazione delle immagini. Questa configurazione usa il rendering software. Per una GPU NVIDIA supportata, consulta i [requisiti di accesso alla GPU](docker.md#graphics-and-gpu-access).

## 3. Creare il container

{{CODE:install-windows-docker:2}}

Salva i tuoi file in `/work`, nel volume `posim-data`. Puoi arrestare il container con `docker stop posim` e riavviarlo con `docker start posim`.

Per gli esempi successivi, apri [due terminali Ubuntu](terminals.md) e collegali allo stesso container. Le finestre di Gazebo compaiono direttamente sul desktop di Windows.

## 4. Avviare il mondo dimostrativo

Entra nel container:

{{CODE:install-windows-docker:3}}

Verifica l'installazione:

{{CODE:install-windows-docker:4}}

Gazebo dovrebbe mostrare l'oceano. Lascia aperto il terminale; **Ctrl+C** arresta la simulazione. Prosegui con il [primo robot](quickstart.md).
