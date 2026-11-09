# Unterwassereffekte für die Szene

`posim_gz_rendering` wendet Unterwasserdunst, RGB-Absorption, tiefenabhängiges Sonnenlicht, Kaustiken und Schwebeteilchen auf Ogre2-PBR-Flächen an. Die Teilchen ähneln Meeresschnee und sind anfangs **ausgeschaltet**.

Gazebo-Szene, RGB-Kameras und RGBD-Farbbilder teilen diese Effekte. Numerische Tiefe und Kollisionsgeometrie bleiben erhalten. Den separaten tiefenbasierten Bildfilter beschreibt [Unterwasserkamera](camera.md).

## System bauen und einbinden

Verwenden Sie eine zum Server passende Gazebo-Jetty/Ogre2-Entwicklungsumgebung. Bauen Sie im [vorbereiteten Terminal A](terminals.md) aus den POSIM-Quellen; unter macOS im Linux-Terminal des RDP-Desktops:

{{CODE:underwater-rendering:0}}

CMake benötigt Gazebo-Entwicklungspakete sowie die pkg-config-Einträge `OGRE-Next` und `OGRE-Next-Hlms`. Der Befehl ergänzt den Pfad des ROS-Pakets `gz_ogre_next_vendor` unter Ubuntu. Verwenden Sie Bibliotheken, Header und Shader aus demselben Build.

Fügen Sie dieses Plugin unter `<world>` ein. Ersetzen Sie `/absolute/underwater` durch das Installationspräfix von oben. Laden Sie eine Instanz pro Gazebo-Prozess und vergeben Sie je Sitzung eine eigene `session_nonce`:

{{CODE:underwater-rendering:1}}

Linux verwendet `.so`, auch in Docker/RDP unter macOS. Direkt für macOS gebaute Bibliotheken verwenden `.dylib`. Zum Erzeugen von Sensorbildern braucht die Szene eine aktive Kamera.

Bei zusätzlichem `UnderwaterCamera` setzen Sie `POSIM_UNDERWATER_PLUGIN` in der Serverumgebung auf den Pfad des geladenen Systems. Bereits eingefärbte Pixel werden dadurch ohne zweite Dämpfung weitergegeben. Setzen Sie es nur, wenn das Szenensystem installiert und geladen ist.

## Effekte und Standardwerte

| Einstellung | Wirkung | Standard |
|---|---|---|
| Wasseroptik | Hauptschalter für optische Effekte | Ein |
| Unterwasserdunst | Farbmischung abhängig vom Lichtweg im Wasser | Ein |
| RGB-Absorption | Wellenlängenabhängiger Farbverlust | Ein |
| Sonnenlichtdämpfung | Schwächeres gerichtetes Licht mit der Tiefe | Ein |
| Kaustiken | Bewegte Lichtmuster auf untergetauchten Flächen | Ein |
| Schwebeteilchen / Meeresschnee | Vereinzelt verteilte prozedurale Lichtpunkte | Aus |
| Bodentextur | Details aus den PBR-Karten des Meeresbodens | Ein |
| Bodengitter | Abstand 10 m, einstellbar von 1 bis 500 m | Aus |
| Tiefenlinien | Abstand 5 m, einstellbar von 1 bis 50 m | Aus |

Beim Ausschalten der Wasseroptik bleiben Textur-, Gitter- und Tiefenlinienauswahl erhalten. Bodenüberlagerungen erkennen Visuals namens `seabed` oder `shallow`, einschließlich vollständiger Gazebo-Namen. Stellen Sie Texturen in den Geländeressourcen bereit; siehe [Gelände mit Höhenkarten](heightmaps.md).

<a id="turn-on-marine-snow"></a>
## Meeresschnee einschalten

Lesen Sie bei laufender Welt den Zustand im vorbereiteten Terminal B:

{{CODE:underwater-rendering:2}}

Die ersten 13 Werte haben diese Reihenfolge; die Indizes beginnen bei null:

| Index | Wert |
|---|---|
| 0–1 | Schema `1`, Revision |
| 2–5 | Wasseroptik, Bodentextur, Gitter, Tiefenlinien |
| 6–10 | Dunst, Absorption, Sonnenlichtdämpfung, Kaustiken, Teilchen |
| 11–12 | Gitter- und Tiefenlinienabstand, m |

Bei einem frisch gestarteten System mit Revision `0` aktiviert diese Anfrage Teilchen und erhält die übrigen Standardwerte:

{{CODE:underwater-rendering:3}}

Behalten Sie bei späteren Änderungen die gewünschten Einstellungen und erhöhen Sie die Revision um eins. Boolesche Werte sind `0` oder `1`. Eine identische Anfrage darf wiederholt werden.

Lesen Sie nach dem Rendern erneut `/state`: Die Indizes 13–15 ergänzen `ready`, Kamerazahl und `renderer_failed`. Prüfen Sie die gewünschte Revision, `ready = 1` und `renderer_failed = 0`. Die `/set`-Antwort bestätigt nur den Empfang.

Für Teilchen direkt beim Start setzen Sie diesen Tag in das Plugin, anstelle der ersten Anfrage:

{{CODE:underwater-rendering:4}}

WWW-POSIM verwendet diesen öffentlichen Renderer und ergänzt ein eigenes Einstellungsfenster, Sitzungssteuerung und gespeicherte Einstellungen.

## Modellgrenzen und Vergleich

Die Berechnung der Lichtweglänge unter Wasser verwendet eine mittlere Meeresspiegelebene bei **z = 0 m** und folgt nicht einzelnen Wellenkämmen. Kaustiken und Teilchen sind visuelle Näherungen ohne kalibrierte Partikelkonzentrationen oder optisches Sensorrauschen.

Lokale Lampen behalten direkte Beleuchtung. Himmelsstrahlen werden unter Wasser gedämpft; unbeleuchtete Materialien und eigene Shader der Wasseroberfläche behalten ihr Rendering. Die Wellenunterseite hängt daher vom Oberflächenmaterial ab.

Vergleichen Sie Ansichten über/unter Wasser, RGB und RGBD-Farbe sowie jeden Schalter. Halten Sie Pose und Licht konstant. Prüfen Sie byte-identische Tiefe, mit der Simulationszeit pausierende Effekte und gemeldete Rendererfehler. Das [öffentliche Paket](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering) enthält RGB/RGBD-Prüfbeispiele, Shader und Build-Details.
