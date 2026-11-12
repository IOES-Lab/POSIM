# Mit QGroundControl steuern

Verbinden Sie QGroundControl (QGC) mit dem simulierten BlueROV2, bedienen Sie die virtuellen Joysticks und betrachten Sie das Bild der Frontkamera. Schließen Sie die [Installation](requirements.md) ab und bereiten Sie [Terminal A und B](terminals.md) vor.

## 1. QGC in der POSIM-Umgebung öffnen

**Windows/WSL:** Verwenden Sie für diese Tutorials die Linux-Version von QGC. Starten Sie nicht die unter Windows installierte Anwendung. QGC muss in derselben Ubuntu-Distribution wie POSIM laufen, bei Docker im selben POSIM-Container. WSLg kann das Linux-Fenster auf dem Windows-Desktop anzeigen; die Anwendung läuft weiterhin in Linux.

Die Adressen `127.0.0.1` in dieser Anleitung beziehen sich auf die Umgebung, in der QGC läuft. Damit wird eine Windows-Anwendung nicht für den Empfang von Telemetrie oder Video aus WSL oder Docker eingerichtet.

| Installation | Wo QGC ausgeführt wird |
|---|---|
| Ubuntu nativ | Linux-Anwendung in Ubuntu installieren und starten. |
| Windows mit nativer WSL-Installation | Linux-Anwendung in derselben Ubuntu-Distribution installieren und starten. |
| Linux oder Windows mit Docker | Mitgelieferte Anwendung im POSIM-Container starten. |
| macOS mit Docker und RDP | Anwendung in einem Terminal des RDP-Desktops starten. |

Für native Installationen folgen Sie der [offiziellen QGC-Anleitung](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html). Wählen Sie die Linux-Version für Ihre Prozessorarchitektur. Im Container verwenden Sie Terminal B:

{{CODE:qgroundcontrol:0}}

Das `&` gibt die Eingabeaufforderung frei, während QGC geöffnet bleibt. Lassen Sie auch ein separat für QGC geöffnetes Terminal weiterlaufen. Wählen Sie bei den Einstellungen zum ersten Start **Metric System**.

QGC, MAVROS und Gazebo laufen in derselben Linux-Umgebung. Telemetrie verwendet **UDP 14550**, Video **UDP 5600**. Dafür sind keine zusätzlichen Docker-Portfreigaben nötig. Unter macOS greifen Sie per RDP auf diese Umgebung zu.

## 2. Joysticks vorbereiten

Öffnen Sie **Application Settings → Fly View → Virtual Joystick**:

- Schalten Sie **Virtual Joystick** ein.
- Schalten Sie **Auto-Center Throttle** ein, damit der Steuerbefehl beim Loslassen zur Mitte zurückkehrt.
- Lassen Sie **Left-Handed Mode** für die hier verwendete Anordnung ausgeschaltet.

## 3. Videoempfang einstellen

Öffnen Sie **Application Settings → Video**. Wählen Sie zuerst die Videoquelle und tragen Sie anschließend UDP-Adresse und weitere Einstellungen aus der Tabelle ein.

| Einstellung | Wert |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`, automatisches Seitenverhältnis |
| Disable Video Stream When Disarmed | Aus |
| Low Latency Mode | Ein |
| Auto-reconnect on stream loss | Ein |

## 4. Roboter und Kamera starten

Beenden Sie die vorherige Simulation. Im vorbereiteten Terminal A:

{{CODE:qgroundcontrol:1}}

Gazebo öffnet die Meeresbodenszene; QGC verbindet sich mit ArduSub. Warten Sie, bis die Initialisierung abgeschlossen ist. In Terminal B:

{{CODE:qgroundcontrol:2}}

Wechseln Sie zu **Fly View** und klicken Sie auf die Videovorschau, um sie zu vergrößern.

{{MEDIA:qgroundcontrol}}

Die Kamera liefert Bilder mit **640 × 480 Pixeln bei 5 Hz bezogen auf die Simulationszeit**. Software-Rendering kann die tatsächliche Bildrate verringern. BlueROV2 hat positiven Auftrieb: Auch bei deaktivierten Antrieben kann es aufsteigen, sodass die Objekte aus dem Bild verschwinden.

`qgc_video:=true` fügt den H.264-Sender hinzu. Das RGB-Bild enthält die [Unterwassereffekte der Szene](underwater-rendering.md). Der separate [Kamerafilter](camera.md) hat einen eigenen ROS-Ausgang. GNSS verliert beim Untertauchen der Antenne die Position und erfasst sie nach dem Auftauchen erneut; Telemetrie, Video und manuelle Steuerung bleiben verbunden. Verwenden Sie unter Wasser Manual. Die Linse liegt unter der Antenne, sodass trotz gültiger GNSS-Position ein Unterwasserbild möglich ist.

## 5. Antriebe freigeben, fahren und stoppen

1. Warten Sie auf **Ready** und wählen Sie **Manual**.
2. Öffnen Sie das Fahrzeugstatusmenü und halten Sie **Arm** gedrückt, bis der Vorgang abgeschlossen ist.
3. Bewegen Sie den rechten Joystick kurz vor und zurück.
4. Lassen Sie ihn los, um den Befehl auf neutral zurückzusetzen. Strömung und Trägheit können den Roboter weiterhin bewegen.
5. Halten Sie zum Abschluss **Disarm** im Statusmenü gedrückt.

Lassen Sie Tastatursteuerung und Web-Joystick deaktiviert, damit keine konkurrierenden Befehle entstehen. Beenden Sie die Videoübertragung in B:

{{CODE:qgroundcontrol:3}}

Drücken Sie in A **Strg+C** und schließen Sie QGC.

MAVROS meldet einen Verbindungsverlust, wenn bis zu 60 Sekunden lang kein Heartbeat des Fahrzeugs eintrifft. Damit wird langsames Software-Rendering berücksichtigt; ein tatsächlicher Ausfall der Heartbeats kann deshalb erst nach bis zu 60 Sekunden in `/mavros/state` erscheinen.

## Verbindungsprobleme beheben

- **Kein Fahrzeug:** Prüfen Sie die gemeinsame Linux-Umgebung und ob eine zweite QGC-Instanz UDP 14550 belegt.
- **Waiting for video:** Prüfen Sie H.264-Quelle, UDP-Adresse und `qgc_video:=true`. Aktivieren Sie die Übertragung in B und warten Sie einige Bilder ab.
- **Anderer Port:** Verwenden Sie `qgc_video_port:=5602` und in QGC `127.0.0.1:5602`.
- **Option `qgc_video` fehlt:** Aktualisieren Sie POSIM oder [laden Sie das Image und erstellen Sie den Container neu](docker.md#update-an-existing-container).
- **Firmware- oder serielle Hinweise:** Die Simulation verwendet das enthaltene ArduSub und eine Netzwerkverbindung. Firmware muss dafür weder geflasht noch eine serielle Schnittstelle eingerichtet werden.

Weitere Optionen finden Sie in den [offiziellen Videoeinstellungen](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html). Die ROS-Schnittstellen erläutert [Thruster und Joystick-Steuerung](robot-control.md).
