# Sensordaten anzeigen und aufzeichnen

Lassen Sie den Simulator in Terminal A laufen. Verwenden Sie B, um Messwerte zu prüfen, eine Visualisierung zu öffnen oder Daten aufzuzeichnen.

## Kamerabild empfangen

Bereiten Sie [beide Terminals in derselben Umgebung](terminals.md) vor. Beenden Sie die vorherige Simulation in A mit Strg+C und starten Sie diese Szene. Gazebo öffnet sich auf dem bei der Installation eingerichteten Desktop.

{{CODE:visualization:0}}

In B:

{{CODE:visualization:1}}

Prüfen Sie, ob Breite und Höhe größer als null sind, sowie Kodierung und Zeitstempel. `--no-arr` blendet das Pixel-Array aus. Um das Bild zu sehen, fügen Sie in RViz ein **Image**-Display hinzu und wählen Sie dieses Topic. Die [Unterwasserkamera](camera.md) beschreibt die Dämpfungseinstellungen.

## Passende Anzeige wählen

Nutzen Sie in RViz **Image** für Kameras, **PointCloud2** für Punktwolken und **TF-/Pose-Anzeigen** für Koordinatenrahmen und Zustand.

Jeder separate ROS-Client benötigt installierte und geladene `posim_interfaces`, wenn er POSIM-Nachrichten verwendet. Topics und Nachrichtentypen stehen in den Referenzen zu [Kamera](camera.md), [Druck](pressure.md), [DVL](dvl.md), [USBL](usbl.md) und [Sonar](sonar.md). [Strömungen](currents.md) und [sphärische Koordinaten](coordinates.md) bieten außerdem ROS-Services für geänderte Versuchseingaben.

## Aufzeichnen und wiedergeben

Dieser eigenständige Kamerastart erstellt keinen ROS-Publisher für `/clock`. Um auch die Simulationszeit aufzuzeichnen, bereiten Sie Terminal C in derselben ROS-Umgebung wie A und B vor und lassen Sie die folgende unidirektionale Bridge laufen. Starten Sie keine zweite Bridge, wenn `/clock` bereits einen Publisher hat.

{{CODE:visualization:2}}

Prüfen Sie vor der Aufzeichnung in Terminal B, ob eine Clock-Nachricht ankommt:

{{CODE:visualization:3}}

{{CODE:visualization:4}}

Strg+C beendet die Aufzeichnung. Verwenden Sie bei jedem Durchlauf ein neues Ausgabeverzeichnis und prüfen Sie die Anzahl der Nachrichten:

{{CODE:visualization:5}}

Prüfen Sie nach der Aufzeichnung, ob das Bag sowohl das Bild-Topic als auch `/clock` enthält. Beenden Sie die Bridge in Terminal C mit Ctrl+C, bevor Sie das Bag wiedergeben.

Beenden Sie den Simulator, bevor Sie die Daten auf denselben Topics wiedergeben:

{{CODE:visualization:6}}
