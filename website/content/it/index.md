# Panoramica

POSIM è una libreria open source per la simulazione di robot marini con ROS 2 e Gazebo. Consente di eseguire ambienti oceanici, robot di superficie e subacquei, sensori simulati ed esperimenti di controllo. Deriva da [Project DAVE](https://field-robotics-lab.github.io/dave.doc/), basato su [UUV Simulator](https://uuvsimulator.github.io/).

{{OVERVIEW}}

## Da dove iniziare

[Scegli l’ambiente](requirements.md) e segui la guida per [Ubuntu nativo](install.md), [Linux con Docker](install-ubuntu-docker.md), [Windows con WSL](install-windows.md), [WSL con Docker](install-windows-docker.md) o [macOS con Docker e RDP](install-macos.md). Consigliamo l’installazione nativa su Ubuntu.

Poi [prepara i terminali](terminals.md) e avvia la [prima simulazione](quickstart.md).

- ROS: **ROS 2 Lyrical**
- Simulatore Gazebo Sim: **Gazebo Jetty**

## Cosa puoi simulare

La pagina [Funzionalità ed esempi](features.md) collega le funzionalità alle implementazioni e alle relative guide.

| Attività | Guide |
| --- | --- |
| Preparare oceano e fondale | [Mondi](worlds.md), [correnti](currents.md), [effetti subacquei](underwater-rendering.md) |
| Scegliere un veicolo | [ROV](rovs.md), [Slocum](gliders.md), [robot di superficie](surface.md) |
| Osservare i sensori | [Telecamera](camera.md), [GNSS](gnss.md), [pressione](pressure.md), [DVL](dvl.md), [USBL](usbl.md), [sonar](sonar.md) |
| Controllare e registrare un esperimento | [ROS](ros.md), [Python](python-control.md), [propulsori e joystick](robot-control.md), [dati dei sensori](visualization.md) |

## Struttura di una simulazione

1. Definisci l’ambiente e i sistemi del mondo in SDF.
2. Descrivi link, collisioni, inerzie e sensori del modello.
3. Un file di lancio avvia Gazebo, il modello e i bridge ROS.
4. I nodi ROS ricevono le misure e inviano i comandi.

Per estendere una scena, segui le guide per [creare un mondo](custom-worlds.md), [aggiungere un robot](custom-robots.md) e [configurare sensori](custom-sensors.md). Ogni pagina di riferimento di un plugin offre un avvio rapido, un frammento SDF e i parametri disponibili.

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/) combina POSIM con POSIM-Terrain per il terreno geografico, POSIM-Routing per la pianificazione dei percorsi e POSIM-Control per il controllo dei veicoli. Aggiunge un’interfaccia grafica e applicazioni web e desktop per preparare scene, pilotare robot e consultare i dati di simulazione.

Il video presenta le funzionalità principali. Per installazione e utilizzo, consulta la [documentazione di WWW-POSIM](https://www-posim.vercel.app/guide/en.html).

{{INTRO_VIDEO}}

[Guarda la presentazione su YouTube](https://www.youtube.com/watch?v=_cQtl5JzN2E).
