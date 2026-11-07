# Ubuntu: native Installation

Bauen Sie POSIM unter Ubuntu 26.04. Öffnen Sie die Anwendung Terminal und führen Sie die Befehlsblöcke einzeln aus. Warten Sie jeweils, bis der Schritt beendet ist. Für ein vorbereitetes Image gibt es die [Docker-Anleitung für Linux](install-ubuntu-docker.md).

## 1. Quellcode herunterladen

{{CODE:install:0}}

Behalten Sie den Verzeichnisnamen `posim` bei. Das Repository-Manifest verwendet diesen Namen.

## 2. Software installieren

Lesen Sie zuerst `src/posim/extras/ros-lyrical-gz-jetty-install.sh`. Das Skript richtet Paketquellen ein, verwendet `sudo` und führt `apt-get full-upgrade -y` aus. Eine eigene Ubuntu-Umgebung wird empfohlen.

{{CODE:install:1}}

Das Skript installiert ROS 2 Lyrical, Gazebo Jetty, ArduSub, MAVROS und die Wellenbibliotheken. Wave Sim wird als externe Abhängigkeit aus einer festgelegten Revision gebaut. Ein rekursiver POSIM-Klon ist nicht nötig.

## 3. Abhängigkeiten des Arbeitsbereichs auflösen

{{CODE:install:2}}

`--skip-existing` erhält den bereits geklonten POSIM-Quellcode. Beheben Sie gemeldete Abhängigkeitsfehler vor dem Build.

Das Manifest importiert auch eine festgelegte `ros_gz`-Revision mit Korrekturen an Bridge-Lebensdauer und POSIX-Prozessstart. Ein vorhandenes `src/ros_gz` wird nicht ersetzt: Vergleichen Sie Herkunft und Revision mit dem Manifest oder verwenden Sie einen neuen Arbeitsbereich.

## 4. Bauen und Umgebung laden

{{CODE:install:3}}

Beide `ros_gz`-Pfade müssen zum `install/`-Verzeichnis dieses Arbeitsbereichs gehören. Zeigt ein Pfad auf `/opt/ros/lyrical`, ist weiterhin das Systempaket ausgewählt. Laden Sie ROS und den Arbeitsbereich in jedem neuen Terminal gemäß [Terminals vorbereiten](terminals.md).

### Bei Speichermangel

`--executor sequential` baut Pakete nacheinander, begrenzt aber nicht die Compilerprozesse. Beides lässt sich so begrenzen:

{{CODE:install:5}}

### Arbeitsbereich aktualisieren

Sichern Sie eigene Änderungen zuerst als Git-Commit. Notieren Sie die Revision, laden Sie die Änderungen und wiederholen Sie anschließend Abhängigkeitsauflösung und Build:

{{CODE:install:6}}

Fahren Sie mit der [ersten Simulation](quickstart.md) fort. Für Sonar gilt die [Build- und Einrichtungsanleitung für CPU/WGPU/CUDA](sonar-tuning.md).

## 5. Demowelt starten

Die abschließende Prüfung öffnet die Meereswelt in Gazebo. Beim ersten Start können Fuel-Modelle heruntergeladen werden. Beenden Sie die Simulation mit **Strg+C**, bevor Sie mit [den Terminals](terminals.md) fortfahren.

{{CODE:install:7}}
