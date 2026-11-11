# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![POSIM-Docker-Images erstellen und veröffentlichen](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM ist eine Open-Source-Bibliothek für Simulationen in der maritimen Robotik. Das [IOES Lab. an der Korea Maritime & Ocean University (KMOU)](https://lab.wschoi.com) betreut die Bibliothek.
Sie verbindet Meeresumgebungen, Unterwasser- und Oberflächenroboter, simulierte Sensoren und ROS-2-Steuerschnittstellen in Gazebo Sim.

## Dokumentation und Einstieg

**[Deutsche Dokumentation lesen](https://ioes-lab.github.io/POSIM/de/)**

[Änderungsprotokoll](CHANGELOG.md)

POSIM verwendet ROS 2 Lyrical und Gazebo Sim Jetty. Wählen Sie den passenden Installationsweg für Ihr Betriebssystem:

- **Ubuntu 26.04:** Empfohlen wird die [native Installation](https://ioes-lab.github.io/POSIM/de/install.html). Unter Linux steht außerdem die [Installation mit Docker](https://ioes-lab.github.io/POSIM/de/install-ubuntu-docker.html) zur Verfügung.
- **Windows:** [WSL](https://ioes-lab.github.io/POSIM/de/install-windows.html) oder [WSL + Docker](https://ioes-lab.github.io/POSIM/de/install-windows-docker.html).
- **macOS:** [Docker + RDP](https://ioes-lab.github.io/POSIM/de/install-macos.html).

Vergleichen Sie die Möglichkeiten unter [Umgebung auswählen](https://ioes-lab.github.io/POSIM/de/requirements.html).
Anschließend [bereiten Sie die Terminals vor](https://ioes-lab.github.io/POSIM/de/terminals.html)
und starten Ihre [erste Simulation](https://ioes-lab.github.io/POSIM/de/quickstart.html).

Der [Funktions- und Beispielkatalog](https://ioes-lab.github.io/POSIM/de/features.html) führt zu Anleitungen für Roboter,
Welten, Sensoren und Steuerung. Dazu gehören auch [Unterwassereffekte und Meeresschnee](https://ioes-lab.github.io/POSIM/de/underwater-rendering.html)
sowie [GNSS in maritimen Umgebungen](https://ioes-lab.github.io/POSIM/de/gnss.html).

## Docker-Images

Laden Sie das aktuelle Image herunter:

```bash
docker pull ioeslab/posim:latest
```

Docker wählt die passende Variante für AMD64 oder ARM64. Um ein Experiment mit derselben Version zu wiederholen,
wählen Sie auf [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags) einen Tag im Format `sha-<12-character-commit>`.
Er enthält die ersten zwölf Zeichen der Commit-ID.
Richten Sie vor dem Erstellen des Containers die Bildschirmausgabe für Linux, Windows WSL oder macOS ein;
die passenden Anleitungen finden Sie unter [Umgebung auswählen](https://ioes-lab.github.io/POSIM/de/requirements.html).

## Zugehörige Anwendung

[WWW-POSIM](https://www-posim.vercel.app/) verbindet POSIM mit Bibliotheken für geografisches Gelände, Routenplanung
und Fahrzeugsteuerung. Darauf bauen eine grafische Benutzeroberfläche sowie Web- und Desktop-Anwendungen auf.
Deren Bedienung beschreibt die eigene Dokumentation von WWW-POSIM.

## Mitwirken

Unter [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM) können Sie Fehler melden und Änderungen vorschlagen.
Der [Leitfaden für Beiträge](https://ioes-lab.github.io/POSIM/de/contributing.html) beschreibt die Prüfungen während der Entwicklung
und die Pflege der Dokumentation.

## Lizenz und Urheberhinweise

POSIM enthält Quellcode aus [Project DAVE](https://github.com/Field-Robotics-Lab/dave) unter Beibehaltung der Autoren- und Copyright-Hinweise.
Für POSIM geschriebener Quellcode steht unter der [Apache-Lizenz 2.0](LICENSE).
Für Quellcode und Ressourcen Dritter gelten weiterhin deren ursprüngliche Bedingungen.
Die externe Abhängigkeit Wave Sim wird aus ihrem Upstream-Repository gebaut und unterliegt der GPL.
Weitere Angaben enthalten die [Lizenzhinweise zu Wave Sim](extras/surface/README.md#licensing).
