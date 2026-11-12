# Windows: WSL und Docker

Docker Desktop führt POSIM aus; WSLg zeigt Gazebo auf dem Windows-Desktop an. Führen Sie die Docker-Befehle dieser Seite im **Ubuntu-Terminal innerhalb von WSL** aus.

## 1. WSL und Grafik vorbereiten

Folgen Sie den ersten beiden Schritten der [Windows-Installation](install-windows.md): Ubuntu 26.04 in WSL2 installieren, anschließend WSL und den **Windows-Grafiktreiber** aktualisieren. Das gilt auch für NVIDIA-Karten. Installieren Sie keinen Linux-Grafiktreiber in WSL.

Installieren Sie [Docker Desktop mit WSL2](https://docs.docker.com/desktop/features/wsl/) und aktivieren Sie **Settings → Resources → WSL Integration → Ubuntu-26.04**. Verwenden Sie dessen Docker Engine; eine zusätzliche Engine in Ubuntu ist nicht nötig.

Prüfen Sie den Server und den Display-Socket:

{{CODE:install-windows-docker:0}}

Fehlt der Socket, schließen Sie zunächst die [WSLg-Einrichtung](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps) ab.

## 2. POSIM herunterladen

{{CODE:install-windows-docker:1}}

Versionen und eigene Builds beschreibt die Seite [Docker-Umgebung](docker.md). Die folgende Einrichtung verwendet Software-Rendering. Für unterstützte NVIDIA-GPUs gelten die [Voraussetzungen für den GPU-Zugriff](docker.md#graphics-and-gpu-access).

## 3. Container erstellen

{{CODE:install-windows-docker:2}}

Speichern Sie eigene Dateien unter `/work` im Volume `posim-data`. Mit `docker stop posim` halten Sie den Container an, mit `docker start posim` starten Sie ihn wieder.

Öffnen Sie für die weiteren Beispiele [zwei Ubuntu-Terminals](terminals.md) und verbinden Sie beide mit diesem Container. Gazebo erscheint direkt als Windows-Fenster.

## 4. Demo-Welt starten

Öffnen Sie eine Sitzung im Container:

{{CODE:install-windows-docker:3}}

Prüfen Sie die Installation mit der Demo:

{{CODE:install-windows-docker:4}}

Gazebo sollte die Ozeanwelt anzeigen. Lassen Sie das Terminal geöffnet; **Strg+C** beendet die Simulation. Fahren Sie mit Ihrem [ersten Roboter](quickstart.md) fort.

Starten Sie die Linux-Version von QGC im selben POSIM-Container. Öffnen Sie QGC weder auf dem Windows-Host noch außerhalb des Containers in WSL. Folgen Sie der [QGC-Einrichtung](qgroundcontrol.md).
