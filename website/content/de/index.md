# Überblick

POSIM ist eine Open-Source-Bibliothek für maritime Robotiksimulation mit ROS 2 und Gazebo. Damit lassen sich Meeresumgebungen, Überwasser- und Unterwasserroboter, simulierte Sensoren und Regelungsversuche ausführen. POSIM geht aus [Project DAVE](https://field-robotics-lab.github.io/dave.doc/) hervor, das auf dem [UUV Simulator](https://uuvsimulator.github.io/) aufbaut.

{{TUTORIALS}}

{{OVERVIEW}}

## Hier beginnen

[Wählen Sie eine Umgebung](requirements.md): [Ubuntu nativ](install.md), [Linux mit Docker](install-ubuntu-docker.md), [Windows mit WSL](install-windows.md), [WSL mit Docker](install-windows-docker.md) oder [macOS mit Docker und RDP](install-macos.md). Empfohlen wird die native Installation unter Ubuntu.

[Bereiten Sie anschließend die Terminals vor](terminals.md) und starten Sie Ihre [erste Simulation](quickstart.md).

- ROS: **ROS 2 Lyrical** — LTS; Supportende: [Mai 2031](https://docs.ros.org/en/lyrical/Releases/Release-Lyrical-Luth.html)
- Gazebo-Sim-Simulator: **Gazebo Jetty** — LTS; Supportende: [Mai 2031](https://gazebosim.org/docs/latest/releases/#release-list)

## Simulationsmöglichkeiten

Unter [Funktionen und Beispiele](features.md) finden Sie die Implementierungen und die passenden Anleitungen.

| Aufgabe | Anleitungen |
| --- | --- |
| Meer und Meeresboden vorbereiten | [Welten](worlds.md), [Strömungen](currents.md), [Unterwassereffekte](underwater-rendering.md) |
| Fahrzeug auswählen | [ROVs](rovs.md), [Slocum](gliders.md), [Überwasserroboter](surface.md) |
| Sensoren beobachten | [Kamera](camera.md), [GNSS](gnss.md), [Druck](pressure.md), [DVL](dvl.md), [USBL](usbl.md), [Sonar](sonar.md) |
| Versuche steuern und aufzeichnen | [ROS](ros.md), [Python](python-control.md), [Thruster und Joystick](robot-control.md), [Sensordaten](visualization.md) |

## Aufbau einer Simulation

1. Umgebung und Weltsysteme werden in SDF definiert.
2. Das Modell beschreibt Links, Kollisionen, Trägheiten und Sensoren.
3. Eine Launch-Datei startet Gazebo, das Modell und die ROS-Bridges.
4. ROS-Nodes empfangen Messwerte und senden Steuerbefehle.

Die weiterführenden Anleitungen zeigen, wie Sie eine [Welt erstellen](custom-worlds.md), einen [Roboter hinzufügen](custom-robots.md) und [Sensoren konfigurieren](custom-sensors.md). Jede Plugin-Referenz enthält einen Schnellstart, einen SDF-Ausschnitt und die zugehörigen Parameter.

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) verbindet POSIM mit POSIM-Terrain für geografisches Gelände, POSIM-Routing für die Routenplanung und POSIM-Control für die Fahrzeugsteuerung. Eine grafische Oberfläche sowie Web- und Desktop-Anwendungen erleichtern die Szenenvorbereitung, Roboterbedienung und Anzeige von Simulationsdaten.

Das Video stellt die wichtigsten Funktionen vor. Installation und Bedienung beschreibt die [WWW-POSIM-Dokumentation](https://www-posim.vercel.app/guide/en.html).

{{INTRO_VIDEO}}

[Einführung auf YouTube ansehen](https://www.youtube.com/watch?v=_cQtl5JzN2E).
