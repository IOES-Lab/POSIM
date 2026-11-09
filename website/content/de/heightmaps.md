# Gelände aus Höhenkarten

Eine Höhenkarte beschreibt Gelände als regelmäßiges Raster von Höhenwerten. Stimmen Sie visuelle und Kollisionsgeometrie aufeinander ab, damit der sichtbare Meeresboden mit der Kontaktfläche übereinstimmt.

## Arbeitsumgebung

Nutzen Sie die [vorbereiteten Linux-Terminals](terminals.md) unter nativem Ubuntu, WSL oder Docker. Auf macOS arbeiten Sie im RDP-Desktop. Bearbeiten und bauen Sie dort; `POSIM_WS` bezeichnet diesen Arbeitsbereich.

## Beispiele ansehen

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) zeigt eine geografische Szene mit dem Fuel-Modell `Santorini Scaled`. Der [abgestufte Meeresboden](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world) ist ein weiteres Beispiel für die Weltzusammenstellung.

Für die Höhenkartengeometrie selbst verwenden Sie die [Gazebo-Anleitung](https://gazebosim.org/api/sim/10/heightmap_dem.html) und die [SDFormat-Referenz](http://sdformat.org/spec?ver=1.12&elem=geometry).

## Ressourcen im Paket speichern

Legen Sie die Welt unter `models/posim_worlds/worlds` und Daten unter `models/posim_worlds/media` ab. Prüfen Sie CMake-Installationsregeln und Ressourcenpfade des Pakets. Verwenden Sie portable URIs installierter Ressourcen.

Innerhalb einer visuellen oder Kollisionsgeometrie sieht die Grundstruktur so aus:

{{CODE:heightmaps:0}}

Ersetzen Sie die URI durch Ihre eigene Ressource. Rasterformat und Abtastung müssen zum Renderer und Physik-Backend passen. Prüfen Sie die Höhenkodierung, bevor Sie vertikale Größe und Versatz festlegen.

## Maßstab und Koordinaten erhalten

`size` beschreibt horizontale Ausdehnung und Höhenskalierung in Metern; `pos` fügt einen Versatz hinzu. Ein normiertes Bild ist nicht automatisch georeferenziert. Notieren Sie Quellausdehnung, Höhenbereich, Meeresspiegelbezug sowie Umprojektion oder Neuabtastung.

Bei geografischen Positionen setzen Sie `<spherical_coordinates>` in der Welt. Die [Koordinatenreferenz](coordinates.md) erläutert die Umrechnung zwischen Breiten-/Längengraden und lokalen Koordinaten.

## Bauen und prüfen

Bauen und laden Sie den Arbeitsbereich nach neuen Ressourcen erneut. Starten Sie außerhalb des Repositorys und prüfen Sie:

- Wiedererkennbare Geländeformen und Übereinstimmung von Darstellung und Kollision.
- Sensortiefe.
- Kontakt mit einem kontrollierten Fallversuch.
- Meeresspiegel und Starthöhe des Roboters.

Unter Jetty kann ein von null verschiedenes `<heightmap><pos>` die Darstellung verschieben, ohne die Kollision gleich zu versetzen. Auch nach einer Verschiebung über die Modellpose können Abweichungen bestehen bleiben. Prüfen Sie beides mit Ihrem Renderer und Physik-Backend. Bleiben Unterschiede, wählen Sie vor Navigationsversuchen eine Geometrie oder ein Backend, das Kontakt- und Darstellungsprüfung besteht.
