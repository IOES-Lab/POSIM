# ROS-Terminals und Topics

Der Simulator läuft in A; B dient zum Lesen von Daten und Senden von Befehlen. Beide Terminals müssen dieselbe ROS-Distribution und denselben POSIM-Arbeitsbereich laden.

## Datenfluss verstehen

<div class="docs-flow"><div><strong>ROS-Programme und Befehle</strong><span>Abonnements, Services und Steuerbefehle</span></div><div><strong>ROS-Schnittstelle</strong><span>ros_gz-Bridge oder Publisher des Plugins</span></div><div><strong>Gazebo und POSIM-Plugins</strong><span>Physik, Zustand und Messwerte</span></div></div>

Messwerte gelangen zum ROS-Programm, Befehle zum Simulator. Ein Gazebo-Topic ist in ROS verfügbar, wenn eine Bridge oder ein ROS-fähiges Plugin es veröffentlicht. Der Roboterstart lädt die passende Bridge-Konfiguration.

## A und B vorbereiten

Folgen Sie der [Terminal-Einrichtung für Ihre Installation](terminals.md). Lassen Sie die [erste Simulation](quickstart.md) in A laufen und verwenden Sie B für die folgenden Befehle.

Die Topic-Namen gelten für BlueROV2 ohne Sonar. Ersetzen Sie mit Sonar `bluerov2` durch `bluerov2_heavy_multibeam_sonar`, beim Oberflächenbeispiel durch `wamv`.

## Verbindungen untersuchen

`--no-daemon` fragt ROS direkt ab und umgeht den Cache des CLI-Daemons.

{{CODE:ros:0}}

`ros2 topic list` zeigt ROS-Topics, `gz topic -l` Gazebo-Transport-Topics. Prüfen Sie Nachrichtentyp, Publisher, Subscriber und QoS, bevor Sie ein Programm anbinden.

## Messwerte verfolgen

{{CODE:ros:1}}

Eine fast konstante Position ist möglich, solange neue Messwerte mit fortschreitenden Zeitstempeln eintreffen. **Strg+C in B** beendet die Anzeige; A simuliert weiter. Mit `--once` empfangen Sie genau eine Nachricht. Nodes mit Simulationszeit benötigen `use_sim_time:=true` und `/clock`.

## Weiteren ROS-Rechner verbinden

Verwenden Sie kompatible ROS- und Nachrichtenpakete, passende Domain-Einstellungen und ein Netzwerk, das DDS-Erkennung und Datenübertragung erlaubt. Installieren Sie `posim_interfaces` für POSIM-Nachrichten. Gibt es Publisher, aber keine empfangenen Werte, hilft die [Fehlersuche](troubleshooting.md).

Weiter geht es mit [Python](python-control.md), [Thrustern und Joysticks](robot-control.md) oder [Anzeige und Aufzeichnung](visualization.md).
