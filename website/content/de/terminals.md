# Terminals vorbereiten

Ein **Terminal** ist ein Fenster, in dem Sie Befehle eingeben und deren Ausgaben lesen. Die Anleitungen verwenden zwei gewöhnliche Terminalfenster: **Terminal A** führt die Simulation aus; **Terminal B** liest Sensordaten oder sendet Steuerbefehle. A und B bezeichnen ihre Aufgaben, keine unterschiedlichen Programme.

## Die beiden Fenster öffnen

| Installation | Terminal A und B |
|---|---|
| [Ubuntu nativ](install.md) | Zwei Fenster der Ubuntu-Anwendung Terminal |
| [Docker unter Linux](install-ubuntu-docker.md) | Zwei Linux-Terminals, beide im selben Container |
| [WSL unter Windows](install-windows.md) | Zwei Ubuntu-Tabs in Windows Terminal, in derselben WSL-Distribution |
| [WSL mit Docker](install-windows-docker.md) | Zwei Ubuntu-Tabs, beide im selben Container |
| [Docker und RDP unter macOS](install-macos.md) | Zwei Terminals auf dem über RDP geöffneten Linux-Desktop |

Bei Docker unter Linux und Windows wechseln Sie in jedem Terminal des Hostsystems in den Container `posim`:

{{CODE:terminals:0}}

Der RDP-Desktop unter macOS läuft bereits im Container. Führen Sie ROS-Befehle dort in Linux-Terminals aus. PowerShell und das macOS-Terminal dienen Befehlen auf dem Hostsystem, etwa zum Starten eines Containers.

## Der ROS-Arbeitsbereich

Der **ROS-Arbeitsbereich** enthält Quellcode und gebaute Pakete: `src/` enthält die Quellen, `build/` die Zwischenergebnisse des Builds und `install/` die Pakete, die ROS lädt.

| Installation | Pfad |
|---|---|
| Ubuntu oder WSL nativ | `~/posim_ws` |
| Docker AMD64 | `/opt/posim_ws` |
| Docker ARM64 mit RDP | `/home/docker/posim_ws` |

`~` steht für das Home-Verzeichnis des Linux-Benutzers. Die Variable `POSIM_WS` speichert den Pfad des Arbeitsbereichs für die folgenden Beispiele.

## Umgebung in A und B laden

**Ubuntu oder WSL nativ:**

{{CODE:terminals:1}}

**Docker, auch auf dem RDP-Desktop:**

{{CODE:terminals:2}}

`source` lädt die Pfade und Einstellungen, über die das Terminal ROS und POSIM findet. Wiederholen Sie diese Schritte in jedem neu geöffneten Terminal.

## Einrichtung prüfen

{{CODE:terminals:3}}

Beide Pfade müssen unter `install/` im Arbeitsbereich liegen. Fehlt ein Paket, gehen Sie zur Installationsanleitung zurück. Liegt `ros_gz_sim` unter `/opt/ros/lyrical`, ist die Version aus dem Arbeitsbereich noch nicht geladen.

Weiter geht es mit der [ersten Simulation](quickstart.md). Solange ein Befehl Meldungen ausgibt, läuft er weiter. Beenden Sie ihn mit **Strg+C im zugehörigen Terminal** und warten Sie auf die Eingabeaufforderung.
