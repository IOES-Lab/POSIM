# Fehlerbehebung

Beginnen Sie mit dem ersten Fehler in der Startausgabe. Prüfen Sie danach Umgebung, Modellressourcen, Simulationszeit, Plugins und Datenempfang. Bewahren Sie den Fehler und die für seine Reproduktion relevanten Nachbarzeilen auf.

## Ein Paket wird nicht gefunden

{{CODE:troubleshooting:0}}

Erwartet werden `lyrical` und der Pfad des gewählten Workspaces. Bereiten Sie das [Terminal](terminals.md) passend zu Ihrem [Installationsweg](requirements.md) vor. Unterscheiden Sie dabei Host- und Containerpfade.

## Modelle oder Meshes fehlen

Suchen Sie die erste nicht auflösbare URI. Prüfen Sie, ob die Paketressourcen installiert sind und die passende Umgebung geladen wurde. Für Fuel-Ressourcen benötigt die Gazebo-Umgebung Netzwerkzugriff. Der erste Download kann dauern; prüfen Sie seinen Fortschritt, bevor Sie einen weiteren Start versuchen.

## Es gibt kein Bild oder das Rendering startet nicht

Prüfen Sie Grafiktreiber und Bildschirmanbindung anhand Ihrer [Installationsanleitung](requirements.md). Gazebo-Oberfläche und Kameras benötigen einen funktionierenden Renderer. Für Sonar müssen außerdem die [CUDA-Bibliotheken](sonar-tuning.md) verfügbar sein.

Prüfen Sie bei [Unterwassereffekten](underwater-rendering.md):

- Den Dienst `/posim/appearance/<session_nonce>/state`, Renderer- und Shaderpfade sowie die Zahl aktiver Kameras.
- Die angewandte Revision und den Bereitschaftszustand nach dem Rendering.
- Ob Schwebteilchen ausdrücklich aktiviert wurden; standardmäßig sind sie aus. Das Beispiel zur Kameradämpfung lädt allein noch nicht das System für Szeneneffekte.

## GNSS liefert keine gültige Position

Prüfen Sie den [GNSS-Gültigkeitsstatus](gnss.md), die Antennenhöhe im Weltkoordinatensystem, die lokale Wasserhöhe und den Aufnahmezeitstempel. Die Welt braucht passende sphärische Koordinaten, die Antenne muss oberhalb des Wassers und unverdeckt sein. Lassen Sie genügend Simulationszeit für erneuten Empfang und Latenz verstreichen.

Ein konfiguriertes Wellen-Topic muss aktuelle, kompatible lokale Höhenwerte liefern. Die Installation des Empfängers fügt ihn weder dem Modell hinzu noch stellt sie einen ROS- oder Autopilot-Adapter bereit.

## Es kommen keine Nachrichten an

Prüfen Sie zuerst, ob die Zeit fortschreitet, und ermitteln Sie Topic und Typ:

{{CODE:troubleshooting:1}}

Verwenden Sie den richtigen Transport und dieselbe vorbereitete Umgebung. Prüfen Sie QoS, Domain sowie Bridge- und Pluginfehler. Beim Sensorstart muss `namespace` einen vorhandenen Deskriptor benennen. [USBL](usbl.md) ist bereits in seiner Beispielwelt enthalten und wird über den Weltstart geladen.

## Bild-Topics sind sichtbar, aber Bilder bleiben aus {#image-topics-appear-but-frames-stall}

Wenn Gazebo Bilder erzeugt und ROS den Publisher findet, aber kaum oder keine Bilder empfängt, prüfen Sie den DDS-Datenpfad. Große UDP-Datagramme können bei Cyclone DDS an Grenzen der IP-Fragmentierung stoßen. Kleinere Datagramme können bei einer Netzwerk-MTU von 1500 Byte helfen. Siehe [Fragmentierung in Cyclone DDS](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation).

### Ein Profil vorbereiten

Erstellen Sie dieses Profil einmal, wenn `rmw_cyclonedds_cpp` installiert ist. Verwenden Sie bereits ein Cyclone-DDS-Profil, ergänzen Sie dort die beiden Größen in `General`. Behalten Sie die Einstellungen für Discovery und Netzwerkschnittstellen bei.

{{CODE:troubleshooting:2}}

### Mit einheitlichen Einstellungen neu starten

Beenden Sie Simulation und Anzeigeprogramme. Laden Sie die POSIM-Umgebung und wählen Sie das Profil in jedem Terminal, bevor Sie die Programme neu starten. Bei einem vorhandenen Profil verwenden Sie dessen Pfad.

{{CODE:troubleshooting:3}}

Läuft in dieser Domain bereits der Hintergrundprozess der ROS-Kommandozeile, beenden Sie ihn einmal. Nachfolgende Befehle verwenden dann die neue Konfiguration:

{{CODE:troubleshooting:4}}

Wiederholen Sie die [Bild- und Ratenprüfung](camera.md#run-the-example). Prüfen Sie neben der Empfangsrate die Aufnahmezeitstempel. Kleinere Pakete verursachen zusätzlichen Aufwand; bei niedrigerer MTU müssen auch die Größen weiter sinken.

## Das Fahrzeug bewegt sich nicht

Prüfen Sie Controllerzustand, Arming, Navigationsgültigkeit, Befehlsempfang und Betriebsmodus. Kontrollieren Sie Gelenknamen und Schubachsen. Lassen Sie nur eine Befehlsquelle aktiv. Neutrale Eingaben, eine pausierte Welt oder ein nicht scharfgeschalteter Autopilot können das Fahrzeug stillhalten.

## Die Simulation beendet sich nicht

Schließen Sie die Anzeigeprogramme, drücken Sie einmal Ctrl+C im Startterminal und warten Sie auf das Ende der Prozesse. Bewahren Sie relevante Meldungen zu Abbruch, Speicherzugriffsfehlern oder erzwungenem Beenden auf. Nennen Sie beim [Fehlerbericht](https://github.com/IOES-Lab/POSIM/issues) Befehl, Revision, Betriebssystem, Architektur und Renderer.

### RViz stürzt nach Discard ab

Unter Ubuntu 26.04 kann RViz 15.2.6 gelegentlich beim Schließen des Fensters mit **Discard** abstürzen, obwohl die Sensoranzeigen funktionieren. Beenden Sie verbleibende Startprozesse mit Ctrl+C, bevor Sie neu starten. Geben Sie RViz-Version und Schließvorgang im Bericht an.

## Tiefenkameras mit einem nativen macOS-SDK neu erzeugen

Für **macOS arm64 / Ogre-Next 2.3.3** enthält die [isolierte Ogre-Umgebung](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) die Korrektur der Materiallebensdauer beim Entfernen und erneuten Erzeugen von Tiefenkameras.

Beachten Sie die vorgegebenen SDK- und Quellversionen, bauen Sie mit dem Hilfsprogramm und übergeben Sie Ihren Gazebo-Befehl an `run_overlay.py`. Ein Build allein wählt die Bibliothek noch nicht aus. Prüfen Sie die tatsächlich geladenen Pfade und bewahren Sie das Validierungsergebnis auf. Der normale Installer aktiviert diese Umgebung nicht; sie ersetzt keine Systembibliotheken und richtet keine Linux-Container ein. Andere Versionen oder Plattformen benötigen ein eigenes passendes Profil.

## Einen nachvollziehbaren Bericht erstellen

Geben Sie Quell-SHA oder Image-Digest, Abhängigkeitsversionen, Befehl und Argumente, ersten Fehler, erwartetes Verhalten, relevante Topic-Typen und ein minimales Beispiel an. Unterscheiden Sie zwischen einem nur gefundenen Topic und tatsächlich empfangenen Daten.
