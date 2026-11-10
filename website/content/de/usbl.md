# USBL-Ortung

USBL (Ultra-Short Baseline) bestimmt Unterwasserpositionen mit akustischen Signalen. Dieses Beispiel verbindet einen Transceiver mit zwei Transpondern. Fragen Sie alle Transponder ab oder wählen Sie einen anhand seiner ID.

## Beispielwelt starten

Im [für Ihre Installation vorbereiteten Terminal A](terminals.md):

{{CODE:usbl:0}}

Die Welt enthält bereits alle drei Geräte. Empfangen Sie ihre Positionen im vorbereiteten Terminal B:

{{CODE:usbl:1}}

Beide Nachrichten enthalten `transponder_id`, ihre Felder haben jedoch unterschiedliche Bedeutungen:

| Ausgabe | x | y | z |
|---|---|---|---|
| `transponder_location` | Horizontalwinkel, Grad | Entfernung, m | Höhenwinkel, Grad |
| `transponder_location_cartesian` | Relatives x, m | Relatives y, m | Relatives z, m |

Die Richtungen beruhen auf der Differenz der Weltpositionen. Der Horizontalwinkel beträgt 0° in Richtung +x und 90° nach +y; positive Höhenwinkel zeigen nach +z. Berücksichtigen Sie diese Achsen beim Kombinieren mit Daten im Fahrzeugkoordinatensystem.

## SDF-Integration

Setzen Sie den Transceiver in das Modell `box`. Jede ID entspricht dem Modell an derselben Stelle in `transponder_attached_object`:

{{CODE:usbl:2}}

Setzen Sie diesen Transponder in `sphere` und einen zweiten in `sphere2` mit `transponder_ID` gleich `2`. Transceivername, ID und Namespace müssen an beiden Enden übereinstimmen:

{{CODE:usbl:3}}

## Parameter

| Parameter | Gerät | Bedeutung und Standard |
|---|---|---|
| `namespace` | Beide | Erforderlicher Namespace, etwa `USBL` |
| `transceiver_device`, `transceiver_ID` | Beide | Erforderlicher Gerätename und ID für die Pfade |
| `transponder_device`, `transponder_ID` | Beide | Erforderlicher Name und ID; der Transceiver akzeptiert kommagetrennte IDs |
| `transponder_attached_object` | Transceiver | Erforderliche kommagetrennte Modellnamen in ID-Reihenfolge |
| `transceiver_model` | Transponder | Erforderlicher Transceiver-Modellname für die Positionsabfrage |
| `sound_speed` | Beide | Referenzschallgeschwindigkeit bei 10 °C und z = 0, m/s; `1540`, endlich und positiv |
| `enable_ping_scheduler` | Transceiver | Erforderlicher boolescher Wert; `false` nutzt manuelle Pings |
| `ping_frequency` | Transceiver | Rate in Simulationszeit, Hz; `1`, Beispiel `0.5`; endlich und positiv |
| `interrogation_mode` | Transceiver | `common` oder `individual`; Standard `common` |
| `mu`, `sigma` | Transponder | Mittelwert und Standardabweichung des Positionsrauschens, m; `0` und `1`, Beispiel `0` und `0`; endlich, sigma ≥ 0 |

Verwenden Sie vorhandene Modelle, eindeutige IDs und gleich lange ID-/Modelllisten. IDs sind Dezimalzahlen von **0 bis 2147483647**, ohne Vorzeichen oder führende Nullen. Jedes Plugin benötigt ein Modell mit einem Link. Die [vollständige SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) enthält Geometrie und Platzierung.

Ungültige Konfiguration lässt das Plugin inaktiv. Unbekannte Kanäle oder Befehlsziele werden ignoriert; bei einem unbekannten Kanal bleibt die bisherige Auswahl erhalten. Der erste gelistete Transponder ist der Startkanal. Geplante Pings pausieren mit der Simulation und werden höchstens einmal pro Update gesendet.

## Schalllaufzeit

Jede Abfrage hat zwei Wege: Anfrage zum Transponder und Positionsantwort zurück zum Transceiver. Die jeweilige Verzögerung ist Entfernung geteilt durch die Schallgeschwindigkeit des betreffenden Geräts.

- Auch Sekundenbruchteile folgen der Simulationszeit, pausieren mit der Welt und werden bei Zeitrücklauf verworfen.
- Pro Transponder wartet höchstens eine Messung je Teilstrecke; zusätzliche Anfragen oder Antworten werden ignoriert.
- Wählen Sie den Pingabstand länger als die erwartete Hin- und Rücklaufzeit, damit jede Anfrage eine Antwort erhält.

Die effektive Geschwindigkeit lautet `sound_speed + 0.017 × depth + 4 × (temperature − 10)`. Einheiten sind m/s, Tiefe in m unter z = 0 und Temperatur in °C. Über Wasser ist die Tiefe null. Die Standardtemperatur beträgt 10 °C.

Veröffentlichen Sie `std_msgs/msg/Float64` auf `/USBL/transceiver_manufacturer_168/temperature` oder `/USBL/transponder_manufacturer_1/temperature`. Änderungen gelten für spätere Teilstrecken. Nichtendliche Temperaturen sowie Änderungen mit nichtpositiver oder nichtendlicher Geschwindigkeit werden ignoriert.

Die Näherung folgt der [DOSITS-Übersicht zur Schallgeschwindigkeit](https://dosits.org/decision-makers/tutorials/science/sound-speed/). Entfernungen sind geradlinig, ohne Brechung, Salzgehalt, Mehrwegeausbreitung oder Interferenz. Der Rückweg nutzt die empfangene Position einschließlich des eingestellten Rauschens.

## Alle Transponder abfragen

{{CODE:usbl:4}}

`common` sendet pro geplantem Ping einen Broadcast und kann mehrere IDs auf demselben Topic liefern. Bewahren Sie die ID jeder Antwort auf. Der Transceiver hält den Modus für später hinzukommende Transponder vor.

## Einen Transponder auswählen

{{CODE:usbl:5}}

Ein Kanalwechsel wählt `individual`; jeder geplante Ping richtet sich an diese ID. Wechseln Sie zurück zu `common`, um alle Geräte abzufragen.

## Anordnung und Abfragemodi

{{MEDIA:usbl}}

Bilder: POSIM Notion Wiki; DAVE-Zuordnung unter [Zitation und Lizenzen](citation.md). Verwenden Sie die Befehle und Topics dieser Seite.
