# Eine Welt erstellen

Beginnen Sie mit einer kleinen Welt und einem Drucksensor in bekannter Tiefe. So können Sie Änderungen messen, bevor Gelände und ein beweglicher Roboter hinzukommen.

## Arbeitsumgebung vorbereiten

Verwenden Sie die [vorbereiteten Linux-Terminals](terminals.md) unter nativem Ubuntu, WSL oder Docker. Auf macOS öffnen Sie das Terminal im RDP-Desktop. Bearbeiten und bauen Sie in dieser Umgebung; `POSIM_WS` bezeichnet deren Arbeitsbereich.

## Vollständige Welt speichern

Speichern Sie den folgenden Inhalt als `calm_water.sdf`. Einfache Geometrien und installierte Plugins machen zusätzliche Mesh-Downloads überflüssig.

{{CODE:custom-worlds:0}}

Die Welt enthält:

- **Physik:** maximaler Schritt 0,001 s, angestrebter Echtzeitfaktor 1 und Schwerkraft nach unten.
- **Licht und Szene:** gerichtetes Sonnenlicht, Umgebungslicht und Hintergrundfarbe.
- **Meeresboden:** feste Box bei z = −20 m mit übereinstimmender visueller und Kollisionsgeometrie.
- **Druckstation:** feste Markierung bei z = −2 m. Ihr [Druck-Plugin](pressure.md) auf Modellebene veröffentlicht Druck und geschätzte Tiefe.

Der Druck bezieht sich auf die Ebene z = 0 der Welt. Ergänzen Sie Wellen, Auftrieb und Fahrzeugbewegung, nachdem dieses Grundbeispiel funktioniert.

## Prüfen und starten

In Terminal A, im Verzeichnis der Datei:

{{CODE:custom-worlds:1}}

Die Prüfung sollte erfolgreich sein; anschließend öffnet sich Gazebo. `-r` startet die Simulationszeit. Wählen Sie `pressure_station` im Objektbaum und richten Sie die Ansicht darauf aus.

In einem weiteren Terminal mit derselben geladenen Umgebung:

{{CODE:custom-worlds:2}}

Erwartet werden ungefähr **120 938 Pa** und **2 m**. Das Plugin veröffentlicht direkt in ROS; eine zusätzliche Bridge ist hier nicht erforderlich.

## Tiefe verändern

1. Stoppen Sie Terminal A mit Strg+C.
2. Ändern Sie die Stationspose von `0 0 -2 0 0 0` auf `0 0 -4 0 0 0`.
3. Prüfen und starten Sie die Welt erneut.
4. Wiederholen Sie beide Abfragen. Erwartet werden etwa **140 551 Pa** und **4 m**.

Lassen Sie die Druckkoeffizienten unverändert. Die erwartete Zunahme beträgt **19 612,76 Pa**. Bewahren Sie beide Dateien zusammen mit Ihren Beobachtungen auf.

## Einstellungen zuordnen

| SDF-Pfad | Einheit | Funktion |
|---|---|---|
| `world/physics/max_step_size` | s | Maximaler Integrationsschritt: `0.001` |
| `world/physics/real_time_factor` | — | Zielverhältnis von Simulations- zu Echtzeit: `1.0` |
| `world/gravity` | m/s² | Schwerkraftvektor |
| `world/scene`, `world/light` | RGBA / Richtung | Hintergrund und Beleuchtung |
| `model/pose` | m, rad | Position und Orientierung |
| `model/static` | boolesch | Station und Boden festhalten |
| `model/plugin` | pluginspezifisch | [Druckparameter](pressure.md) |

Die tatsächliche Geschwindigkeit hängt von der Last ab. Sensordefinition und zugehöriges Weltsystem haben unterschiedliche Geltungsbereiche. [Sensoren hinzufügen](custom-sensors.md) ergänzt diese Datei um eine IMU und ROS-Bridge.

## Welt erweitern

- [Höhenkartengelände](heightmaps.md): Boden ersetzen und Kollision prüfen.
- [Roboter hinzufügen](custom-robots.md): Mechanik, Antrieb und ROS.
- [Strömungen](currents.md): Strömungsfeld und Modellreaktion.
- [Sphärische Koordinaten](coordinates.md): geografischer Ursprung.
- [Unterwassereffekte](underwater-rendering.md): Darstellung.

Legen Sie verteilbare Welten unter `models/posim_worlds/worlds` und ihre Ressourcen im selben Paket ab. Bauen und laden Sie den Arbeitsbereich erneut. Verwenden Sie beim Teilen URIs installierter Ressourcen.
