# Multibeam-Sonar

Das Mehrstrahlsonar von POSIM erzeugt Intensitäts- und Entfernungsdaten sowie Sonarbilder mit einem Punktstreumodell. Dasselbe Plugin bietet Berechnungen über CPU, WGPU und CUDA. WGPU nutzt Vulkan unter Linux oder Metal in einer nativen macOS-Umgebung und unterstützt damit GPUs verschiedener Hersteller. Hinweise zur [Einrichtung und Auswahl](sonar-tuning.md).

## Ein Beispiel starten {#run-a-sonar-example}

Bereite die [Terminals A und B](terminals.md) vor. Falls die Sonarpakete fehlen, [baue sie zunächst](sonar-tuning.md#build-the-sonar-packages). Starte die Simulation in A und prüfe die Daten in B. Der erste Befehl wählt das Backend automatisch; das gesonderte Beispiel wählt WGPU und öffnet zusätzlich RViz.

{{CODE:sonar:0}}

Das Paket `posim_multibeam_sonar_demo` enthält außerdem ein eigenständiges Beispiel:

{{CODE:sonar:1}}

Starte jeweils nur ein Beispiel. Ersetze `wgpu` durch `cpu`, `cuda` oder `auto`, um das Backend zu wechseln. Für ein Sonar am Roboter wählst du `bluerov2_heavy_multibeam_sonar` und eine passende Welt.

## Funktionsweise des Sensors

Gazebo liefert die Szenengeometrie. Das Sonarmodell schätzt Oberflächennormalen, berechnet die akustische Rückstreuung, wendet die Empfangsantwort an und ordnet die Echos Entfernungszellen zu. Akustische Daten und Bild zeigen dieselbe Beobachtung. Nutze Rohdaten für die Analyse und bewahre Verstärkungs- und Anzeigeeinstellungen auf.

Ein positiver Wert für `spec/receiveBeamWidth` aktiviert den physikalischen Modus. Er verwendet die gemessenen XYZ-Koordinaten einer Tiefenkamera, die den gesamten Sonarfächer erfasst. Geometrieabtastung, Empfangsrichtungen und Entfernungsausgaben sind unabhängig. Zellen mit Mittelpunkt im Fächer teilen dessen vollständigen Raumwinkel auf; ihre projizierten Oberflächen und die vorzeichenbehaftete Empfangsantwort bestimmen die Beiträge zu den Strahlen. Ein Hamming-Basisbandpuls mit fester Bandbreite und Spitzenwert eins verwendet ein längeres internes Berechnungsintervall als das veröffentlichte Entfernungsintervall. Der Ausgabeabstand ist `soundSpeed / (2 × bandwidth)`; die Berechnungslänge bestimmt nicht die Pulsverstärkung. Das interne Schutzintervall wird nicht ausgegeben und verändert diesen Abstand nicht.

`ray/scan/horizontal/samples` legt die Geometriespalten fest, `ray/scan/vertical/rays` die Bildzeilen und `beams` die Empfangsrichtungen. Der physikalische Modus benötigt einen symmetrischen Fächer mit weniger als 180° je Achse. Entfernungsgrenzen sind radial; Pixel außerhalb des Fächers oder der Grenzen haben ungültige XYZ-Koordinaten in der organisierten Punktwolke. `spec/receiveBeamWidth` bezeichnet die volle Halbwertsbreite der Leistung in Radiant, `spec/receiveGain` die lineare Druckverstärkung (Standard: 1). Eine fehlende Breite oder der Wert null wählt das Standardraster. Der physikalische Modus unterstützt CPU und WGPU, nicht CUDA. Es gibt keine feste Grenze von 4096 Frequenzabtastwerten; große Raster unterliegen weiterhin den Ressourcenlimits des Backends und darstellbaren Dimensionen. Streukoeffizienten werden pro Bild und Geometriezelle erzeugt. Das Modell beschreibt Basisbandechos, ohne Trägerphasenausbreitung oder Amplitudenkalibrierung der Hardware.

## Ausgaben prüfen

In Terminal B:

{{CODE:sonar:2}}

- Der Bild-Publisher verwendet die QoS-Zuverlässigkeit `reliable`. Stellen Sie diese auch in der Image-Anzeige von RViz ein.
- Fügen Sie PointCloud2 für `/sensor/multibeam_sonar/point_cloud` hinzu.
- Akustische Rohdaten verwenden `marine_acoustic_msgs/msg/ProjectedSonarImage`.
- Der BlueView-Start überträgt die Gazebo-Zeit nach ROS `/clock`. Für Nodes mit `use_sim_time` muss die Welt laufen und die Uhr fortschreiten.

## Sensorkonfiguration {#sensor-configuration}

Fügen Sie diese Systeme direkt unter `<world>` ein:

{{CODE:sonar:3}}

Der P900-Sensor gehört in einen `<link>` des Modells:

{{CODE:sonar:4}}

Das Beispiel fordert 512 horizontale Strahlen über etwa 130°, 300 vertikale Abtaststrahlen über etwa 12° und eine Reichweite von 0.1–10 m an. Es verwendet 900 kHz, 29.9 kHz Bandbreite und eine Schallgeschwindigkeit von 1500 m/s. Die eingestellten 30 Hz sind die Sollrate; die empfangene Rate hängt von Berechnung und Rendering ab. Alle Werte stehen im [Sensormodell](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf).

## Parameter in `<spec>`

| Parameter | Bedeutung | Standardwert im Code | P900-Beispiel |
| --- | --- | --- | --- |
| `verticalFOV` | Vertikale akustische Strahlbreite, Grad | 10 | 12 |
| `sonarFreq` | Akustische Frequenz, Hz | 900000 | 900000 |
| `bandwidth` | Signalbandbreite, Hz | 29500000 | 29900 |
| `soundSpeed` | Schallgeschwindigkeit, m/s | 1500 | 1500 |
| `sourceLevel` | Quellpegel, dB | 220 | 220 |
| `maxDistance` | Verarbeitungsreichweite, m; an die Strahlreichweite anpassen | 60 | 10 |
| `raySkips` | Vertikale Abtastschrittweite; positive ganze Zahl | 10 | 10 |
| `sensorGain` | Verstärkung der Echoamplitude im CPU-Standardraster; im physikalischen Modus und von WGPU/CUDA nicht verwendet | 0.02 | 0.02 |
| `blazingSonarImage` | Zufallsstartwert aus der tatsächlichen Uhrzeit; bei `false` fest 1234 | false | true |
| `pointCloudTopicName` | Suffix des Gazebo-Punktwolken-Topics | `point_cloud` | Gleich |
| `sonarImageRawTopicName` | Suffix des ROS-Topics für Rohdaten | `sonar_image_raw` | Gleich |
| `sonarImageTopicName` | Suffix des ROS-Bild-Topics | `sonar_image` | Gleich |
| `frameName` | Kennung des optischen Koordinatensystems | `forward_sonar_optical_link` | Gleich |
| `writeLog` | CSV-Dateien in einem eigenen Verzeichnis je Lauf schreiben | false | false |
| `writeFrameInterval` | Bilder zwischen zwei Schreibvorgängen; positive ganze Zahl | 10 | 5 |
| `debugFlag` | Diagnoseausgaben auf der Konsole | false | false |
| `plotScaler` | Wird eingelesen und ausgegeben, wirkt aber nicht auf die Bilderzeugung | 10 | Nicht gesetzt |

### Topics und Koordinatensysteme

Die Suffixe werden an `<topic>` angehängt, hier `/sensor/multibeam_sonar`. Gazebo veröffentlicht die Punktwolke als `gz.msgs.PointCloudPacked`; die mitgelieferte ROS-Bridge überträgt sie als `sensor_msgs/msg/PointCloud2`. Stimmen Sie `pointCloudTopicName` mit `sensor_config.py` ab und setzen Sie `frameName` auf das optische Koordinatensystem des Sonars.

### Strahlgeometrie

Die Standardwerte in `<ray>` sind 256 horizontale Strahlen, 3 vertikale Abtaststrahlen, horizontale Winkel ±π/4, vertikale Winkel ±π/8 und 0.1–5 m Reichweite. Winkel sind im Bogenmaß angegeben; `degrees="true"` schaltet auf Grad um. Das optionale `<reference_frame>` in `<gz:multibeam_sonar>` verwendet standardmäßig die Nullpose.

Behalten Sie die vollständigen Blöcke `<ray>` und `<spec>` bei: Die Standardwerte gelten für deren einzelne Felder. Strahlreichweite und `maxDistance` müssen zusammenpassen. Die [Implementierung](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc) legt das Einlesen und die Datenausgabe fest.

### Vergleichbare Messungen

Lassen Sie die Aufzeichnung bei Durchsatzmessungen ausgeschaltet, sofern sie nicht selbst zum Versuch gehört. Nutzen Sie die Anleitung [Sonar konfigurieren und abstimmen](sonar-tuning.md) und halten Sie für Vergleiche die akustischen Parameter sowie `blazingSonarImage` fest.

### Rechenbackend {#compute-backend}

| Auswahl | Verhalten |
| --- | --- |
| `auto` | Versucht CUDA, danach WGPU, dann CPU; Standardeinstellung |
| `wgpu` | Nutzt einen geeigneten Vulkan-/Metal-Adapter; ohne ihn schlägt die Sensorinitialisierung fehl |
| `cuda` | Benötigt eine NVIDIA-GPU und das gebaute, installierte CUDA-Backend; andernfalls schlägt die Initialisierung fehl |
| `cpu` | Berechnet Schallreflexionen auf der CPU; Gazebo benötigt weiterhin einen Renderer |

Der gesonderte Launch akzeptiert `compute_backend`. Andere Launches lesen `POSIM_SONAR_COMPUTE_BACKEND`. Prüfe beim Start die Zeile `Using sonar compute backend:`. [Sonar konfigurieren und abstimmen](sonar-tuning.md) erläutert Adapterwahl, Voraussetzungen und numerische Unterschiede.

## Einen Sensor hinzufügen

1. Kopieren Sie `models/posim_sensor_models/description/blueview_p900/` unter einer neuen Sensorkennung.
2. Erstellen Sie die passende `config/<Sensorkennung>/sensor_config.py`. Stimmen Sie Modell-, Link- und Frame-Namen mit den Bridge-Topics ab.
3. Passen Sie Abtastung, Winkel und Reichweite an. Laden Sie die beiden oben gezeigten Systeme in der Welt; weitere Einstellungen zeigt die [Sonarwelt als SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world).
4. Bereiten Sie Rechenbackend und Rendering vor, bauen Sie den Workspace neu und laden Sie seine Umgebung. Wählen Sie den Sensor über die [Startargumente](quickstart.md).

## Rohdaten aufzeichnen und darstellen

Mit `writeLog=true` legt jeder Sensor `SonarRawData_<run>_<suffix>/` im Arbeitsverzeichnis des Simulators an und gibt den Pfad aus. Dort stehen nummerierte CSV-Dateien und `SonarRawData_beam_angles.csv`; bestehende Aufzeichnungen bleiben erhalten. Intervalle unter 1 werden auf 1 gesetzt. Kann das Verzeichnis nicht angelegt werden, wird die Aufzeichnung mit einer Fehlermeldung deaktiviert.

Öffnen Sie im ausgegebenen Verzeichnis das erste Bild:

{{CODE:sonar:6}}

So speichern Sie die Diagramme als PNG:

{{CODE:sonar:7}}

Das [Darstellungsprogramm](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py) lässt sich auch ohne CUDA installieren. Es erzeugt eine Sonaransicht und fünf gleichmäßig ausgewählte Strahlprofile; bei weniger als fünf Strahlen zeigt es alle.

| Option | Wirkung |
| --- | --- |
| `--range-max` | Größte angezeigte Entfernung, m |
| `--beam-step` | Jeden N-ten Strahl darstellen |
| `--dynamic-range` | Bereich unterhalb des Maximums, dB; Standard: 60 |

Farben beziehen sich auf das Maximum des jeweiligen Bildes. Vergleichen Sie für unterschiedliche Läufe die CSV-Werte. Die erste Spalte enthält Entfernungen, die weiteren Spalten komplexe Echos je Strahl. Bewahren Sie die zugehörige Winkeldatei zusammen mit den Daten auf und prüfen Sie den belegten Speicherplatz.

## Szenen und Beobachtungen

{{MEDIA:sonar}}

Bilder und Videos: POSIM Notion Wiki. Unter [Zitieren und Lizenzen](citation.md) stehen die Quellen zu DAVE und zum Sonarmodell. Verwenden Sie die Befehle und Topic-Namen aus den Codeblöcken dieser Seite.
