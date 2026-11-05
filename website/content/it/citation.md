# Citazioni e licenze

IOES-Lab mantiene POSIM. Quando lo utilizzi o distribuisci, conserva le attribuzioni e gli avvisi dei singoli componenti.

## Citare le basi della simulazione

L'articolo del progetto DAVE descrive l'ambiente di simulazione acquatica:

Zhang e coautori, *DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*, IEEE/OES Autonomous Underwater Vehicles Symposium, 2022. [DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808).

Per il modello di sonar multibeam:

Choi e coautori, *Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*, Frontiers in Robotics and AI, 2021. [DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646).

Per rendere riproducibile un esperimento, registra anche revisione di POSIM, configurazione del robot e del mondo e dipendenze rilevanti. Ricava l'elenco completo degli autori e i metadati bibliografici dalla pubblicazione originale.

## Attribuzione dei componenti

| Componente | Origine e avvisi |
| --- | --- |
| Codice proprio ed ereditato sotto licenza Apache | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE) e avvisi nei singoli file |
| Base DAVE | [Project DAVE](https://github.com/Field-Robotics-Lab/dave), attribuzioni e avvisi di origine |
| Wave Sim esterno | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim), licenza GPLv3 del progetto originale |
| Risorse esterne WAM-V/VRX | Avvisi conservati con la dipendenza scaricata |
| Modelli Gazebo Fuel | Origine e licenza della singola risorsa |

La compilazione delle onde conserva separatamente i sorgenti originali e le modifiche. Consulta le note su dipendenze e distribuzione in [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md). La separazione dei repository non sostituisce le condizioni di licenza dei componenti.

## Fonti della documentazione

La [documentazione originale del progetto ROS 2](https://dave-ros2.notion.site/) è una fonte per modelli, ambienti e sensori. I comandi di queste pagine usano i nomi dei pacchetti POSIM e le versioni ROS/Gazebo indicate nelle guide di installazione.

Altri riferimenti sono disponibili nella [POSIM Notion Wiki di IOES-Lab](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74) e nei collegamenti delle singole pagine.

Laboratorio e manutenzione: [IOES-Lab · KMOU](https://lab.wschoi.com). Codice e segnalazioni: [IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM).
