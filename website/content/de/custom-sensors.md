# Sensoren hinzufügen und einstellen

Ein Sensor benötigt seine Modelldefinition, das aktualisierende System und eine Verbindung zum Empfänger. Fügen Sie jeweils einen Sensor hinzu und prüfen Sie seine Messwerte.

## Arbeitsumgebung

Nutzen Sie die [vorbereiteten Linux-Terminals](terminals.md) unter nativem Ubuntu, WSL oder Docker. Auf macOS arbeiten Sie im RDP-Desktop. Bearbeiten und bauen Sie dort; `POSIM_WS` bezeichnet diesen Arbeitsbereich.

## Einbaupunkt wählen

| Messung | Platzierung im Modell | Weltsystem / ROS-Verbindung |
|---|---|---|
| IMU | `link/sensor type="imu"` | IMU-System; Bridge von `gz.msgs.IMU` zu ROS `Imu` |
| [Unterwasserkamera](camera.md) | RGBD-Sensor und Effekt-Plugin | Sensors-System mit Renderer und Kamera-Topics |
| [DVL](dvl.md) | DVL-Sensor und Strahlkonfiguration | DVL-System und mitgelieferte Bridge für Akustiknachrichten |
| [Druck](pressure.md) | Plugin direkt unter `model` | Direkte ROS-Ausgabe von Druck und Tiefe |
| [Marine-GNSS](gnss.md) | Empfänger-Plugin am Modell | Gazebo-Nachrichten vom Typ `gz.msgs.Double_V`; für ROS ist ein separater Adapter erforderlich |
| [USBL](usbl.md) | Transceiver- und Transponder-Plugins | Passende akustische IDs und ROS-Schnittstellen |
| [Multibeam-Sonar](sonar.md) | Benutzerdefinierter Sensor unter einem Link | Sensors-Renderer, Multibeam-Weltsystem und CPU/WGPU/CUDA |

Verwenden Sie SDF-Beispiel und Parametertabelle der jeweiligen Referenz. Die Platzierung ist entscheidend: Weltsysteme gehören in die Welt; Modell-Plugins benötigen ihre zugehörige Modellentität.

## IMU in die Beispielwelt einbauen

Öffnen Sie `calm_water.sdf` aus [Welt erstellen](custom-worlds.md). Ergänzen Sie dieses System direkt unter `<world>` neben dem Physiksystem:

{{CODE:custom-sensors:0}}

Fügen Sie unter `<link name="base_link">` des Modells `pressure_station` Folgendes ein:

{{CODE:custom-sensors:1}}

Die Pose gilt relativ zum Link, in Metern und Radiant. Die Rate bezieht sich auf Simulationszeit. Da die Station feststeht, sollte die Winkelgeschwindigkeit nahe null liegen; die Beschleunigung enthält die Schwerkraftreaktion.

## Verbinden und beobachten

Stoppen Sie die bisherige Welt. Prüfen und starten Sie in Terminal A neu:

{{CODE:custom-sensors:2}}

Starten Sie in **Terminal B** mit geladener Umgebung die Bridge und lassen Sie sie laufen:

{{CODE:custom-sensors:3}}

Lesen Sie in entsprechend vorbereitetem **Terminal C** einen Messwert:

{{CODE:custom-sensors:4}}

Prüfen Sie Zeitstempel, Frame-ID, Winkelgeschwindigkeit und Beschleunigung. Ohne `--once` werden fortlaufend Werte angezeigt; Strg+C beendet die Anzeige. Stoppen Sie abschließend Bridge und Simulator. [ROS-Terminals und Topics](ros.md) erläutert die gemeinsame Umgebung.

## Rate ändern

Ändern Sie `update_rate` von `20` auf `5` und starten Sie Welt und Bridge neu. Die Zeitstempelabstände sollten von etwa **0,05 s auf 0,2 s** Simulationszeit steigen. Die in Echtzeit empfangene Rate hängt zusätzlich von Echtzeitfaktor und Transport ab.

Prüfen Sie bei Kameras Auflösung, Sichtfeld sowie Nah- und Ferngrenze gemeinsam. Prüfen Sie bei Scannern Strahlgeometrie und Reichweite. Bestätigen Sie Koordinatenrahmen und Orientierung, bevor Sie einen Sensor versetzen.

## Integration prüfen

- Modell: gültige Einbaupose, Einstellungen und eindeutiges Topic.
- Welt: erforderliches System und Renderer für bildgebende Sensoren.
- ROS: Bridge-Richtung, Nachrichtentyp, Koordinatenrahmen und QoS.
- Daten: aktuelle Zeitstempel und Reaktion auf eine kontrollierte Änderung.

Prüfen Sie den Boden mit [Höhenkartengelände](heightmaps.md). Für Sonarvergleiche verwenden Sie [Einrichtung und Leistung](sonar-tuning.md) zusammen mit der [Parameterreferenz](sonar.md).
