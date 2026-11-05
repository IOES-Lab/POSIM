# Objekte und Aufgabenszenen

POSIM enthält Modelle für Manipulation, Inspektion und Interaktion. `posim_object_models` installiert Beschreibungen, Meshes und Konfigurationen. Führen Sie die Befehle im [POSIM-Terminal Ihrer Installation](terminals.md) aus, auch unter WSL, Docker oder RDP.

## Lokaler Katalog

Die folgenden Verzeichnisse stammen aus `models/posim_object_models/description`. Darin finden Sie SDF-Dateien, Links, Gelenke und Ressourcen. Beispielwelten können zusätzlich Objekte aus Gazebo Fuel laden.

{{OBJECT_CATALOG}}

## Mit einer fertigen Szene beginnen

{{CODE:objects:0}}

Weitere Szenen sind `posim_electrical_mating` und `posim_bimanual_example`. Sie verwenden etwa eine Blowout-Preventer-Bedientafel, Stecker, Buchsen und versenkte Vasen aus Fuel. Prüfen Sie deren URIs und Positionen in der SDF, bevor Sie den Roboter platzieren oder Kontakt- und Gelenkversuche durchführen.

## Objekt in eine Welt einsetzen

Ersetzen Sie die URI durch den tatsächlichen Ressourcennamen des gewählten Modells:

{{CODE:objects:1}}

Positionen sind in Metern, Winkel in Radiant angegeben. Jede Instanz braucht einen eigenen Namen. Wählen Sie `static` passend zum Versuch: Ein festes Inspektionsziel benötigt ein anderes Bewegungsverhalten als ein Objekt, das durch Kontakt bewegt wird.

Die visuelle Geometrie bestimmt das Aussehen, die Kollisionsgeometrie den Kontakt. Prüfen Sie Maßstab, Pose, Masse, Trägheit, Gelenkgrenzen und Reibung. Beginnen Sie mit einer kleinen kontrollierten Interaktion und zeichnen Sie die Zustände über [ROS 2](ros.md) auf.

## Einzelnes Objekt starten

Starten Sie bei geladener Umgebung `mossy_cinder_block`:

{{CODE:objects:2}}

Dieser Start erzeugt eine eigene Welt. Kombinieren Sie Beispiele über `<include>`-Elemente in einer gemeinsamen Welt. Siehe [Roboter hinzufügen](custom-robots.md) und [Weltbibliothek](worlds.md).

## Fuel-Modelle lokal verwenden

Laden Sie ein Modell aus dem [Fuel-Katalog](https://app.gazebosim.org/fuel/models) herunter und entpacken Sie es. Legen Sie `model.sdf`, `model.config`, `meshes/` und `materials/` zusammen ab, etwa unter `~/posim_models/my_object/`. Registrieren Sie das übergeordnete Verzeichnis im Terminal, das Gazebo startet:

{{CODE:objects:3}}

Öffnen Sie **Resource Spawner** in Gazebo und platzieren Sie das lokale Modell. Mit `<uri>model://my_object</uri>` verwenden Sie es erneut in einer Welt-SDF. Auch die HTTPS-URI aus dem Fuel-SDF-Beispiel funktioniert; sie benötigt zunächst einen Download. Bewahren Sie Dateien und Lizenzen für Offline-Versuche auf.

## Auf Fuel veröffentlichen

Melden Sie sich bei [Gazebo Fuel](https://app.gazebosim.org/home) an und erstellen Sie ein Modell. Geben Sie Namen, Beschreibung und Lizenz an. Laden Sie `model.sdf`, `model.config` sowie referenzierte Meshes und Materialien hoch. Laden Sie das Ergebnis wieder herunter und prüfen Sie, dass es keine externen lokalen Pfade benötigt. Bei Bedarf können Sie es einer Sammlung hinzufügen.

Verwenden Sie unter Jetty verfügbare Plugin-Bibliotheken und Klassen. [Roboter hinzufügen](custom-robots.md) erläutert Auftrieb, Hydrodynamik, Schub und Sensoren.

{{MEDIA:objects}}

Abbildungen: POSIM-Notion-Wiki. Die Herkunft der DAVE-Dokumentation ist unter [Zitieren und Lizenzen](citation.md) angegeben. Nutzen Sie für Befehle und Topic-Namen die Codeblöcke dieser Seite.
