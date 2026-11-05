# macOS: Docker und RDP

Auf einem Mac mit Apple Silicon führen Sie das ARM64-Image von POSIM aus und verbinden sich per RDP mit dessen Linux-Desktop. Dort öffnen Sie Gazebo und die Terminals für die Beispiele.

## 1. Docker und RDP-Client vorbereiten

Installieren und starten Sie [Docker Desktop für Mac](https://docs.docker.com/desktop/setup/install/mac-install/). Installieren Sie außerdem die [Windows App für macOS](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps) als RDP-Client.

Öffnen Sie die macOS-App Terminal:

{{CODE:install-macos:0}}

Bei Apple Silicon muss `arm64` erscheinen. Der zweite Befehl zeigt die Version des Docker-Servers.

## 2. Image herunterladen und Container erstellen

Docker wählt automatisch ARM64:

{{CODE:install-macos:1}}

Der RDP-Port ist nur von diesem Mac aus erreichbar. Speichern Sie eigene Modelle und Aufzeichnungen unter `/work` im Volume `posim-data`. Der [Docker-Leitfaden](docker.md) erklärt Image-Versionen und lokale Builds.

## 3. Mit dem Linux-Desktop verbinden

Fügen Sie in der Windows App einen PC mit der Adresse **127.0.0.1:3389** hinzu. Wählen Sie auf dem xrdp-Anmeldebildschirm **Xorg**. Benutzername und Passwort lauten jeweils **docker**.

Öffnen Sie im Linux-Desktop ein Terminal und ändern Sie das Passwort dieses lokalen Entwicklungskontos:

{{CODE:install-macos:2}}

POSIM-Befehle gehören in diesen Desktop. Das macOS-Terminal dient zur Verwaltung von Docker. [Terminals vorbereiten](terminals.md) erklärt auch den Pfad zum Arbeitsbereich.

## 4. Stoppen und erneut verbinden

Das Schließen der RDP-Verbindung beendet die Simulation nicht. Drücken Sie zuerst **Strg+C** in ihrem Terminal. Führen Sie anschließend auf macOS aus:

{{CODE:install-macos:3}}

Verbinden Sie sich danach wieder über die Windows App. Das Image verwendet Software-Rendering; aufwendige Szenen können langsam laufen. CUDA-Mehrstrahlsonar benötigt eine unterstützte NVIDIA-GPU und ist in diesem ARM64-Image nicht enthalten. Beginnen Sie mit den [Kamera- und Drucksensorbeispielen](features.md).

## 5. Installation prüfen

Führen Sie den abschließenden Test in einem Terminal **auf dem RDP-Desktop** aus:

{{CODE:install-macos:4}}

Gazebo sollte die Ozeanwelt anzeigen. Lassen Sie das Terminal während der Simulation geöffnet. Beenden Sie sie mit **Strg+C** und fahren Sie mit [Ihrem ersten Roboter](quickstart.md) fort.

{{MEDIA:install-macos}}
