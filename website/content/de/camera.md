# Unterwasserkamera

`UnderwaterCamera` nutzt Tiefendaten, um RGB-Farben mit zunehmender Entfernung abzuschwächen. Passen Sie Dämpfung und Hintergrundfarbe an das gewünschte Wasser an. Gemeinsame Effekte für die ganze Szene beschreibt das [Unterwasser-Rendering](underwater-rendering.md).

<a id="run-the-example"></a>
## Beispiel starten

Dieses Beispiel verwendet den tiefenbasierten Kamerafilter. Der Szenenrenderer hat eigene [Build- und Konfigurationsschritte](underwater-rendering.md). Im [für Ihre Installation vorbereiteten Terminal A](terminals.md):

{{CODE:camera:0}}

Im vorbereiteten Terminal B:

{{CODE:camera:1}}

Fügen Sie in RViz eine Image-Anzeige hinzu. Ist das Topic sichtbar, liefert aber keine Bilder mehr, prüfen Sie die [Übertragung großer Bildnachrichten](troubleshooting.md#image-topics-appear-but-frames-stall).

## Plugin konfigurieren

Setzen Sie diesen RGBD-Sensor in einen `<link>`. Die Welt braucht `gz-sim-sensors-system` mit `<render_engine>ogre2</render_engine>`, wie in der [Kamerawelt](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world). Das Plugin liest Gazebo-Farb-/Tiefenbilder und veröffentlicht direkt auf dem ROS-Topic `/underwater_camera/simulated_image`.

{{CODE:camera:2}}

| SDF-Parameter | Bedeutung | Standard |
|---|---|---|
| `attenuationR` | Dämpfung im roten Kanal pro Meter, endlich und ≥ 0 | `1/30` |
| `attenuationG` | Dämpfung im grünen Kanal pro Meter, endlich und ≥ 0 | `1/30` |
| `attenuationB` | Dämpfung im blauen Kanal pro Meter, endlich und ≥ 0 | `1/30` |
| `backgroundR` | Roter Hintergrund, Ganzzahl 0–255 | `0` |
| `backgroundG` | Grüner Hintergrund, Ganzzahl 0–255 | `0` |
| `backgroundB` | Blauer Hintergrund, Ganzzahl 0–255 | `0` |

## Topics und Kamerageometrie

`<topic>` muss einen gültigen ROS-Namen ergeben. Fehlt es, nutzt das Plugin Gazebos Pfad `/world/<world>/model/<model>/link/<link>/sensor/<sensor>` und hängt für ROS `/simulated_image` an. Verschachtelte Modelle ergänzen die Pfade ihrer übergeordneten Modelle.

Die Umrechnung von Tiefe in Entfernung entlang des Kamerastrahls nutzt zuerst eine ausdrücklich gesetzte Linsenprojektion, danach explizite intrinsische Parameter, sonst Bildgröße und horizontales Sichtfeld. Hauptpunktkoordinaten dürfen null sein. Brennweiten müssen positiv, Linsenwerte endlich und die Clipping-Abstände in Metern `0 < near < far` erfüllen. Ungültige Einstellungen erzeugen einen Fehler und deaktivieren die Ausgabe.

## Zeitstempel und Beispielwerte

Farb- und Tiefenbilder werden unabhängig von der Ankunftsreihenfolge anhand ihres Aufnahmezeitstempels gepaart. ROS behält Zeitstempel und Frame-ID des Farbbilds bei. Ist der Puffer voll, werden Bilder ohne Partner verworfen. Ein Zurücksetzen der Simulationszeit verwirft wartende Bilder und erlaubt einen Neustart der Zeitstempel.

Das Beispiel fordert **320 × 240 bei 10 Hz**, RGB-Dämpfung **0.8/0.5/0.2** und Hintergrund **85/107/47** an. SDF-Werte überschreiben die Standardwerte des Codes.

## Abstimmen und vergleichen

Platzieren Sie ein erkennbares Objekt in bekannten Abständen. Halten Sie Pose, Licht, Materialien und Kamera fest, während Sie einen Kanal ändern. Prüfen Sie auch Zeitstempel und empfangene Bildrate.

Die Dämpfung ist pro Kanal exponentiell und wird mit dem Hintergrund gemischt. Für einen ungedämpften Vergleich setzen Sie **alle sechs Parameter** der Tabelle auf null. Fehlende Tags verwenden die Standardwerte.

Der [Szenenrenderer](underwater-rendering.md) wendet gemeinsame Effekte auf PBR-Flächen und Kameras an. Ist dieses native System aktiv, lässt `POSIM_UNDERWATER_PLUGIN` bereits eingefärbte Pixel unverändert durch und verhindert eine zweite Filterung. Das [Modell](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) legt Verbindungen und Topics fest.

## Schwebeteilchen

Der Renderer kann Schwebeteilchen in RGB-/RGBD-Farbbildern darstellen. Anfangs sind sie aus; aktivieren Sie sie über den [Appearance-Service](underwater-rendering.md#turn-on-marine-snow).

- Kameras teilen dieselben Weltpositionen der Teilchen. Diese driften gleichmäßig mit der Simulationszeit, unabhängig von der Strömung.
- Undurchsichtige Flächen verdecken sie. Der Sichtbereich beträgt **0.05–3 m**, begrenzt durch Szene und mittleren Wasserspiegel.
- Numerische Tiefe und akustische Strahlen bleiben unverändert.

Dies sind prozedurale optische Effekte. Für kalibrierte Streuungsstudien ist ein passendes optisches Modell erforderlich. Behandeln Sie Teilchen bei visueller Navigation als bewegliche Beobachtungen und halten Sie die Einstellungen zwischen Vergleichen konstant.

## Unterwasserbilder vergleichen

{{MEDIA:camera}}

Bilder: POSIM Notion Wiki; DAVE-Zuordnung unter [Zitation und Lizenzen](citation.md). Verwenden Sie die Befehle und Topics in den Codeblöcken dieser Seite.
