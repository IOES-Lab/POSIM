# Einen Roboter hinzufügen

Verwenden Sie einen ähnlich aufgebauten POSIM-Roboter als Ausgangspunkt. SDF beschreibt Mechanik und Systeme; die ROS-Konfiguration verbindet Messungen und Befehle.

## Arbeitsumgebung

Nutzen Sie die [vorbereiteten Linux-Terminals](terminals.md) unter nativem Ubuntu, WSL oder Docker. Auf macOS arbeiten Sie im Terminal des RDP-Desktops. Bearbeiten und bauen Sie dort; `POSIM_WS` ist der Pfad dieses Arbeitsbereichs.

## 1. Referenzmodell kopieren

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) verwendet Gazebo-Thruster. [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) bindet ArduSub/MAVROS ein. [Slocum](gliders.md) zeigt einen weiteren Fahrzeugaufbau.

Kopieren Sie REXROV ausgehend von `$POSIM_WS` vollständig, damit Meshes, Bridge-Konfiguration und Topic-Namen zusammenbleiben:

{{CODE:custom-robots:0}}

Speichern Sie das folgende Programm als `copy_robot.py` und führen Sie `python3 copy_robot.py` aus. Existiert `my_robot` bereits, bricht es ab.

{{CODE:custom-robots:1}}

Die erzeugte Struktur enthält:

{{CODE:custom-robots:2}}

`my_robot` ist die gewählte Kennung. Der Start sucht `description/<namespace>/model.sdf` und die passende Konfiguration.

## 2. Mechanik und Ressourcen festlegen

Behalten Sie die kopierte Mechanik beim ersten Start bei. Prüfen Sie bei Rumpfänderungen gemeinsam:

| SDF-Pfad | Einheit | Prüfung |
|---|---|---|
| `model/@name`, Link- und Gelenknamen | Kennung | Übereinstimmung mit Plugins und ROS-Konfiguration |
| `link/inertial/mass` | kg | Positive Masse jedes beweglichen Links |
| `link/inertial/pose` | m, rad | Schwerpunkt und Orientierung des Trägheitsrahmens |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | Momente um dessen Achsen |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | Nebendiagonalelemente; physikalisch gültiger Gesamttensor |
| `link/visual`, `link/collision` | m, rad | Maßstab und Ausrichtung beider Geometrien |
| `joint/axis`, Thruster-Einstellungen | pluginspezifisch | Antriebsrichtung und Kraftverteilung |

STL liefert Geometrie. Masse und Dynamik gehören in die SDF; prüfen Sie Mesh-Einheiten und Maßstab. Folgen Sie den installierten Ressourcen-URIs der Beispiele. Vereinfachen Sie die Kollisionsform, wenn visuelle Details den Kontakt unnötig teuer machen.

## 3. Systeme und ROS verbinden

Passen Sie Hydrodynamik, Auftrieb, Schubachsen und Sensorposen an das Fahrzeug an. Die Koeffizienten müssen zu seinen physikalischen Eigenschaften passen.

Verwenden Sie [Strömungen](currents.md), [Sensoren hinzufügen](custom-sensors.md) und die dort verlinkten Plugin-Referenzen. Für Breiten- und Längengrade legen Sie den Ursprung über [sphärische Koordinaten](coordinates.md) fest.

Passen Sie `config/<namespace>/robot_config.py` an: Ändern Sie Entitäts- und Gelenknamen, Topics, Koordinatenrahmen und Ports gemeinsam. Das Paket installiert bereits `description`, `meshes`, `config` und die Ressourcenpfade. Bearbeiten Sie Quellen, keine erzeugten Dateien unter `install/`.

## 4. Bauen und ausprobieren

Im Arbeitsbereich mit geladener Umgebung:

{{CODE:custom-robots:3}}

Prüfen Sie jede Achse mit kleinen Befehlen und starten Sie auch einmal außerhalb des Quellverzeichnisses:

- Installierte Ressourcenpfade.
- Masse, Trägheit und Gleichgewichtslage.
- Kollisionsgeometrie und Kontaktverhalten.
- Koordinatenrahmen und Nachrichteninhalte.

Fragen Sie das umbenannte Modell in einem weiteren vorbereiteten Terminal ab:

{{CODE:custom-robots:4}}

Sie sollten Position und Geschwindigkeit empfangen. Bringen Sie zuerst die Kopie zum Laufen, ändern Sie danach die Mechanik und anschließend Sensoren oder Antrieb. Vergleichen Sie Versuche über [Sensordaten](visualization.md); fehlende Ressourcen behandelt die [Fehlerbehebung](troubleshooting.md).
