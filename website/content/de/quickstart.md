# Erste Simulation

Starten Sie einen REXROV in einer Ozeanwelt, empfangen Sie Messwerte und beenden Sie die Simulation. Verwenden Sie für diesen ersten Versuch das vorgegebene Modell und die vorgegebene Welt.

## Terminals vorbereiten

Schließen Sie die [Installation](requirements.md) ab und öffnen Sie [zwei Terminals mit geladener ROS- und POSIM-Umgebung](terminals.md). Terminal A führt den Simulator aus; B liest Daten. Bei Docker erscheint Gazebo auf dem eingerichteten Linux-, WSLg- oder RDP-Desktop.

## 1. Roboter und Welt starten

In **Terminal A**:

{{CODE:quickstart:0}}

REXROV wird fünf Meter unter z = 0 eingesetzt; zugleich starten seine ROS-Bridges. Der Befehl startet auch die Welt. Führen Sie ihn daher als eine Simulation aus und lassen Sie A geöffnet.

## 2. Position lesen

In **Terminal B**:

{{CODE:quickstart:1}}

Es sollten laufend Messwerte eintreffen. Eine nahezu konstante Position ist normal, wenn sich der Roboter kaum bewegt. **Strg+C in B** beendet die Anzeige.

## 3. Simulationszeit prüfen

{{CODE:quickstart:2}}

Die Uhr sollte weiterlaufen. Beenden Sie die Anzeige mit Strg+C. Bleibt die Zeit stehen oder fehlen Nachrichten, hilft die [Fehlersuche](troubleshooting.md).

## 4. Sitzung beenden

Drücken Sie **Strg+C in A** und warten Sie auf das Ende der Prozesse. Das Beenden einer Topic-Anzeige stoppt den Simulator nicht. Beenden Sie die aktuelle Simulation, bevor Sie ein anderes Beispiel starten.

## Nächste Schritte

- [Sensordaten anzeigen und aufzeichnen](visualization.md): Bilder betrachten und Beobachtungen speichern.
- [Mit Python steuern](python-control.md): Zustand lesen und einen Thruster ansteuern.
- [ROVs und BlueROV2](rovs.md): ArduSub, Tastatur und Joystick.
- [Weltbibliothek](worlds.md): eine andere Umgebung auswählen.
