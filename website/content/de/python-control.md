# Mit Python steuern

Lassen Sie REXROV aus der [ersten Simulation](quickstart.md) weiterlaufen. Lesen Sie in einem zweiten [vorbereiteten Terminal](terminals.md) seine Position und steuern Sie anschließend kurz einen Thruster an.

## Position abonnieren

Speichern Sie dieses Programm als `observe.py` und starten Sie `python3 observe.py` in der geladenen ROS-Umgebung.

{{CODE:python-control:0}}

Etwa einmal pro Sekunde erscheint die Position. Strg+C beendet den Beobachter; der Simulator läuft weiter.

## Zeitlich begrenzten Befehl senden

Speichern Sie das folgende Programm als `pulse_thruster.py` und starten Sie `python3 pulse_thruster.py`. Es wartet auf einen Subscriber, gibt für ungefähr zwei Sekunden Host-Zeit **5 N an Thruster 1** vor und sendet danach null. Die Einbauachse bestimmt die entstehende Kraft und das Drehmoment.

{{CODE:python-control:1}}

Die Signaleinstellung sorgt dafür, dass Strg+C den Aufräumblock erreicht, während der ROS-Publisher noch verfügbar ist. Bei einem erzwungenen Prozessabbruch beenden Sie den Simulator oder senden neutrale Befehle wie unter [Thruster-Steuerung](robot-control.md).

## Reaktion beobachten

Beobachten Sie die Winkelgeschwindigkeit während des Impulses in einem weiteren vorbereiteten Terminal:

{{CODE:python-control:2}}

Sie sollte auf den Befehl reagieren und anschließend gegen null gehen. Vergleichen Sie die Odometrie, um Aktuatorreaktion und Fahrzeugbewegung zu unterscheiden. Strg+C beendet die Anzeige.

## Eigenen Regler entwickeln

Verteilen Sie die Kräfte anhand des gemessenen Zustands und Ihres Regelgesetzes auf die acht REXROV-Thruster. Halten Sie Topics, Einbauachsen und Einheiten konsistent. Verwenden Sie Simulationszeit für die Versuchslogik und setzen Sie beim Beenden alle angesteuerten Aktuatoren auf neutral.

BlueROV2 nutzt den [MAVROS-/ArduSub-Pfad](robot-control.md). Für den Vergleich von Versuchen können Sie [Beobachtungen aufzeichnen](visualization.md).
