# Thruster und Joystick-Steuerung

Wählen Sie die zum Roboter passende Schnittstelle. Prüfen Sie zuerst die Sensordaten und aktivieren Sie jeweils nur einen Regler.

| Fahrzeug | Steuerpfad | Anleitung |
|---|---|---|
| REXROV | ROS-Kraftbefehle → Gazebo-Thruster | [Python](python-control.md) |
| BlueROV2 | ROS/Joystick → MAVROS → ArduSub | [QGC und Video](qgroundcontrol.md) · [ROS/Joystick](rovs.md) |
| WAM-V | Oberflächenintegration und Fahrzeugregler | [Oberflächenfahrzeuge](surface.md) |

## REXROV-Thruster

Lassen Sie REXROV im [vorbereiteten Terminal A](terminals.md) laufen und senden Sie Befehle aus B. Die Konfiguration verbindet acht `cmd_thrust`-Topics als `std_msgs/msg/Float64`. Die Werte geben Schub in Newton an. Ein neutraler Befehl für den ersten Thruster lautet:

{{CODE:robot-control:0}}

Verteilen Sie die Kräfte anhand der Positionen und Achsen der Thruster. Die Fahrzeuggeschwindigkeit lässt sich nur durch abgestimmte Ansteuerung regeln. Senden Sie zum Abschluss null an alle verwendeten Thruster.

## BlueROV2 mit ArduSub

Starten Sie das [BlueROV2-Beispiel](rovs.md#start-bluerov2), warten Sie auf die Autopilot-Verbindung und prüfen Sie die Schnittstellen:

{{CODE:robot-control:1}}

Die Durability-Einstellung `volatile` wählt neue Zustandsnachrichten aus; der Filter wartet nach dem Verbindungsaufbau auf `connected: true`. Bei anhaltender Wartezeit prüfen Sie Startausgaben und MAVROS-/ArduSub-Konfiguration.

Der Adapter setzt `/joy` und `/keyboard/joy` in MAVROS-Befehle um:

- Veröffentlichungsrate: **20 Hz**.
- Eingabe-Timeout: **0,3 s**.

Nutzen Sie eine Befehlsquelle und prüfen Sie Antriebsfreigabe und Navigationsstatus. [ROVs und BlueROV2](rovs.md) erläutert Tastatur und WebSocket-Bridge.
