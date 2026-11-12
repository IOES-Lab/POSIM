# Ubuntu: installazione nativa

Compila POSIM su Ubuntu 26.04. Apri l’applicazione Terminale ed esegui un blocco alla volta, aspettando che ogni passaggio termini. Per usare un’immagine pronta, segui [Linux con Docker](install-ubuntu-docker.md).

## 1. Scaricare i sorgenti

{{CODE:install:0}}

Mantieni il nome della directory `posim`: viene usato dal manifesto dei repository.

## 2. Installare lo stack software

Leggi prima `src/posim/extras/ros-lyrical-gz-jetty-install.sh`. Lo script configura repository, usa `sudo` ed esegue `apt-get full-upgrade -y`. È consigliato un ambiente Ubuntu dedicato.

{{CODE:install:1}}

Installa ROS 2 Lyrical, Gazebo Jetty, ArduSub, ArduRover, MAVROS e le librerie delle onde. Wave Sim viene scaricato a una revisione fissata e compilato come dipendenza esterna; non serve clonare POSIM in modo ricorsivo.

## 3. Risolvere le dipendenze del workspace

{{CODE:install:2}}

`--skip-existing` conserva il repository POSIM già clonato. Controlla ogni errore di dipendenza prima della compilazione.

Il manifesto importa anche una revisione fissata di `ros_gz` con correzioni al ciclo di vita del bridge e all’avvio POSIX. Se `src/ros_gz` esiste già, rimane invariato: confronta origine e revisione con il manifesto oppure usa un workspace nuovo.

## 4. Compilare e caricare l’ambiente

{{CODE:install:3}}

Entrambi i percorsi `ros_gz` devono appartenere alla directory `install/` del workspace. Se compare `/opt/ros/lyrical`, viene ancora selezionato il pacchetto di sistema. Carica ROS e il workspace in ogni nuovo terminale, come spiegato in [Preparare i terminali](terminals.md).

### Se la memoria non basta

`--executor sequential` esegue un pacchetto alla volta, ma non limita i processi del compilatore. Per limitare entrambi:

{{CODE:install:5}}

### Aggiornare il workspace

Salva prima le tue modifiche in un commit Git. Annota la revisione, scarica gli aggiornamenti, poi ripeti la risoluzione delle dipendenze e la compilazione:

{{CODE:install:6}}

Prosegui con la [prima simulazione](quickstart.md). Per il sonar, consulta [compilazione e configurazione CPU/WGPU/CUDA](sonar-tuning.md).

## 5. Avviare il mondo dimostrativo

L’ultima verifica apre Gazebo con la scena oceanica. Al primo avvio può essere necessario scaricare modelli Fuel. Ferma la simulazione con **Ctrl+C** prima di passare alla [preparazione dei terminali](terminals.md).

{{CODE:install:7}}
