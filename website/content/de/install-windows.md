# Windows: Installation in WSL

Installieren Sie POSIM in Ubuntu 26.04 unter Windows Subsystem for Linux 2 (WSL2). WSLg zeigt Gazebo auf dem Windows-Desktop an. Ein vorbereitetes Image verwenden Sie über [WSL mit Docker](install-windows-docker.md).

## 1. WSL und Ubuntu installieren

Öffnen Sie PowerShell als Administrator:

{{CODE:install-windows:0}}

Starten Sie Windows neu, falls Sie dazu aufgefordert werden. Öffnen Sie Ubuntu und legen Sie ein Linux-Benutzerkonto an. Prüfen Sie in PowerShell, ob die Distribution WSL 2 verwendet:

{{CODE:install-windows:1}}

Bei vorhandenem WSL oder Installationsproblemen hilft die [Microsoft-Anleitung](https://learn.microsoft.com/de-de/windows/wsl/install).

## 2. Grafiktreiber und WSLg vorbereiten

Installieren oder aktualisieren Sie den **Windows-Grafiktreiber** für Ihre Intel-, AMD- oder NVIDIA-GPU. Das gilt auch für NVIDIA. WSLg nutzt diesen Treiber zur Grafikbeschleunigung; siehe [grafische Anwendungen unter WSL](https://learn.microsoft.com/de-de/windows/wsl/tutorials/gui-apps).

Befolgen Sie bei NVIDIA die Anleitung zu [CUDA unter WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html): Installieren Sie den Windows-Treiber, keinen Linux-Anzeigetreiber innerhalb von WSL. Das CUDA Toolkit wird nur zum Bauen des CUDA-Sonarbackends benötigt.

Aktualisieren Sie WSL in PowerShell:

{{CODE:install-windows:2}}

Speichern Sie vor einem angeforderten Neustart Ihre Arbeit in den WSL-Terminals. Öffnen Sie Ubuntu anschließend erneut.

## 3. POSIM im Ubuntu-Terminal installieren

Verwenden Sie für Linux-Befehle das Profil **Ubuntu** in Windows Terminal. Prüfen Sie die Distribution:

{{CODE:install-windows:3}}

Folgen Sie [Ubuntu: native Installation](install.md) vom Herunterladen der Quellen bis zum Build. Legen Sie den Arbeitsbereich unter `~/posim_ws` im Linux-Dateisystem an. Das Installationsskript nutzt `sudo` und aktualisiert Ubuntu-Pakete.

Sind Bild-Topics sichtbar, aber es kommen keine Bilder an, hilft die [Fehlerbehebung](troubleshooting.md). [Terminals vorbereiten](terminals.md) erklärt die beiden Fenster für weitere Beispiele.

## 4. Die Demo-Welt starten

Führen Sie diese Befehle im Ubuntu-Terminal aus. Gazebo sollte auf dem Windows-Desktop erscheinen und die Ozeanszene zeigen. Beim ersten Start können Fuel-Modelle heruntergeladen werden. Beenden Sie die Simulation mit Strg+C, bevor Sie ein weiteres Beispiel starten.

{{CODE:install-windows:4}}
