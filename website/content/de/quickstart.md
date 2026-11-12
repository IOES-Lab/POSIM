# Erste Simulation

Erkunden Sie mit BlueROV2 eine Inspektionsszene im flachen Wasser und anschließend mit WAM-V einen Bojenparcours. Jedes Beispiel öffnet Gazebo und ein vorkonfiguriertes RViz-Fenster. Führen Sie jeweils nur eine Simulation aus.

## Vorbereitungen

Schließen Sie eine [Installation](requirements.md) ab und laden Sie denselben ROS/POSIM-Arbeitsbereich in den [Terminals A und B](terminals.md). A führt den Simulator aus; B liest Daten und startet die Videoübertragung. Docker verwendet den bei der Installation eingerichteten Linux-, WSLg- oder RDP-Desktop. Beide Beispiele gehören zur Standardinstallation; ein separates Image für Oberflächenfahrzeuge ist nicht erforderlich.

## Unterwasserinspektion mit BlueROV2

In **Terminal A**:

{{CODE:quickstart:0}}

BlueROV2 startet etwa 3,1 m unter dem mittleren Wasserspiegel über einem 4 m tiefen Meeresboden. Vor dem Fahrzeug liegen Kasten, Rohr, Säule und Kugel. Kleine Wellen, Unterwassertrübung, Absorption, Kaustiken und Meeresschnee sind aktiviert. Angepasster Ballast hält das noch nicht scharfgeschaltete Fahrzeug nahe den Objekten. Gazebo zeigt neben der Szene das Live-Kamerabild.

Für Mehrstrahlsonar hängen Sie `sonar:=true` an diesen Befehl an. Damit wird BlueROV2 Heavy mit Sonar gewählt: Gazebo erhält ein Sonarbild, RViz ein Sonarbild und eine Punktwolke. Sonar benötigt viele Rechenressourcen und **kann Bildrate und Reaktionsgeschwindigkeit der Steuerung verringern**. `auto` versucht abhängig von Host und Sensorkonfiguration zuerst CUDA, dann WGPU und CPU. Die üblichen Rendering-Vorgaben bleiben bestehen. Anforderungen stehen unter [Sonar konfigurieren](sonar-tuning.md).

Die folgenden Aufnahmen zeigen die Sonarvariante. RViz lädt Fahrzeugmodell, Kamera und Sensoranzeigen automatisch. Das 1-m-Raster dient als Entfernungsmaß im Fahrzeugkoordinatensystem; es ist kein gemessener Meeresboden. **Move Camera** ist ausgewählt: Ziehen dreht die Ansicht, die mittlere Maustaste verschiebt sie und das Rad zoomt. Die dargestellten Propeller behalten ihren Ruhewinkel; die Simulationsdynamik bleibt unverändert.

{{MEDIA:quickstart:0}}

### Live-Daten prüfen

In **Terminal B**:

{{CODE:quickstart:1}}

Ersetzen Sie bei der Sonarvariante `bluerov2` im Topic durch `bluerov2_heavy_multibeam_sonar`. Auch bei nahezu ruhendem Fahrzeug müssen weiter Positionsdaten eintreffen. Beenden Sie die Anzeige mit **Strg+C** und prüfen Sie, ob die Simulationszeit fortschreitet:

{{CODE:quickstart:2}}

Beenden Sie diese Anzeige mit Strg+C. Bei fehlenden Nachrichten oder angehaltener Zeit hilft die [Fehlersuche](troubleshooting.md).

### Video und Steuerung mit QGC

Starten Sie unter Windows/WSL die Linux-Version von QGC in derselben Ubuntu-Distribution wie POSIM, bei Docker im selben POSIM-Container. Verwenden Sie für diese Tutorials nicht die unter Windows installierte Anwendung.

Folgen Sie den Schritten 1–3 der [QGC-Einrichtung](qgroundcontrol.md) und aktivieren Sie die virtuellen Joysticks. Wählen Sie **UDP h.264 Video Stream**, Adresse **127.0.0.1:5600**. Aktivieren Sie **Low Latency Mode** und deaktivieren Sie **Disable Video Stream When Disarmed**. Lassen Sie das Tutorial weiterlaufen und starten Sie die Videoübertragung in Terminal B:

{{CODE:quickstart:3}}

Ein Klick auf das kleine Videofeld in QGC vergrößert das Bild. **No GPS Lock** ist normal, solange die marine GNSS-Antenne unter Wasser liegt. Telemetrie, Video und manuelle Steuerung bleiben verfügbar. Warten Sie auf **Ready**, wählen Sie **Manual**, schalten Sie mit **Arm** scharf und bewegen Sie den Joystick kurz. Lassen Sie ihn zur Neutralstellung zurückkehren und wählen Sie **Disarm**. Beachten Sie die [QGC-Bedienschritte](qgroundcontrol.md). Starten Sie keine zusätzliche Tastatur- oder Joysticksteuerung.

{{MEDIA:quickstart:1}}

## Sensoren an der Oberfläche mit WAM-V

Beenden Sie zuerst die Unterwassersimulation. In **Terminal A**:

{{CODE:quickstart:4}}

WAM-V startet auf kleinen Wellen vor vier verankerten Bojen. RViz zeigt die Kamera sowie Fahrzeug und Lidar-Punktwolke in einer gemeinsamen Ansicht. Das 1-m-Raster und Move Camera sind voreingestellt. Der Lidar scannt horizontal: Die Punkte zeigen Bojenkonturen auf Sensorhöhe, keine vollständige 3D-Rekonstruktion.

Klicken Sie im Gazebo-Panel **Lidar** auf Aktualisieren und wählen Sie `/tutorial/lidar`. Deaktivieren Sie **Show Non Hitting Rays**, um nur Strahlen mit Objekttreffern zu zeigen. Das Kamerapanel bleibt neben der Szene sichtbar.

{{MEDIA:quickstart:2}}

Die Kamera verwendet dasselbe Topic `/tutorial/camera`, denselben Startbefehl und dieselben QGC-Videoeinstellungen wie das Unterwasserbeispiel. WAM-V nutzt den Bootsmodus von ArduRover mit simulierter Position und Geschwindigkeit als externer Navigation; siehe [Oberflächenfahrzeuge und Wellen](surface.md). Dieser Eingang ist vom marinen GNSS-Empfänger des BlueROV2 getrennt.

Im QGC-Modus Manual steuern Sie den WAM-V-Schub mit der vertikalen Bewegung des linken Sticks. Wählen Sie Arm, bewegen Sie ihn kurz, lassen Sie ihn zur Neutralstellung zurückkehren und wählen Sie Disarm.

{{MEDIA:quickstart:3}}

Lesen Sie in **Terminal B** einen Lidar-Scan:

{{CODE:quickstart:5}}

Die Punktwolke liegt unter `/tutorial/lidar/points`, die Fahrzeugodometrie unter `/model/wamv/odometry`.

## Sitzung beenden

Wählen Sie Disarm, schließen Sie RViz und QGC und drücken Sie **Strg+C in Terminal A**. Warten Sie, bis die gestarteten Prozesse beendet sind. Verwerfen Sie gegebenenfalls Änderungen an der RViz-Ansicht, um die gelieferte Voreinstellung zu behalten. Bei Problemen beim Beenden hilft die [Viewer-Fehlersuche](troubleshooting.md). Das Schließen einer Topic-Anzeige stoppt die Simulation nicht. Beenden Sie die aktuelle Sitzung, bevor Sie ein weiteres Beispiel starten.

Um zur Ausgangsposition zurückzukehren, beenden Sie die Sitzung und führen Sie den Startbefehl erneut aus. Ein vollständiger Welt-Reset in Gazebo entfernt nach dem Start hinzugefügte Modelle, einschließlich des Tutorial-Fahrzeugs, und startet dessen Autopiloten nicht neu.

## Weiterlernen

- [Sensordaten anzeigen und aufzeichnen](visualization.md): Bilder ansehen und Messwerte speichern.
- [Mit Python steuern](python-control.md): das separate REXROV-Beispiel starten und einen Schubgeber testen.
- [ROVs und BlueROV2](rovs.md): ArduSub, Tastatur oder lokalen Joystick nutzen.
- [Weltenbibliothek](worlds.md): Umgebung wählen und Startargumente prüfen.
