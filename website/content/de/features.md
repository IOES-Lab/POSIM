# Funktionen und Beispiele

POSIM bietet Gazebo-Systeme, Robotermodelle und ROS-Schnittstellen. Welt-SDF und Startkonfiguration bestimmen, welche Systeme eine Simulation verwendet.

## Umgebung und Darstellung

| Funktion | Implementierung | Anleitung |
|---|---|---|
| Unterwassertrübung, RGB-Absorption und Abschwächung des Sonnenlichts | `gazebo/posim_gz_rendering` | [Unterwassereffekte](underwater-rendering.md) |
| Lichtkaustiken am Boden und Schwebteilchen | `gazebo/posim_gz_rendering` | [Effekte einschalten](underwater-rendering.md) |
| Bodentextur, Messraster und Tiefenlinien | `gazebo/posim_gz_rendering`; Karten des Geländes | [Materialien und Abstände](underwater-rendering.md) |
| Spektrale Wellen und WAM-V-Hydrodynamik | Externes Wave Sim; Integration unter `extras/surface` | [Oberflächenfahrzeuge](surface.md) |
| Gleichmäßige, tiefenabhängige und Gezeitenströmungen | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [Strömungen](currents.md) |
| Geografischer Ursprung und Koordinatenumrechnung | `posim_ros_gz_plugins` | [Sphärische Koordinaten](coordinates.md) |
| Meeresboden, Objekte und Aufgabenszenen | `models/posim_worlds`, `posim_object_models` | [Welten](worlds.md), [Objekte](objects.md), [Gelände](heightmaps.md) |

Die Unterwasseroptik verändert PBR-Oberflächen und erhält numerische Tiefenwerte. Die Meeresoberfläche verwendet ein eigenes Material. Meeresschnee ist eine prozedurale visuelle Näherung und zunächst ausgeschaltet. Annahmen zum Meeresspiegel und Kameraverhalten beschreibt die [Rendering-Anleitung](underwater-rendering.md).

## Sensoren, Roboter und Steuerung

| Funktion | Implementierung | Anleitung |
|---|---|---|
| Marine-GNSS, Empfangsausfall unter Wasser, Wiedererfassung und Latenz | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| RGBD-basierte Unterwasserabschwächung | `posim_gz_sensor_plugins/UnderwaterCamera` | [Kamera](camera.md) |
| DVL-Messungen und ROS-Umsetzung | Gazebo-DVL; `posim_ros_gz_plugins` | [DVL](dvl.md) |
| Hydrostatischer Druck und berechnete Tiefe | `posim_gz_sensor_plugins` | [Druck](pressure.md) |
| Akustische USBL-Abfrage und Positionierung | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| CUDA-Multibeam-Sonar | `gazebo/posim_gz_multibeam_sonar` | [Sonar](sonar.md), [Einrichtung und Leistung](sonar-tuning.md) |
| REXROV, BlueROV2-Varianten und Slocum-Gleiter | `models/posim_robot_models` | [ROVs](rovs.md), [Gleiter](gliders.md), [eigene Roboter](custom-robots.md) |
| WAM-V mit ArduRover und ROS-Steuerung | `extras/surface`, Startkonfigurationen, `posim_interfaces` | [Oberflächenfahrzeug](surface.md), [ROS 2](ros.md) |

Verfügbare Sensoren hängen von installierten Plugins und der Modellkonfiguration ab. GNSS-Profile beschreiben die Montage; ergänzen Sie bei der Integration Link und Empfänger. CUDA-Sonar benötigt eine unterstützte GPU und Werkzeugkette. Prüfen Sie laufende Messungen mit dem Schnellstart der jeweiligen Sensorreferenz.

## Quellverzeichnisse

Die Pfade beziehen sich auf das POSIM-Repository.

| Verzeichnis | Inhalt |
|---|---|
| `models/posim_worlds` | Weltdateien und Umgebungsressourcen |
| `models/posim_robot_models` | Robotermechanik, Sensoren und ROS-Konfiguration je Roboter |
| `models/posim_sensor_models` | Eigenständige Sensorbeschreibungen und Beispiele |
| `gazebo/` | Plugins für Meeresumgebung und Sensoren |
| `posim_interfaces` | POSIM-Nachrichten und Services |
| `examples/posim_demos` | Startdateien zum Verbinden von Welten, Modellen und ROS |

Die [erste Simulation](quickstart.md) führt diese Bestandteile zusammen aus. Änderungen erläutern [Roboter hinzufügen](custom-robots.md) und [Welt erstellen](custom-worlds.md).

## Alternative Ogre-Bibliothek für macOS

Das [Ogre-Hilfsprogramm](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) baut einen isolierten Ogre-Kern für macOS arm64 mit Ogre-Next 2.3.3. Beachten Sie das unterstützte SDK-Profil und die Prüfung der geladenen Bibliotheken.

Die Lizenzbedingungen stehen bei den jeweiligen [Komponenten](citation.md).
