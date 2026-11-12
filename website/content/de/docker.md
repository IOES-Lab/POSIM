# Docker-Umgebung

Die POSIM-Images enthalten Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, ArduRover, MAVROS und QGroundControl. Wählen Sie zunächst die [Anleitung für Ihre Umgebung](requirements.md), um Anzeige und Grafiktreiber einzurichten.

## Ein veröffentlichtes Image herunterladen

Die Images liegen auf [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags). Docker wählt automatisch die passende Variante für AMD64 oder ARM64.

{{CODE:docker:0}}

| Tag | Verwendung |
|---|---|
| `latest` | Veröffentlichter Build des Hauptzweigs für beide Architekturen |
| `sha-<12-character-commit>` | Bestimmter Quellstand; übernehmen Sie einen vorhandenen Tag von Docker Hub |
| SHA-Tag mit `-amd64` oder `-arm64` | Ausdrückliche Auswahl einer Architektur |
| Veröffentlichte Versionsnummer | Ein mit einem Release-Tag gekennzeichneter Stand |

Das Image-Label enthält die vollständige Quellrevision. Halten Sie zusätzlich den Digest fest, wenn ein Versuch genau dasselbe Image verwenden soll.

{{CODE:docker:1}}

Verwenden Sie die Dokumentation zum gewählten Stand. Ein neu heruntergeladenes Image verändert vorhandene Container nicht.

## Ein Image aus dem Quellcode bauen

Führen Sie diese Befehle auf dem Host aus:

{{CODE:docker:2}}

Für AMD64:

{{CODE:docker:3}}

Für ARM64:

{{CODE:docker:4}}

Ersetzen Sie in der Installationsanleitung `ioeslab/posim:latest` durch den lokalen Tag. Beide Images bauen den vorhandenen Quellcode und die festgelegte Version der Wellenbibliothek. ARM64 enthält außerdem einen XFCE-Desktop und einen RDP-Server.

## Die Umgebung öffnen

- [Linux mit Docker](install-ubuntu-docker.md): Anzeige auf dem Linux-Desktop.
- [Windows mit WSL und Docker](install-windows-docker.md): Linux-Fenster über WSLg.
- [macOS mit Docker und RDP](install-macos.md): Verbindung zum Linux-Desktop im Container.

Alle drei Anleitungen erstellen einen Container namens `posim`. Unter [Terminals vorbereiten](terminals.md) erfahren Sie, wo Sie Befehle eingeben und den Arbeitsbereich laden.

## Eigene Dateien aufbewahren

Der Arbeitsbereich liegt bei AMD64 unter `/opt/posim_ws` und bei ARM64 unter `/home/docker/posim_ws`. Die Variablen `POSIM_WS` oder `POSIM_UNDERLAY` geben diesen Pfad an.

Speichern Sie eigene SDF-Dateien und Aufzeichnungen in `/work`. Dieses Verzeichnis gehört zum Volume `posim-data`. Andere Dateien bleiben nach einem Stopp erhalten, werden aber zusammen mit dem Container gelöscht. Sichern Sie Quellcodeänderungen in Git oder kopieren Sie sie vor dem Austausch nach außen. Das Volume bleibt bestehen, bis es ausdrücklich entfernt wird.

## Stoppen, fortsetzen und aktualisieren {#update-an-existing-container}

Beenden Sie zuerst die Simulation mit **Strg+C**. Führen Sie anschließend auf dem Host aus:

{{CODE:docker:5}}

Für ein Update sichern Sie Ihre Arbeit, stoppen den Container, laden das gewünschte Image herunter und entfernen den gestoppten Container. Erstellen Sie ihn mit dem Befehl aus Ihrer Installationsanleitung erneut und verwenden Sie dabei wieder `posim-data`.

## Grafik und GPU {#graphics-and-gpu-access}

Unter Linux benötigen Sie den Host-Treiber und das [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). Unter Windows gilt die [GPU-Konfiguration für Docker Desktop mit WSL2](https://docs.docker.com/desktop/features/gpu/). CUDA-Sonar benötigt außerdem Bibliotheken im Image; siehe [Sonar konfigurieren](sonar-tuning.md). Die ersten Schritte verwenden Software-Rendering, das langsamer sein kann.
