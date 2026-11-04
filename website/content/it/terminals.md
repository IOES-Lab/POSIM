# Preparare i terminali

Il **terminale** è una finestra in cui si inseriscono comandi e si leggono i risultati. Le guide usano due normali finestre: il **terminale A** esegue la simulazione; il **terminale B** permette di leggere i sensori o inviare comandi. Le lettere indicano il ruolo delle finestre, non programmi diversi.

## Dove aprire le due finestre

| Installazione | Terminali A e B |
|---|---|
| [Ubuntu nativo](install.md) | Due finestre dell’applicazione Terminale di Ubuntu |
| [Docker su Linux](install-ubuntu-docker.md) | Due terminali Linux, entrambi nello stesso container |
| [WSL su Windows](install-windows.md) | Due schede Ubuntu in Windows Terminal, nella stessa distribuzione WSL |
| [WSL con Docker](install-windows-docker.md) | Due schede Ubuntu, entrambe nello stesso container |
| [Docker e RDP su macOS](install-macos.md) | Due terminali nel desktop Linux aperto tramite RDP |

Con Docker su Linux o Windows, entra nel container `posim` da ciascun terminale del computer:

{{CODE:terminals:0}}

Il desktop RDP usato su macOS si trova già nel container: esegui lì i comandi ROS. PowerShell e Terminale di macOS servono per i comandi del sistema host, per esempio l’avvio dei container.

## Che cos’è il workspace

Il **workspace ROS** raccoglie il codice e i pacchetti compilati. `src/` contiene i sorgenti, `build/` i file intermedi della compilazione e `install/` i pacchetti caricati da ROS.

| Installazione | Percorso |
|---|---|
| Ubuntu o WSL nativo | `~/posim_ws` |
| Docker AMD64 | `/opt/posim_ws` |
| Docker ARM64 con RDP | `/home/docker/posim_ws` |

`~` indica la cartella personale dell’utente Linux. La variabile `POSIM_WS` memorizza il percorso del workspace per gli esempi successivi.

## Caricare l’ambiente in A e B

**Ubuntu o WSL nativo:**

{{CODE:terminals:1}}

**Docker, anche nel desktop RDP:**

{{CODE:terminals:2}}

`source` carica i percorsi e le impostazioni necessari per trovare ROS e POSIM. Ripeti questi passaggi in ogni nuovo terminale.

## Verificare i percorsi

{{CODE:terminals:3}}

Entrambi i percorsi devono trovarsi nella cartella `install/` del workspace. Se manca un pacchetto, torna alla guida d’installazione. Se `ros_gz_sim` si trova sotto `/opt/ros/lyrical`, non è stata caricata la versione del workspace.

Prosegui con la [prima simulazione](quickstart.md). Un comando che continua a stampare messaggi è ancora in esecuzione. Fermalo con **Ctrl+C nel suo terminale** e attendi che ricompaia il prompt.
