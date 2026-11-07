# Doppler-Geschwindigkeitsmessung (DVL)

Ein DVL schätzt die Geschwindigkeit relativ zum Meeresboden oder zum umgebenden Wasser. POSIM verwendet Gazebos DVL-System und eine ROS-Bridge für `posim_interfaces/msg/DVL`.

## Sensor starten

Im [für Ihre Installation vorbereiteten Terminal A](terminals.md):

{{CODE:dvl:0}}

Lesen Sie im vorbereiteten Terminal B zunächst Gazebo-Daten, beenden Sie die Anzeige mit **Strg+C** und lesen Sie dann ROS:

{{CODE:dvl:1}}

Verwenden Sie bei einem eigenen Sensor das Topic aus dessen Modell.

## SDF-Integration

Setzen Sie diese Systeme direkt in `<world>`:

{{CODE:dvl:2}}

Die POSIM-Bridge gehört in das `<model>` des Sensors. Ihr Topic muss zum `<topic>` des DVL in `<link>` passen:

{{CODE:dvl:3}}

Die [vollständige DVL500-300-SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf) definiert `type="custom" gz:type="dvl"`, vier Strahlen und die Tracking-Modi. Die [Welt](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world) enthält die Bodengeometrie.

## Parameter und Einheiten

`DVLBridge` akzeptiert nur `topic`, standardmäßig `/dvl/velocity`. Dort empfängt sie Gazebo und veröffentlicht ROS. Die übrigen Felder gehören zum Gazebo-Sensor; die Tabelle zeigt DVL500-300-Beispielwerte.

| Sensor-SDF-Feld | Bedeutung | Beispiel |
|---|---|---|
| `topic`, `update_rate` | Pfad und gewünschte Frequenz, Hz | `/dvl/velocity`, 8 |
| `gz:dvl/type` | Wandleranordnung | `phased_array` |
| `arrangement`: `aperture`, `rotation`, `tilt` | Strahlwinkel; `degrees="true"` verwendet Grad | Öffnung 3°, Drehungen −135/135/45/−45°, Neigung 25° |
| `minimum_range`, `maximum_range` | Akustische Reichweite, m | 0.3, 200 |
| `reference_frame` | Position in m, Rotation in rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | Auswahl der Messreferenz | Beide `best` |
| Tracking: `noise/stddev` | Standardabweichung des Geschwindigkeitsrauschens, m/s | Boden 0.005; Wasser 0.015 |
| Wassermasse: `water_velocity` | x/y/z-Komponenten, m/s | 0/0/0 |
| Wassermasse: `boundaries/near`, `boundaries/far`, `bins` | Abtastgrenzen in m und Zellenzahl | 10, 100, 10 |
| Tracking: `visualize` | Abtastgeometrie anzeigen | true |

Die [Bridge-Implementierung](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc) beschreibt die Umrechnung. Der Tiefensensor nutzt Gazebos Rendering; schließen Sie die [Umgebungseinrichtung](requirements.md) ab.

## Verfügbare Modelle

| Beschreibung | Familie |
|---|---|
| `nortek_dvl500_300`, `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300`, `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000`, `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL mit [CUDA-Sonar](sonar-tuning.md) |

Prüfen Sie Abtastung und Geometrie in der jeweiligen SDF.

## Bezugssystem und Kovarianz

Das Beispiel rechnet ENU mittels `<reference_frame>` in sein Bezugssystem vorwärts/steuerbord/abwärts um. Die Bridge erhält den Gazebo-Framenamen und ersetzt `::` durch `/`. Verwenden Sie dasselbe Bezugssystem im Schätzer und unterscheiden Sie Messungen relativ zum Boden und zur Wassermasse.

Gesamtgeschwindigkeit und jeder Strahl enthalten eine ROS-Twist-Kovarianz von 6×6. Oben links steht die gemessene 3×3-Kovarianz der linearen Geschwindigkeit in m²/s². Die übrigen Einträge sind null, weil keine Winkelgeschwindigkeit geschätzt wird; sie bedeuten keine gemessene Unsicherheit von null.

## Beobachtungen prüfen

Lesen Sie `ros2 interface show posim_interfaces/msg/DVL`. Prüfen Sie Geschwindigkeit, Bezugssystem und Status im Stillstand und während einer kontrollierten Bewegung. Boden, Reichweite und Strahlschnittpunkte beeinflussen die Messung. Aktivieren Sie `visualize` im Modell, um die Strahlen anzuzeigen.

## DVL-Strahlen und Ausgabe

{{MEDIA:dvl}}

Bilder: POSIM Notion Wiki; DAVE-Zuordnung unter [Zitation und Lizenzen](citation.md). Verwenden Sie die Befehle und Topics dieser Seite.
