# Sonar konfigurieren und abstimmen

Wähle ein Rechenbackend und vergleiche anschließend Reichweite, Detailgrad und Aktualisierungsrate. Die [Mehrstrahlsonar-Referenz](sonar.md) enthält Schnellstart, SDF und Parameter.

## Ausführungsumgebung wählen

Nutze die [vorbereiteten POSIM-Terminals](terminals.md). Bearbeite, baue und starte in derselben Ubuntu-, WSL- oder Docker-Umgebung; `POSIM_WS` bezeichnet deren Workspace. Alle Rechenbackends benötigen Gazebos Rendering-Sensoren.

| Umgebung | Möglichkeiten |
| --- | --- |
| Natives Linux | CPU; WGPU mit nutzbarem Vulkan-Adapter; CUDA mit geeigneter NVIDIA-GPU und Toolkit |
| WSL 2 oder Linux-Container | CPU; für GPU-Berechnungen müssen Adapter und Treiber **innerhalb** der Umgebung verfügbar sein |
| macOS mit Docker/RDP | Beginne mit CPU. Der Linux-Container stellt das Metal-Gerät des Macs nicht bereit. |
| Natives macOS | WGPU kann Metal mit einer separat eingerichteten nativen ROS-/Gazebo-Werkzeugkette nutzen; die [macOS-Anleitung](install-macos.md) beschreibt Docker/RDP. |

`auto` versucht CUDA → WGPU → CPU. Wähle für Vergleichsmessungen ausdrücklich `cpu`, `wgpu` oder `cuda`. Ist das gewählte Backend nicht verfügbar, schlägt die Sensorinitialisierung fehl. Siehe die [Auswahltabelle](sonar.md#compute-backend).

## Sonarpakete bauen {#build-the-sonar-packages}

Ausgangspunkt ist die [Installation aus dem Quellcode](install.md) mit allen Abhängigkeiten. `wgpu_vendor` baut die Rust-Bibliothek auch für CPU-Betrieb. Prüfe Rust und Cargo; fehlen sie, folge der [offiziellen Installationsanleitung](https://www.rust-lang.org/tools/install).

{{CODE:sonar-tuning:0}}

Baue Bibliothek, Sensor, Weltsystem und Beispiel im Workspace mit geladener Umgebung:

{{CODE:sonar-tuning:1}}

CUDA ist optional. Um ein installiertes Toolkit beim CPU-/WGPU-Build auszulassen, ergänze `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`. `Cargo.lock` fixiert die Rust-Abhängigkeiten; der erste Download benötigt Internet.

{{CODE:sonar-tuning:2}}

Prüfe alle vier Pakete und starte das [Beispiel](sonar.md#run-a-sonar-example), bis Bilder und Rohdaten ankommen. Ein Paketeintrag bestätigt noch keinen geladenen Sensor. Prüfe die Pakete im Container; fehlen sie, baue aus den Quellen. Ein Image-Tag allein bestätigt ihren Inhalt nicht.

## WGPU-Adapter auswählen

Führe unter Linux `vulkaninfo --summary` in Gazebos Umgebung aus. Nutze die Vulkan-Treiber und -Werkzeuge des Systems. Softwareadapter wie llvmpipe können Vulkan ohne Hardwarebeschleunigung ausführen. WGPU nennt den gewählten Adapter beim Start.

{{CODE:sonar-tuning:3}}

| Variable | Bedeutung |
| --- | --- |
| `WGPU_BACKEND` | `vulkan` unter Linux, `metal` im nativen macOS-Prozess; ohne Angabe gelten Plattformvorgaben |
| `WGPU_ADAPTER_NAME` | Erster Adapter, dessen Name den Text enthält, unabhängig von Groß-/Kleinschreibung; verwende einen Namen aus Treiber- oder Startausgabe |
| `WGPU_POWER_PREF` | `high`, `low` oder `none`; ohne Adapternamen standardmäßig `high` |

Setze die Variablen vor dem Start. Beende und starte die Simulation nach Änderungen erneut: Die Initialisierung wird im Prozess gemeinsam genutzt. Das [wgpu-Projekt](https://github.com/gfx-rs/wgpu) beschreibt die APIs. Prüfe bei Initialisierungs- oder Ressourcenfehlern Adapter und Grenzen; wähle `cpu` oder verkleinere die Sensordimensionen.

## CUDA optional einrichten

Benötigt werden NVIDIA-Treiber, CUDA-Compiler und -Laufzeit, cuFFT und cuBLAS. Prüfe die Ausführungsumgebung:

{{CODE:sonar-tuning:4}}

Richte für Linux-Container das [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) auf dem Host ein. Unter Windows gelten [CUDA unter WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) und gegebenenfalls die [GPU-Einrichtung von Docker Desktop](https://docs.docker.com/desktop/features/gpu/). Installiere den Windows-Anzeigetreiber, keinen Linux-Anzeigetreiber in WSL. Auch der Container braucht CUDA-Bibliotheken.

Wähle eine von GPU und Toolkit unterstützte Architektur. Ersetze im SM-75-Beispiel `75` durch den Wert deiner GPU. `OFF` aktiviert die Toolkit-Suche nach einem reinen CPU-/WGPU-Build wieder.

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` zeigt unterstützte Ziele. Ohne Angabe gilt die Compilervorgabe. Wähle bei mehreren Toolkits `-DCUDAToolkit_ROOT=/path/to/cuda` und nimm dessen `bin` in `PATH` auf. Starte das Beispiel mit `compute_backend:=cuda`.

## Ausgangsmessung aufnehmen

Halte Szene, Zielentfernung und Sensorpose konstant. Miss in Terminal B:

{{CODE:sonar-tuning:6}}

Beende mit Strg+C und prüfe die Simulationsgeschwindigkeit:

{{CODE:sonar-tuning:7}}

Die Weltentität heißt `default`. Erfasse `real_time_factor` zusammen mit der Empfangsrate: Angeforderte 30 Hz Simulationszeit garantieren keine 30 Bilder pro realer Sekunde. Beende die Anzeige mit Strg+C.

## Abtastung und Reichweite abstimmen

Ändere jeweils einen Wert in einer Kopie des [SDF](sonar.md#sensor-configuration), baue neu, lade die Umgebung und starte mit derselben Pose.

| Versuch | Vergleich | Beobachtung |
| --- | --- | --- |
| Winkeldetail | 512 und 256 Strahlen, gleiches Sichtfeld | Zieltrennung und Empfangsrate |
| Vertikale Abtastung | 300 und 150 Rays; separat `raySkips` 10 und 5 | Zielkontinuität und Rechenzeit |
| Abdeckung | Ray-Reichweite und `maxDistance` gemeinsam ändern | Fernstes Ziel und Entfernungsskala |
| Aktualisierung | 30 und 10 Hz | Empfangsrate und Echtzeitfaktor |

Der Entfernungsabstand ist `soundSpeed / (2 × bandwidth)`; das Raster hat `ceil(maxDistance / Abstand) + 1` Zellen. Bei 1500 m/s, 29.9 kHz und 15 m sind es 599. Die letzte Koordinate kann `maxDistance` leicht überschreiten; Echos außerhalb der Reichweite werden ausgeschlossen. Lies Entfernungen und Strahlzahl aus Rohdaten oder CSV.

WGPU erhält die angeforderte Transformationslänge auch bei Nicht-Zweierpotenzen und wählt die FFT-Stufen automatisch. Größere Reichweite und Bandbreite erhöhen Speicherbedarf und Rechenaufwand. CPU ordnet Echos der nächsten Entfernungszelle zu; GPU verarbeitet im Frequenzbereich. Vergleiche beim Backendwechsel auch Amplitude und Entfernungsantwort.

## Vergleich reproduzierbar aufzeichnen

Lass den Renderer warmlaufen. Schalte `writeLog` und `debugFlag` für Durchsatzmessungen aus; zeichne getrennt mit [CSV](sonar.md) oder [ROS](visualization.md) auf. `blazingSonarImage=false` setzt den Startwert des Zufallsgenerators auf 1234.

Bewahre SDF, POSIM-Revision, Backend/Adapter, Treiber-/Toolkit-Versionen, Bildzeit, Empfangsrate, Echtzeitfaktor und Speichernutzung auf. Vergleiche rohe Intensitäts-/Entfernungsdaten zusätzlich zu Screenshots. Die [Quellenangaben](citation.md) nennen das Forschungsmodell.

## GPU-Verarbeitung

{{MEDIA:sonar-tuning}}

Die Stufen umfassen Strahlsummierung, Fensterung/Korrektur und FFT. Miss Berechnung und Speichertransfers getrennt. Abbildung: POSIM Notion Wiki.
