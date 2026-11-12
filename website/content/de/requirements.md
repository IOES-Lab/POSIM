# Umgebung auswählen

Wählen Sie eine Methode für das Betriebssystem Ihres Rechners. **Empfohlen wird Ubuntu 26.04 mit nativer Installation**: Gazebo-Grafik, ROS-Werkzeuge und NVIDIA/CUDA-Geräte sind damit direkt zugänglich.

## Installationswege

| Betriebssystem | Methode | Anleitung |
| --- | --- | --- |
| Ubuntu 26.04 | POSIM direkt unter Ubuntu bauen und ausführen | [Ubuntu nativ](install.md) |
| Ubuntu oder eine andere von Docker unterstützte Linux-Distribution, x86-64 | Ubuntu-basiertes POSIM-Image in Docker; Anzeige auf dem Linux-Desktop | [Linux mit Docker](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | Ubuntu 26.04 in WSL2; dort nativ bauen, Fenster über WSLg anzeigen | [Windows mit WSL](install-windows.md) |
| Windows 10/11, x86-64 | Docker Desktop mit WSL2; POSIM-Image und Fenster über WSLg | [WSL und Docker](install-windows-docker.md) |
| macOS auf Apple Silicon | ARM64-Image in Docker; Linux-Desktop über RDP bedienen | [macOS mit Docker und RDP](install-macos.md) |

Das Image enthält seine eigene Ubuntu-Umgebung. Der Linux-Host darf eine andere von Docker unterstützte Version oder Distribution verwenden. ARM64-Emulation auf Intel-Macs wird hier nicht behandelt.

Verwenden Sie für diese Version unter macOS Docker/RDP. Eine native macOS-Installation ist noch nicht vollständig geprüft. Angaben zur nativen Metal-Unterstützung beziehen sich auf einzelne Komponenten und bestätigen keine Unterstützung der gesamten Tutorial-Umgebung.

## Software und Grafik

Alle Wege verwenden **ROS 2 Lyrical** und **Gazebo Sim — Gazebo Jetty**. Bei der nativen Installation werden sie unter Ubuntu eingerichtet; die Images enthalten sie bereits. Installieren Sie den Grafiktreiber auf dem Host entsprechend der gewählten Anleitung.

Gazebo sowie Farb- und Tiefenkameras benötigen einen funktionierenden Renderer. Beginnen Sie mit einem Roboter und seinen Standardsensoren. Das Mehrstrahlsonar bietet CPU, WGPU und CUDA: [Wählen Sie das passende Backend](sonar-tuning.md). WGPU benötigt einen nutzbaren Vulkan-Adapter unter Linux oder Metal unter nativem macOS. Docker/RDP unter macOS nutzt Linux ohne Metal-Zugriff; beginnen Sie mit CPU.

## Ressourcen und Downloads

Der Bedarf hängt von Fahrzeugzahl, Geländedetails, Bildauflösung und Sensorrate ab. Beobachten Sie Speicher, GPU-Auslastung und Echtzeitfaktor, bevor Sie diese Werte erhöhen. Planen Sie Speicherplatz für Quellcode, Build-Dateien, Docker-Images, Fuel-Modelle und Aufzeichnungen ein.

Die Installation benötigt Zugriff auf ROS/Gazebo-Paketquellen, GitHub und Docker Hub. Manche Welten laden beim ersten Start Fuel-Modelle herunter. Wenn die Demowelt läuft, folgen die [Terminals](terminals.md).
