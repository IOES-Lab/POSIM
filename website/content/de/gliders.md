# Slocum-Gleiter

Das Modell `glider_slocum` enthält Rumpf, Thruster, Hydrodynamik sowie IMU-, Navigations- und Druckschnittstellen. Verwenden Sie es für Gleiterversuche oder als Ausgangspunkt für ein eigenes Fahrzeug.

## In einer Ozeanwelt starten

Im [POSIM-Terminal für Ihre Installation](terminals.md):

{{CODE:gliders:0}}

Für eine leere Welt verwenden Sie `world_name:=empty.sdf z:=0.2`.

## Messwerte untersuchen

{{CODE:gliders:1}}

Die Gazebo-Topics für IMU und NavSat liegen unter `/model/glider_slocum/`. Die Bridge-Konfiguration zeigt die zugehörigen ROS-Topics und Typen. Das Modell lädt außerdem das [Druck-Plugin](pressure.md) und den Gazebo-Odometrie-Publisher.

Starten Sie Fahrzeug und Umgebung, bevor Sie Ihren eigenen Steuerungs-Node für Tauch- oder Gleitmissionen anschließen.

## Modell anpassen

Lesen Sie die [Slocum-SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) gemeinsam mit der Bridge-Konfiguration. Passen Sie Masse, Trägheit, Kollisionsgeometrie und hydrodynamische Koeffizienten konsistent an. Prüfen Sie die Koordinatenkonventionen vor dem Anschluss eines Schätzers oder Reglers.

Behalten Sie das Ausgangsmodell für Vergleiche. [Roboter hinzufügen](custom-robots.md) erläutert eine eigene Beschreibung; [ROS](ros.md) zeigt Beobachtung und Ansteuerung.

## Gleiter in Bewegung

{{MEDIA:gliders}}

Bilder und Videos: POSIM Notion Wiki. Die DAVE-Quellenangaben stehen unter [Zitation und Lizenzen](citation.md). Verwenden Sie die Befehle und Topic-Namen dieser Seite.
