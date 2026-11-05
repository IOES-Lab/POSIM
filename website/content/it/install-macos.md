# macOS: Docker e RDP

Su un Mac con Apple Silicon, avvia l’immagine ARM64 di POSIM e collegati al desktop Linux tramite RDP. Gazebo e i terminali degli esempi vengono eseguiti in quel desktop.

## 1. Preparare Docker e il client RDP

Installa e avvia [Docker Desktop per Mac](https://docs.docker.com/desktop/setup/install/mac-install/). Installa anche [Windows App per macOS](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps) per il collegamento remoto.

Nell’app Terminale di macOS:

{{CODE:install-macos:0}}

Apple Silicon deve restituire `arm64`. Il secondo comando deve mostrare la versione del server Docker.

## 2. Scaricare e creare il container

Docker seleziona automaticamente ARM64:

{{CODE:install-macos:1}}

La porta RDP è accessibile solo da questo Mac. Salva modelli e registrazioni in `/work` per conservarli nel volume `posim-data`. La [guida Docker](docker.md) spiega le versioni delle immagini e la compilazione dai sorgenti.

## 3. Collegarsi al desktop Linux

In Windows App aggiungi un PC con indirizzo **127.0.0.1:3389**. Nella schermata xrdp scegli **Xorg** e accedi con nome utente **docker** e password **docker**.

Apri un terminale nel desktop Linux e cambia la password di questo account di sviluppo locale:

{{CODE:install-macos:2}}

Esegui qui i comandi POSIM. Usa il Terminale di macOS per gestire Docker. [Preparare i terminali](terminals.md) spiega anche dove si trova il workspace.

## 4. Arrestare e riprendere

Chiudere RDP non ferma la simulazione. Premi prima **Ctrl+C** nel terminale che la esegue. Poi, da macOS:

{{CODE:install-macos:3}}

Ricollegati con Windows App dopo aver avviato il container. Il rendering Linux è software; le scene complesse possono essere lente. Per il sonar, inizia con CPU dopo aver [compilato e verificato i pacchetti](sonar-tuning.md#build-the-sonar-packages). Metal è disponibile per i processi macOS nativi, non per questo container Linux. Le [telecamere e il sensore di pressione](features.md) sono utili per le prime verifiche.

## 5. Verificare l’installazione

In un terminale **del desktop RDP**, esegui la verifica finale:

{{CODE:install-macos:4}}

Gazebo deve mostrare il mondo oceanico. Lascia il terminale aperto durante l’esecuzione. Premi **Ctrl+C** per fermare la simulazione, poi passa al [primo robot](quickstart.md).

{{MEDIA:install-macos}}
