# Sphärische Koordinaten

Das ROS-Plugin `SphericalCoords` verwaltet den geografischen Weltursprung und rechnet zwischen lokalen kartesischen und geografischen Koordinaten (Breiten-/Längengrad und Höhe) um.

## Konfigurierte Welt starten

Im [für Ihre Installation vorbereiteten Terminal A](terminals.md):

{{CODE:coordinates:0}}

Im vorbereiteten Terminal B:

{{CODE:coordinates:1}}

Die Beispiel-SDF setzt **35.074823°** Breite und **129.084798°** Länge. Die Antwort zeigt den Ursprung der laufenden Welt.

## SDF-Integration und Parameter

Platzieren Sie beide Blöcke direkt unter `<world>`:

{{CODE:coordinates:2}}

Das Plugin liest die Weltkoordinateneinstellungen und hat keine eigenen SDF-Parameter. Seine vier Servicepfade liegen fest unter `/gz/`. Außerhalb einer Welt oder ohne geografische Konfiguration meldet es einen Fehler und lässt die Services deaktiviert.

| Weltfeld | Bedeutung | Beispiel |
|---|---|---|
| `surface_model` | Referenzellipsoid | `EARTH_WGS84` |
| `world_frame_orientation` | Achsenkonvention | `ENU` |
| `latitude_deg`, `longitude_deg` | Ursprung, Grad | `35.074823`, `129.084798` |
| `elevation` | Höhe des Ursprungs, m | `0` |
| `heading_deg` | Richtungsversatz, Grad | `0` |

Definieren Sie einen ausdrücklichen Ursprung. Siehe [Plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc) und [Beispielwelt](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world).

## Services

Die Typen liegen in `posim_interfaces/srv`. Winkel sind in Grad, lokale Koordinaten und Höhe in Metern angegeben.

| Service-Endung unter `/gz/` | Typ | Anfrage |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Leer |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Alle Werte müssen endlich sein. Breitengrade reichen von −90° bis 90°, Längengrade von −180° bis 180°. Eine ungültige Ursprungsänderung liefert `success: false` und erhält den Ursprung. Fehlgeschlagene Umrechnungen liefern `NaN`-Koordinaten; prüfen Sie sie vor der Nutzung im Controller.

## Lokale Position umrechnen

{{CODE:coordinates:3}}

Die Antwort enthält Breite, Länge und Höhe. Übergeben Sie diese an den inversen Service und vergleichen Sie `output` mit dem Ausgangsvektor. So prüfen Sie Ursprung und Achsenkonvention.

## Ursprung ändern

{{CODE:coordinates:4}}

Die Änderung passt die Zuordnung lokaler zu geografischen Positionen an. Stimmen Sie Gelände, Weltausrichtung, Höhenbezug und Controller darauf ab. Die Umrechnung verwendet Gazebo; berücksichtigen Sie den in SDF eingestellten Richtungsversatz und die Orientierung.
