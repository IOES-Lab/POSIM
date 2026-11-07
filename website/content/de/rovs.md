# ROVs und BlueROV2

Das Argument `namespace` wählt das Verzeichnis eines installierten Modells. Die ROS-Topics ergeben sich aus dessen Konfiguration.

## Verfügbare Modelle

| Modell | Verwendung |
|---|---|
| `rexrov` | Gazebo-Thruster, Hydrodynamik und Odometrie |
| `bluerov2` | BlueROV2 mit ArduSub SITL und MAVROS |
| `bluerov2_heavy` | Heavy-Konfiguration |
| `bluerov2_heavy_multibeam_sonar` | Heavy mit CUDA-Sonar |

Verwenden Sie das [für Ihre Installation vorbereitete Terminal](terminals.md). Für REXROV folgen Sie der [ersten Simulation](quickstart.md).

## BlueROV2 starten {#start-bluerov2}

{{CODE:rovs:0}}

Mit `namespace:=bluerov2_heavy` wählen Sie die Heavy-Variante. Prüfen Sie die Autopilot-Verbindung in den Startausgaben und unter `/mavros/state`. Die Sonar-Variante benötigt die [CUDA-Einrichtung](sonar-tuning.md).

## Mit der Tastatur steuern

Verwenden Sie in einem interaktiven Desktop-Terminal `gui:=true headless:=false use_teleop:=true`.

| Taste | Aktion |
|---|---|
| `c` / `x` | Antriebe freigeben / sperren |
| `w` / `s` | Vorwärts / rückwärts |
| `a` / `d` | Nach links / rechts drehen |
| `r` / `f` | Aufsteigen / abtauchen |
| `h` / `j` | ALT_HOLD / STABILIZE |
| Leertaste | Neutraler Befehl |

## Browser-Joystick verwenden

Stellen Sie die installierte Seite in einem weiteren vorbereiteten Terminal bereit:

{{CODE:rovs:1}}

Starten Sie BlueROV2 mit aktivierter Fernsteuerung:

{{CODE:rovs:2}}

Lassen Sie `use_teleop:=true` aktiviert, da dieser Start die WebSocket-Bridge enthält. Öffnet sich der Browser nicht automatisch, rufen Sie die URL auf demselben Linux-Desktop auf. Die Docker-Images enthalten Firefox und QGC. Die Anleitung zu [QGroundControl](qgroundcontrol.md) beschreibt Installation, virtuelle Joysticks und Live-Video.

Für Abonnements und Befehle aus eigenen Programmen folgen Sie [ROS](ros.md).

## Roboter in Bewegung

{{MEDIA:rovs}}

Bilder und Videos: POSIM Notion Wiki. Die DAVE-Quellenangaben stehen unter [Zitation und Lizenzen](citation.md). Verwenden Sie die Befehle und Topics dieser Seite.
