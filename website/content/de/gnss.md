# GNSS auf See

`posim_gz_gnss` simuliert einen Satellitenempfänger am Antennenlink eines Modells. Er wandelt dessen Weltposition in geografische Koordinaten um, ergänzt einstellbare gaußsche Fehler und veröffentlicht Position, Geschwindigkeit, Zeitstempel und Zustand in einer gemeinsamen Nachricht. Empfangsverlust unter Wasser, ausdrückliche Blockierung, Wiedererfassung und Latenz sind berücksichtigt.

## Empfänger bauen und anbringen

Im [für Ihre Installation vorbereiteten Terminal A](terminals.md); unter macOS im Linux-Terminal des RDP-Desktops:

{{CODE:gnss:0}}

Das Modell benötigt einen physischen Antennenlink, meist `gnss_link`, an der gewünschten Position und mit Verbindung zum Fahrzeug. Die Welt braucht [WGS-84-Koordinaten](coordinates.md). Fügen Sie dies in die Modell-SDF ein:

{{CODE:gnss:1}}

Passen Sie Link und Topic an Ihren Roboter an. [Einen Roboter erstellen](custom-robots.md) erklärt das Vorbereiten und Starten eines Modells.

Die [Profile](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json) geben Positionen relativ zu `base_link` vor: WAM-V z = 1.8 m, BlueROV2 0.3 m und REXROV 0.8 m. WAM-V aktiviert den Empfänger; die Unterwasserprofile deaktivieren ihn zunächst. Die Profile enthalten Montagedaten: Link und Plugin müssen in das Modell eingebaut werden. WAM-V benötigt außerdem das [externe Wave-Sim-Modell](surface.md).

## Parameter

| SDF-Parameter | Einheit oder Bedeutung | Standard |
|---|---|---|
| `update_rate` | Messungen pro Simulationssekunde; >0 bis 100 Hz | 5 |
| `latency` | Auslieferungsverzögerung in Simulationssekunden; 0–10 s | 0.2 |
| `reacquire_seconds` | Durchgehende Erfassungsdauer über Wasser, s | 3 |
| `dry_margin` | Erforderlicher Abstand über Wasser zur Erfassung, m | 0.05 |
| `horizontal_stddev` | Standardabweichung der horizontalen Position, m | 0.7 |
| `vertical_stddev` | Standardabweichung der vertikalen Position, m | 1.2 |
| `velocity_stddev` | Standardabweichung der Geschwindigkeit, m/s | 0.1 |
| `water_level` | Mittlere Wasserhöhe z ohne Wellen-Topic, m | 0 |
| `seed` | Zufallsstartwert des Empfängers | 42 |
| `enabled` | Empfänger aktiviert | true |
| `link_name` | Antennenlink im Modell | `gnss_link` |
| `topic` | Gazebo-Mess-Topic | `/model/<model-name>/gnss` |
| `wave_topic` | Optionales Topic mit Wasserhöhen | Leer |

Numerische Einstellungen müssen endlich sein. Latenz, Erfassungsdauer, Abstand und Standardabweichungen dürfen nicht negativ sein.

## Lokale Wasserhöhe

Ohne `wave_topic` gilt die mittlere Wasserebene. Mit konfiguriertem Topic werden aktuelle lokale Höhen benötigt; fehlende oder ungültige Daten ergeben `surface unavailable`.

`gz.msgs.Double_V` enthält in dieser Reihenfolge: Schema `1`, Gittergröße, periodische X/Y-Abmessungen, Windgeschwindigkeit und -winkel, Wellensteilheit, Simulationszeit und zeilenweise Höhenwerte.

- Gittergröße: ganze Zahl von 2 bis 256; die Zahl der Höhenwerte entspricht ihrem Quadrat.
- X/Y-Abmessungen: endlich und positiv.
- Zeitstempel: endlich, höchstens **0.6 Simulationssekunden** alt und nicht in der Zukunft.

Richten Sie einen passenden Publisher separat ein. Die Windmetadaten steuern den Empfang nicht.

## Messungen und Blockierung lesen

Lesen Sie nach dem Start Ihres konfigurierten Modells sein Topic im vorbereiteten Terminal B:

{{CODE:gnss:2}}

Jede Nachricht enthält 15 Double-Werte. Die Indizes beginnen bei null. Behalten Sie den Aufnahmezeitstempel auch bei verzögerter Positionsausgabe.

| Index | Feld und Einheit |
|---|---|
| 0–3 | Schema `1`, Reset-Epoche, Aufnahmezeit in Simulationssekunden, Zustand |
| 4–6 | Breite in Grad, Länge in Grad, Höhe in m |
| 7–9 | Geschwindigkeit nach Osten, Norden und oben, m/s |
| 10–12 | Horizontale/vertikale Positionsvarianz in m², Geschwindigkeitsvarianz in (m/s)² |
| 13–14 | Antennenhöhe z in der Welt und lokale Wasserhöhe, m |

Zustände: `0 disabled` (deaktiviert), `1 submerged` (untergetaucht), `2 acquiring` (Erfassung), `3 fix` (gültige Position), `4 blocked` (blockiert), `5 surface unavailable` (Wasseroberfläche nicht verfügbar).

Ungültige Messungen enthalten `NaN`-Koordinaten und -Geschwindigkeiten. Wasserkontakt oder Blockierung verwerfen noch nicht ausgelieferte Messungen. Zur Wiedererfassung muss die Antenne durchgehend oberhalb des erforderlichen Abstands liegen. Ein Reset setzt den Timer zurück, verwirft verzögerte Messungen und erhöht die Epoche.

So blockieren Sie die Antenne; mit `false` geben Sie sie frei:

{{CODE:gnss:3}}

## Auswertung und Prüfung

Vergleichen Sie eine ruhende trockene, untergetauchte, blockierte und wieder freigegebene Antenne. Prüfen Sie Erfassungszeit, Verzögerung, Epoche nach Reset und steigende Zeitstempel. Verwerfen Sie veraltete oder zukünftige Positionen und behandeln Sie ungültige Zustände. Die Ausgabe nutzt Gazebo; für ROS `NavSatFix`, Autopilot oder Navigationsschätzer ist ein Adapter zum erwarteten Format nötig.

Das Modell basiert auf der Antennenpose. Satellitengeometrie, HF-Ausbreitung, physikalische Mehrwegeausbreitung und akustische Ortung werden nicht berechnet. Eingestellte Genauigkeiten sind Simulationseingaben, keine Herstellerzusicherung. Siehe [Implementierung](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc) und [Paketanleitung](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss).
