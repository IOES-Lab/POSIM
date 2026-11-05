# Sonar einrichten und abstimmen

Wählen Sie Abdeckung, Detailgrad und Aktualisierungsrate passend zum Versuch. Schnellstart, SDF und Parameter stehen in der [Multibeam-Sonar-Referenz](sonar.md). Hier vergleichen Sie Einstellungen und bereiten CUDA vor.

## Benötigte Umgebung

Verwenden Sie die [vorbereiteten Linux-Terminals](terminals.md) unter Ubuntu oder unterstütztem WSL 2 mit NVIDIA-GPU, oder in einem Container mit GPU-Zugriff. Bearbeiten und bauen Sie dort; `POSIM_WS` bezeichnet den Arbeitsbereich. Das macOS-Docker/RDP-System stellt keine CUDA-GPU bereit. Führen Sie dieses Sonar-Backend auf einem NVIDIA-bestückten Linux- oder WSL-2-Host aus.

## Ausgangsmessung aufnehmen

Starten Sie das [BlueView-P900-Beispiel](sonar.md#run-a-sonar-example). Halten Sie Szene, Zielentfernung und Sensorpose konstant. Prüfen Sie Bild- und Rohdaten vor der Durchsatzmessung. Fehlt CUDA, richten Sie zunächst die Werkzeugkette ein.

Messen Sie in einem weiteren vorbereiteten Terminal die empfangene Bildrate:

{{CODE:sonar-tuning:0}}

Beenden Sie nach einer stabilen Messung mit Strg+C. Prüfen Sie die Simulationsgeschwindigkeit getrennt:

{{CODE:sonar-tuning:1}}

Der interne Weltname ist `default`. Notieren Sie `real_time_factor`: Angeforderte 30 Hz Simulationszeit garantieren keine 30 Bilder je realer Sekunde. Strg+C beendet die Anzeige.

## Eine Einstellung vergleichen

Ändern Sie eine Einstellung in einer Kopie der [Sensorbeschreibung](sonar.md#sensor-configuration). Bauen und laden Sie den Arbeitsbereich erneut, dann starten Sie mit derselben Pose.

| Versuch | Vergleich | Beobachtung |
|---|---|---|
| Winkeldetail | 512 gegen 256 horizontale Strahlen bei gleichem Sichtfeld | Zieltrennung und empfangene Rate |
| Vertikale Abtastung | 300 gegen 150 Strahlen | Zielkontinuität und Rechenzeit |
| Abdeckung | Strahlreichweite und `maxDistance` gemeinsam ändern | Entferntestes Ziel und Entfernungsachse |
| Aktualisierung | 30 gegen 10 Hz | Empfangsrate und Echtzeitfaktor |
| Strahlen überspringen | `raySkips` 10 gegen 5 | Vertikale Details und Rechenaufwand |

Messen Sie das Ergebnis jedes Vergleichs. Lassen Sie `writeLog` und `debugFlag` bei Durchsatzmessungen aus; schalten Sie sie für Rohdaten oder Zeitdiagnosen gesondert ein. Bewahren Sie SDF, POSIM-Revision, GPU-/CUDA-Versionen und Beobachtungen auf. [ROS 2](ros.md#visualization-and-recording) beschreibt Topic-Aufzeichnungen.

## Werkzeugkette prüfen

Das [Plugin](sonar.md) benötigt NVIDIA-Treiber, CUDA-Compiler und -Laufzeit, cuFFT, cuBLAS und Gazebo-Rendering. Prüfen Sie in der späteren Ausführungsumgebung:

{{CODE:sonar-tuning:2}}

GPU-Zugriff und Compiler sind getrennte Voraussetzungen. Für Docker Engine unter Linux richten Sie das [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) auf dem Host ein. Unter Windows verwenden Sie die [GPU-Einrichtung für Docker Desktop/WSL 2](https://docs.docker.com/desktop/features/gpu/). Das Image benötigt zusätzlich CUDA-Bibliotheken.

## CUDA-Architektur wählen

Setzen Sie `CMAKE_CUDA_ARCHITECTURES` auf eine von GPU und Toolkit unterstützte SM-Version. Das Beispiel verwendet SM 75. Ersetzen Sie `75` durch Ihre Architektur und bauen Sie im vorbereiteten Arbeitsbereich:

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` zeigt unterstützte Ziele. Ohne explizite Architektur verwendet CMake die Compilervorgabe. Bei mehreren CUDA-Installationen wählen Sie mit `-DCUDAToolkit_ROOT=/path/to/cuda` die passende aus und ergänzen deren `bin`-Verzeichnis in `PATH`.

## Installierte Komponenten prüfen

{{CODE:sonar-tuning:4}}

Pakete können registriert sein, obwohl CUDA-Ziele ausgelassen wurden. Prüfen Sie die Bibliotheken:

{{CODE:sonar-tuning:5}}

Jeder Befehl muss mit Status 0 enden. Prüfen Sie ausgelassene Ziele oder Ladefehler und empfangen Sie anschließend die unter [Multibeam-Sonar](sonar.md) beschriebenen Daten.

## Messungen auswerten

Lassen Sie den Renderer vor Zeitmessungen anlaufen. Erfassen Sie bei gleicher Szene und Pose Bildverarbeitungszeit, Empfangsrate, Echtzeitfaktor, CPU-/GPU-Auslastung und Speicher. Trennen Sie Physik, Rendering, CUDA, Veröffentlichung und Protokollierung.

`blazingSonarImage` wählt wechselnde Zufalls-Startwerte oder den festen Startwert `1234`; vergleichen Sie die Wiederholbarkeit. Behalten Sie bei Änderungen numerischer Kernel Referenzdaten und vergleichen Sie Intensität und Entfernung zusätzlich zu Bildern. Die zugrunde liegende Forschung finden Sie unter [Zitieren](citation.md).

## GPU-Verarbeitung

{{MEDIA:sonar-tuning}}

Die Verarbeitung umfasst Strahlensummierung, Fensterung/Korrektur und FFT. Trennen Sie diese Stufen beim Profiling von GPU-Speichertransfers. Abbildung: POSIM-Notion-Wiki.
