# Oberflächenfahrzeuge und Wellen

Simulieren Sie ein WAM-V mit Wave-Sim-Wellen und ArduRover im Bootsmodus. POSIM-Installer und Docker-Rezepte bauen die externen Wellenbibliotheken.

## WAM-V vorbereiten

Dieses optionale Image verwendet den ARM64-Desktopaufbau. Bereiten Sie auf einem Apple-Silicon-Mac [Docker und RDP](install-macos.md) vor. Führen Sie im macOS-Terminal aus einer POSIM-Repository-Kopie Folgendes aus:

{{CODE:surface:0}}

Verbinden Sie RDP mit **127.0.0.1:3390** und dem Konto `docker`. Öffnen Sie im Linux-Desktop ein Terminal:

{{CODE:surface:1}}

Das Image enthält getrennte ArduSub- und ArduRover-Programme. Das Beispiel wählt ArduRover mit `FRAME_CLASS=2`. Starten Sie **eine Instanz je Umgebung**: Fahrzeugname, erzeugte Dateien und Controller-Ports sind festgelegt. Mehrere Fahrzeuge benötigen getrennte Dateien, Namen, Controller-Kennungen und Ports oder isolierte Umgebungen.

Der Controller wartet auf das Fahrzeug und dessen IMU-Daten. Scheitert das Einsetzen oder dauert es länger als 180 s, oder folgt innerhalb weiterer 180 s keine IMU-Messung, endet der Start mit Fehlerstatus und nennt die betroffene Phase. Prüfen Sie vor einem erneuten Versuch die Gazebo- und Sensormeldungen.

## Modell, Steuerung und Navigation

`extras/surface/wamv.py` bereitet das externe Modell vor. Die hinteren Propeller verwenden Ausgänge 1 und 3; das Fahrzeug startet an der Wasseroberfläche.

Der Adapter für externe Navigation sendet **simulierte Gazebo-Pose und -Geschwindigkeit** an MAVROS. Prüfen Sie Controller-Zustand, gültige Navigation und Schubrichtung, bevor Sie eine Mission senden.

Für Empfängerrauschen, Wasserkontakt und Wiedererfassung integrieren Sie [Marine-GNSS](gnss.md), Antenne und einen passenden Navigationsadapter. Der Adapter für externe Pose ist ein eigener Navigationseingang.

## Wellen einstellen

Die Steuerung nutzt `gz.msgs.Param` auf `/world/<world_name>/waves`. Der Name stammt aus `<world name="…">`. Listen Sie laufende Topics auf:

{{CODE:surface:2}}

Der [WAM-V-Start](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py) verwendet standardmäßig `posim_surface`, also `/world/posim_surface/waves`. Im vorbereiteten Container lässt sich der Name ändern:

{{CODE:surface:3}}

Das Topic lautet dann `/world/harbour/waves`; die ROS-IMU bleibt unter `/model/wamv/imu`. `world_name` benennt die erzeugte Welt. Es wählt keine Geländedatei und isoliert keine zweite Instanz.

Der ebene Meeresboden liegt bei z = −10 m. Eigene Böden beschreibt [Höhenkartengelände](heightmaps.md).

| Parameter (`double`) | Einheit | Bedeutung |
|---|---|---|
| `wind_speed` | m/s | Windgeschwindigkeit des Wellenmodells |
| `wind_angle` | Grad | Windrichtung |
| `steepness` | dimensionslos | Wellensteilheit |

Stimmen Sie Darstellung und Hydrodynamik aufeinander ab. [Strömungen](currents.md) werden getrennt eingestellt. Die Welt muss die Wellensysteme einbinden und konfigurieren; installierte Bibliotheken allein aktivieren sie nicht.

## Grenzen und Abhängigkeiten

Das optionale Dockerfile unterstützt den ARM64-Aufbau. Rumpf, Antrieb und Widerstand müssen für das Fahrzeug und seine Betriebsbedingungen kalibriert werden. Beim Herunterfahren kann der Supervisor MAVROS oder Gazebo beenden müssen; prüfen Sie vor einem Neustart, dass beide Prozesse beendet sind.

`extras/surface/dependency.json` legt die externe Revision fest. Der Installer baut nach `/opt/waves` und behält Quellen unter `/opt/asv_wave_sim`. Es handelt sich um eine Build-Abhängigkeit, nicht um ein Git-Submodul. Siehe [Integrationsanleitung](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md) und [Lizenzen](citation.md).
