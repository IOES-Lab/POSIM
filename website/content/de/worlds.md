# Weltbibliothek

Die Welten liegen unter `models/posim_worlds/worlds`. Wählen Sie eine Welt über den Dateinamen ohne die Endung `.world`.

Die Ozean-Demowelten enthalten `model://calm_waves`: FFT-Wellen von Wave Sim mit 3 m/s Windeingabe, einer periodischen Kachel von 256 m und einer sichtbaren Fläche von 768 × 768 m. Die Oberfläche ist von oben und unten sichtbar. Native Unterwassereffekte schattieren RGB-Sensorbilder; das flache Unterwasser-Tutorial aktiviert sie auch in der Gazebo-Weltansicht. Standardinstallation und Images enthalten Wave Sim. Auftrieb der Unterwasserfahrzeuge, marines GNSS und optische Grenze verwenden den mittleren Wasserspiegel z = 0 unabhängig von der sichtbaren Wellenhöhe. WAM-V nutzt mit den Wellen gekoppelte Wave-Sim-Hydrodynamik. Die trockene USBL-Demo und isolierte Sonartestwelten behalten ihre eigene Geometrie.

## Welt starten

Verwenden Sie das [POSIM-Terminal Ihrer Installation](terminals.md), ob unter nativem Ubuntu, WSL, Docker oder auf dem RDP-Desktop.

{{CODE:worlds:0}}

Dateiname und interner Weltname können unterschiedlich sein: `posim_ocean_waves.world` definiert `oceans_waves`. Der interne Name erscheint in Gazebo-Topics und Services. Prüfen Sie `<world name="…">`, bevor Sie einen Pfad zusammensetzen.

## Nach Versuch auswählen

| Versuch | Welt |
|---|---|
| Erstes Fahrzeug | `posim_ocean_waves` |
| Unterwasserkameraeffekte | `camera_tutorial` |
| DVL-Messungen | `dvl_world` |
| USBL-Abfrage | `usbl_tutorial` |
| Strömungseinstellungen | `ocean_current_plugin` |
| Küsten- und Geografieszene | `posim_Santorini` |
| Sonar | `posim_multibeam_sonar` und Ozeanwelten mit sonar im Namen |
| Manipulation | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

Sonarwelten benötigen die [Sonarumgebung](sonar-tuning.md). Fuel-Verweise können beim ersten Start Modelle herunterladen.

## SDF-Dateien

Öffnen Sie eine Datei, um Systeme, Modelle, Ursprung, Beleuchtung und Kameras zu prüfen.

| Welt-SDF |
|---|
{{WORLD_CATALOG}}

## Eigene Umgebung zusammenstellen

Kopieren Sie eine passende SDF nach `models/posim_worlds/worlds` und vergeben Sie neue Datei- und Weltnamen. Verwenden Sie URIs, die auf installierte Ressourcen auflösen. Bauen und laden Sie den Arbeitsbereich erneut, dann starten Sie den neuen Dateinamen. Weitere Hinweise bieten [Höhenkartengelände](heightmaps.md) und [Objekte](objects.md).

## Fuel-Downloads abwarten

`posim_integrated` und `posim_ocean_models` laden entfernte Modelle. Warten Sie den ersten Download ab und vermeiden Sie wiederholte Starts währenddessen. `verbose:=true` zeigt Meldungen zum Laden der Ressourcen.

Fragen Sie die geladene Szene in einem weiteren Terminal derselben Umgebung ab:

{{CODE:worlds:1}}

Prüfen Sie die erwarteten Modelle in der Antwort sowie die Simulationsuhr und Sensordaten. Stockt ein Download, prüfen Sie die Lademeldung und die Netzwerkverbindung des Gazebo-Rechners oder -Containers.

## Startargumente nachschlagen

{{CODE:worlds:2}}

| Argument | Bedeutung | Vorgabe beim Sensorstart |
|---|---|---|
| `namespace` | Verzeichnis des installierten Modells | Leer; ausdrücklich angeben |
| `world_name` | Weltdatei | `empty.sdf` |
| `paused` | Pausiert starten | `true` |
| `gui` / `headless` | Gazebo-Oberfläche anzeigen / ausblenden | `true` / `false` |
| `use_sim_time` | Simulationsuhr in ROS verwenden | `true` |
| `x`, `y`, `z` | Anfangsposition, m | Jeweils `0.0` |
| `roll`, `pitch`, `yaw` | Anfangsorientierung, rad | Jeweils `0.0` |
| `debug` / `verbosity_level` | Ausführliche Protokollierung / Stufe | `false` / `1` |
| `use_ned_frame` | `world_ned`-Transformation veröffentlichen (ENU → NED) | `false` |

Mit `paused:=false` erzeugen die Sensoren Daten. Die Argumente für Roboter- und Weltstarts zeigt jeweils deren `--show-args`.
