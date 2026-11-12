# Meeresströmungen

POSIM erzeugt Strömungen für die Welt, berechnet die Strömung am jeweiligen Roboter und stellt ROS-Dienste zur Einstellung bereit. Eine gleichförmige Strömung und ein tiefenabhängiges Profil verwenden unterschiedliche Topics und Einstellungen.

## Das Beispiel starten

Bereiten Sie die [Terminals A und B](terminals.md) in Ihrer POSIM-Umgebung vor: natives Ubuntu, WSL, Docker oder der macOS-Remotedesktop. Starten Sie in A:

{{CODE:currents:0}}

Prüfen Sie in B zuerst Gazebo und danach ROS. Beenden Sie die erste Ausgabe mit Ctrl+C, bevor Sie den zweiten Befehl ausführen:

{{CODE:currents:1}}

{{CODE:currents:2}}

## Die Strömung der Welt konfigurieren

Beide Plugins gehören direkt unter `<world>`, das Weltströmungs-Plugin vor das ROS-Plugin. Die vollständige Konfiguration zeigt die [Beispielwelt](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world).

{{CODE:currents:3}}

Verwenden Sie in beiden Plugins denselben, nicht leeren `namespace`. Die Blöcke `constant_current` und `transient_current` sind erforderlich. Fehlt eine Pflichtangabe, wird die Veröffentlichung mit einer Fehlermeldung deaktiviert. Separate Gauß-Markow-Modelle beschreiben Geschwindigkeit, Horizontalwinkel und Vertikalwinkel der gleichförmigen Strömung.

| Parameter | Bedeutung und Standardwert |
| --- | --- |
| `namespace` beider Plugins | Gemeinsamer Namensraum; im Beispiel `hydrodynamics` |
| `constant_current/use_constant_current` | Gleichförmige Strömung veröffentlichen; `false` |
| `constant_current/topic` | Gazebo-Topic der gleichförmigen Strömung; `ocean_current` |
| `transient_current/topic_stratified` | Gazebo-Schichtdaten im Namensraum; `stratified_current_velocity` |
| `transient_current/databasefileName` | CSV im installierten `posim_worlds/worlds`; `transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | Optionaler CSV-Pfad; hat Vorrang vor dem Dateinamen |
| `mean` jedes stochastischen Modells | Anfangsmittelwert der Geschwindigkeit (m/s) oder des Winkels (rad); 0 |
| `min`, `max` | Grenzen; −1 und 1. `min < max` einhalten und den Mittelwert dazwischen wählen |
| `noiseAmp` | Amplitude des Zufallsinkrements; 0 |
| `mu` | Abklingkoeffizient des Gauß-Markow-Modells; 0 |
| `tidal_oscillation` | Optionaler Block für Gezeitendaten |

## Ein Tiefenprofil vorbereiten

Die [Beispiel-CSV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv) enthält in dieser Reihenfolge die Geschwindigkeit entlang der Welt-X-Achse (m/s), der Welt-Y-Achse (m/s) und die Tiefe (m). Die bisherigen Schlüssel `velocity_north`, `velocity_east` und `velocity_down` entsprechen direkt den Weltachsen X, Y und Z.

- Behalten Sie die ersten drei Beschreibungs- und Kopfzeilen bei.
- Geben Sie mindestens zwei Schichten mit genau drei endlichen Zahlen je Zeile an.
- Die Tiefen müssen nichtnegativ und streng aufsteigend sein.
- Eine fehlende oder fehlerhafte Datei deaktiviert die Strömungsausgabe.

Das ROS-Plugin der Welt überträgt die vollständige Datenbank. Das Modell-Plugin interpoliert die Strömung in der Tiefe des Fahrzeugs. Für die hydrodynamische Reaktion zählt die Strömung relativ zur Fahrzeuggeschwindigkeit.

## Das Profil am Roboter anwenden

Das REXROV-Beispiel nutzt zunächst die gleichförmige Strömung `/ocean_current`. Aktivieren Sie für das Tiefenprofil den auskommentierten Block im [REXROV-Modell](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf) genau einmal, direkt unter `<model>`:

{{CODE:currents:4}}

Ergänzen Sie außerdem `<namespace>rexrov</namespace>` im bestehenden Plugin `gz-sim-hydrodynamics-system`. Behalten Sie dessen Link, Dichte und Widerstandskoeffizienten bei. Bauen Sie den Workspace neu und laden Sie seine Umgebung.

| Modellparameter | Bedeutung und Standardwert |
| --- | --- |
| `namespace` | Erforderlich; muss zum Hydrodynamik-Namensraum passen, hier `rexrov` |
| `flow_velocity_topic` | Suffix unter `/model/<namespace>/`; `ocean_current` |
| `transient_current/topic_stratified` | ROS-Eingang der Datenbank; `hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | Optionale stochastische Einstellungen je Komponente; ausgelassene Komponenten verwenden kein Rauschen |
| `mean` jeder Komponente | Anfangsgeschwindigkeit, m/s; 0. Nach Datenempfang durch das interpolierte Profil ersetzt |
| `mu` | Abklingkoeffizient von 0 bis 1, angewandt mit der verstrichenen Simulationszeit in Sekunden; 0 |
| `noiseAmp` | Zulässige Abweichung von der interpolierten Geschwindigkeit, m/s; 0 |
| `noiseFreq` | Amplitude des Zufallsinkrements je Aktualisierung; 0. Keine Frequenz in Hz |
| `tide_oscillation` | Gelieferte Gezeitendaten am Modell anwenden; `false` |

Das Modell veröffentlicht `/model/rexrov/ocean_current` in Gazebo als `gz.msgs.Vector3d` und in ROS als `geometry_msgs/msg/TwistStamped`. Prüfen Sie die Ausgabe und die Fahrzeugreaktion.

Die Tiefe folgt aus der Weltposition des Modells; oberhalb der Oberfläche beträgt sie null. Zwischen den Schichten wird interpoliert, außerhalb des Profils die nächstgelegene Schicht verwendet. Bei Pause oder wiederholtem Zeitstempel wird die Strömung nicht weiterentwickelt. Eine ungültige Datenbanknachricht wird vollständig verworfen; das letzte gültige Profil bleibt bestehen.

## Systeme und Topics

| System | Aufgabe |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | Weltströmung und Daten der Tiefenschichten erzeugen |
| Strömungs-Plugin des Modells | Strömung an der Roboterposition interpolieren |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | ROS-Daten veröffentlichen und Einstelldienste anbieten |

| Beispiel-Topic | Transport und Typ |
| --- | --- |
| `/ocean_current` | Gazebo, `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo, Schichtdaten |
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS, `posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS, `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS, `posim_interfaces/msg/StratifiedCurrentDatabase` |

Prüfen Sie die Namen in der SDF-Datei und verbinden Sie die Hydrodynamik mit der gewünschten Strömungsquelle.

## Strömungen über ROS-Dienste einstellen

Listen Sie in B die Dienste auf und fordern Sie eine horizontale Strömung mit 0.3 m/s an:

{{CODE:currents:5}}

Geschwindigkeiten sind in m/s, Winkel in Radiant angegeben. Prüfen Sie `success` und die veröffentlichten Daten. So sehen Sie die Schnittstelle für eine Tiefenschicht:

{{CODE:currents:6}}

Lesen Sie das Modell, ändern Sie seine Grenzen und stellen Sie die erste Schicht ein:

{{CODE:currents:7}}

| Dienst unter `/hydrodynamics` | Schnittstelle | Felder |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | Leere Anfrage |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

### Zulässige Werte

- Eine Anfrage wird vollständig angewandt oder liefert `success: false`, ohne die Strömung zu ändern.
- Alle Werte müssen endlich sein und innerhalb der konfigurierten Grenzen liegen. Geschwindigkeitsgrenzen sind nichtnegativ; Winkelgrenzen liegen horizontal innerhalb ±π und vertikal innerhalb ±π/2.
- Modelleinstellungen gelten für das globale Modell und alle Tiefenschichten. Deren Mittelwerte bleiben erhalten und müssen ebenfalls in die neuen Grenzen passen.
- `layer` zählt ab 0. Das ROS-Feld `noise` entspricht `noiseAmp` in SDF.

## Gezeitendaten einbinden

Konfigurieren Sie `<tidal_oscillation>` mit einer CSV-Datei oder harmonischen Bestandteilen wie M2/S2/N2, Flut- und Ebbstromrichtung sowie einer GMT-Startzeit. Die harmonischen Beispiele verwenden Meter für die Amplitude, Grad für die Phase und Grad/Stunde für die Winkelgeschwindigkeit. Die Winkel der Strömungsdienste bleiben dagegen in Radiant.

Im Gezeitenmodus wird das berechnete harmonische Signal oder die durch 100 geteilte CSV-Geschwindigkeit mit der ersten Spalte des Tiefenprofils multipliziert. Die Höhenamplituden des Beispiels dienen als Zahlenfaktoren; das Modell rechnet Gezeitenhöhe nicht in Strömungsgeschwindigkeit um. Kalibrieren Sie Signal und Profil gemeinsam für die gewünschte Strömungsgeschwindigkeit.

Die Gezeiten-CSV hat eine Kopfzeile, gefolgt von `YYYY-MM-DD HH:MM,event,speed`:

- Ereignisse: `flood`, `ebb` oder `slack`. Die Geschwindigkeit muss endlich und in cm/s angegeben sein; bei übersprungenen `slack`-Zeilen ist `-` zulässig.
- Zeitangaben müssen gültig und streng aufsteigend sein.
- Eingelesen werden abwechselnd positive und negative Spitzenwerte, mindestens zwei. Außerhalb des Zeitbereichs bleibt der erste beziehungsweise letzte Spitzenwert bestehen.
- Fehlende Dateien, unvollständige Einstellungen oder fehlerhafte Daten deaktivieren das Weltströmungs-Plugin.

Die Daten sind Versuchseingaben. Bewahren Sie Einheiten, Interpolationsbereich, Weltursprung und Zeitbezug. Vergleichen Sie Strömung und Fahrzeugbewegung jeweils mit derselben Ausgangspose.

## Strömung und Fahrzeugreaktion

{{MEDIA:currents}}

Bilder und Videos: POSIM Notion Wiki. Die DAVE-Quellen stehen unter [Zitieren und Lizenzen](citation.md). Verwenden Sie die Befehle und Topic-Namen dieser Seite.
