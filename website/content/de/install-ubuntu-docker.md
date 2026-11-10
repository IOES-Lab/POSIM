# Linux: Installation mit Docker

Verwenden Sie das veröffentlichte POSIM-Image, wenn Ihr Desktop eine andere Ubuntu-Version oder Linux-Distribution nutzt. Diese Anleitung gilt für **AMD64 auf einem x86-64-Rechner** mit X11 oder XWayland. Für Ubuntu 26.04 empfehlen wir die [native Installation](install.md).

## 1. Docker und Anzeige vorbereiten

Installieren Sie [Docker Engine für Ihre Distribution](https://docs.docker.com/engine/install/). Für Ubuntu gibt es eine [eigene Anleitung](https://docs.docker.com/engine/install/ubuntu/). Falls Ihr Konto `docker` nicht ausführen kann, erledigen Sie die [Schritte nach der Installation](https://docs.docker.com/engine/install/linux-postinstall/).

Öffnen Sie ein Terminal in Ihrer Desktop-Sitzung und installieren Sie `xauth`. Unter Ubuntu:

{{CODE:install-ubuntu-docker:0}}

Prüfen Sie die Display-Adresse und die Serverversion:

{{CODE:install-ubuntu-docker:1}}

## 2. POSIM herunterladen

{{CODE:install-ubuntu-docker:2}}

Der [Docker-Leitfaden](docker.md) erklärt die Auswahl eines Quellstands und lokale Builds. Der erste Start nutzt Software-Rendering und kann langsamer sein als eine native Installation mit GPU.

## 3. Den Zugriff auf die Anzeige erlauben

Kopieren Sie die X11-Berechtigung Ihrer Sitzung in eine Datei, die nur Ihr Konto lesen kann:

{{CODE:install-ubuntu-docker:3}}

Der letzte Befehl prüft, ob die Datei Daten enthält. Falls er fehlschlägt, prüfen Sie die Zugehörigkeit des Terminals zur Desktop-Sitzung und die Verfügbarkeit von XWayland. Korrigieren Sie die Berechtigung, bevor Sie fortfahren.

## 4. Den Container erstellen

{{CODE:install-ubuntu-docker:4}}

Speichern Sie eigene Modelle und Aufzeichnungen unter `/work` im Volume `posim-data`. Mit `docker stop posim` und `docker start posim` stoppen und starten Sie den Container. Falls die Anzeige nach einer neuen Linux-Anmeldung nicht mehr funktioniert, erneuern Sie die Berechtigungsdatei.

Für die Übungen [bereiten Sie zwei Terminals vor](terminals.md), die beide mit diesem Container verbunden sind. Lesen Sie vor der Nutzung von Hardwarebeschleunigung oder CUDA-Sonar die [GPU-Einrichtung](docker.md#graphics-and-gpu-access).

## 5. Die Beispielwelt starten

Öffnen Sie eine Sitzung im Container:

{{CODE:install-ubuntu-docker:5}}

Führen Sie darin den abschließenden Test aus:

{{CODE:install-ubuntu-docker:6}}

Gazebo sollte die Ozeanwelt öffnen. Lassen Sie das Terminal während der Simulation geöffnet und beenden Sie sie mit **Strg+C**. Danach folgt [Ihre erste Simulation](quickstart.md).
