# Hydrostatischer Druck

Das Plugin berechnet den Druck aus der Modelltiefe und veröffentlicht ROS-Nachrichten vom Typ `FluidPressure`. Zusätzlich kann es eine aus dem Druck abgeleitete Tiefe ausgeben.

## REXROV beobachten

Starten Sie den Roboter im [für Ihre Installation vorbereiteten Terminal A](terminals.md):

{{CODE:pressure:0}}

Im vorbereiteten Terminal B:

{{CODE:pressure:1}}

Druck wird in **Pa**, Varianz in **Pa²** und Tiefe in **m** ausgegeben.

## Plugin einbinden

Setzen Sie den Block direkt in das zu messende `<model>`. `namespace` legt den Topic-Präfix fest:

{{CODE:pressure:2}}

Die Ausgabe lautet `/model/<namespace>/<topic>`; die Tiefe ergänzt `_depth`. Gemessen wird das Modell, an dem das Plugin hängt. Namespace und Modellname dürfen unterschiedlich sein.

## Parameter

| Parameter | Bedeutung und Standardwert |
|---|---|
| `namespace` | Erforderlich; gültiger ROS-Topic-Präfix |
| `topic` | Ausgabename; `sea_pressure` |
| `standard_pressure` | Endlicher, nicht negativer Oberflächendruck, kPa; `101.325` |
| `kPa_per_meter` | Endlicher, positiver hydrostatischer Anstieg, kPa/m; `9.80638` |
| `estimate_depth_on` | Berechnete Tiefe veröffentlichen; `true` |
| `update_rate` | Endliche Rate, Hz; `0`. Nicht positive Werte veröffentlichen bei jedem fortschreitenden Physikschritt |
| `saturation` | Akzeptierter Wert, `3000`; die Implementierung begrenzt damit keinen Druck |

Die Tiefe ist `max(0, -z)` in Weltkoordinaten, einschließlich übergeordneter Transformationen verschachtelter Modelle. Referenz ist die Ebene z = 0, unabhängig von Wellen. Die Rechnung erfolgt in kPa; zur Veröffentlichung wird in Pa umgerechnet.

## Zeit und Gültigkeit

ROS-Druck, Tiefe und Gazebo-Druck tragen denselben Simulationszeitstempel. Pause oder wiederholte Zeitstempel erzeugen keine neue Messung. Die Veröffentlichung setzt bei fortschreitender oder zurückgesetzter Zeit wieder ein.

Ungültige Einstellungen lassen das Plugin inaktiv. Fehlende oder nicht endliche Posen und numerische Überläufe liefern keinen Messwert.

Die veröffentlichte Varianz ist fest: **9 000 000 Pa²**. Zufälliges Druckrauschen wird nicht hinzugefügt. Berücksichtigen Sie das im Schätzer. Die Berechnungen stehen im [Plugin-Quelltext](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc).

## Tiefen vergleichen

Messen Sie zwei bekannte z-Positionen bei gleicher Drucksteigung. Die Druckdifferenz entspricht dem Anstieg pro Meter multipliziert mit der Tiefenänderung. Die abgeleitete Tiefe beruht auf derselben Rechnung und ist keine unabhängige Messung. Stimmen Sie Einheiten und Referenzdruck des Schätzers darauf ab.

## Druck- und Tiefenausgaben

{{MEDIA:pressure}}

Abbildungen: POSIM-Notion-Wiki; DAVE-Quellenangabe unter [Zitieren und Lizenzen](citation.md). Verwenden Sie Befehle und Topic-Namen aus den Codeblöcken dieser Seite.
