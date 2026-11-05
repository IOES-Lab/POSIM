# Zu POSIM beitragen

Beiträge zu Code und Dokumentation sind willkommen. Beginnen Sie mit einer gezielten, reproduzierbaren Änderung und halten Sie Modelle, Controller und Anleitungen konsistent.

## In einem Branch arbeiten

{{CODE:contributing:0}}

Verwenden Sie für Simulationsänderungen die passende [Installationsumgebung](requirements.md). Generierte Build- und Installationsverzeichnisse sowie lokale Zugangsdaten gehören nicht in den Commit.

## Das geänderte Verhalten prüfen

Wählen Sie ein kleines, passendes Beispiel. Prüfen Sie Ressourcenauflösung, Einfügen des Modells, fortschreitende Zeit, empfangene Daten, gegebenenfalls den Controller und sauberes Beenden. Notieren Sie Revision, Renderer, Architektur und Befehl.

Änderungen an Sensoren oder Dynamik erfordern einen geeigneten numerischen Vergleich. Ein Screenshot zeigt das Aussehen, belegt aber keine physikalische Genauigkeit. Bewahren Sie beim Ändern übernommenen Codes die ursprünglichen Quellenangaben.

## Alle acht Sprachfassungen aktualisieren

Die Website unterstützt Englisch, Koreanisch, Spanisch, Französisch, Deutsch, Italienisch, Japanisch und Chinesisch. Englisch liegt unter `/`, die übrigen Sprachen unter `/ko/`, `/es/`, `/fr/`, `/de/`, `/it/`, `/ja/` und `/zh/`.

Aktualisieren Sie die entsprechenden Dateien in `website/content/<Sprache>`. Befehle, Einheiten, Voraussetzungen und Grenzen müssen übereinstimmen. Formulieren Sie natürlich und verwenden Sie etablierte Robotikbegriffe. Gemeinsame `CODE`- und `MEDIA`-Platzhalter übernehmen ausführbare Beispiele und Abbildungen. Jede Sprache benötigt alle Seiten; fehlender Text wird nicht durch Englisch ersetzt.

Bei einer öffentlich nutzbaren Funktion:

- Aktualisieren Sie mit dem Code auch den [Funktionskatalog](features.md) und die Anleitung.
- Beschreiben Sie Installation und Aktivierung, Standardwerte, Einheiten, Ein- und Ausgaben, Geltungsbereich der Einstellungen, ein beobachtbares Beispiel und Modellgrenzen. Berücksichtigen Sie optionale Effekte und Adapter.
- Tragen Sie neue Seiten in `website/scripts/pages.mjs` und ihre Titel in `website/i18n/<Sprache>.json` ein. Aktualisieren Sie auch die README-Sprachfassungen.

Die Seitenreihenfolge ist gemeinsam; Welt- und Objektkataloge werden aus den Quellen erzeugt. Beschreiben Sie das aktuelle Produkt. Interne Notizen, Pläne und Validierungsberichte bleiben außerhalb des Repositories. Diskutieren Sie Änderungen und Ergebnisse in Issues und Pull Requests.

## Die Website prüfen

Führen Sie mit Node.js 24 und pnpm 10.11.0 im Verzeichnis `website` aus:

{{CODE:contributing:1}}

Öffnen Sie `http://127.0.0.1:4174`. Prüfen Sie Links, Kopieren der Befehle, Suche, Sprachwechsel und mobile Navigation. Veröffentlichung und Wartung sind in [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md) beschrieben.

## Einen Pull Request öffnen

Beschreiben Sie Problem, neues Verhalten und durchgeführte Prüfungen. Fügen Sie bei Simulationsänderungen relevante Befehle und Ausgaben hinzu, bei Layoutänderungen Ansichten für Desktop und Mobilgeräte. Benennen Sie geprüfte und noch ungeprüfte Punkte.

Nutzen Sie [GitHub Issues](https://github.com/IOES-Lab/POSIM/issues) für reproduzierbare Fehler und [Pull Requests](https://github.com/IOES-Lab/POSIM/pulls) für die Prüfung von Änderungen. IOES-Lab betreut das Projekt an der [Korea Maritime & Ocean University](https://lab.wschoi.com).
