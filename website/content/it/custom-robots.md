# Aggiungere un robot

Parti da un robot POSIM con struttura simile. L'SDF descrive meccanica e sistemi; la configurazione ROS collega misure e comandi.

## Ambiente di lavoro

Usa i [terminali Linux preparati](terminals.md) su Ubuntu nativo, WSL o Docker. Su macOS lavora nel terminale del desktop RDP. Modifica e compila lì; `POSIM_WS` è il percorso del relativo spazio di lavoro.

## 1. Copiare un modello di riferimento

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) usa propulsori Gazebo. [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) integra ArduSub/MAVROS. [Slocum](gliders.md) mostra un'altra disposizione.

Da `$POSIM_WS`, copia interamente REXROV per mantenere insieme mesh, configurazione del bridge e nomi dei topic:

{{CODE:custom-robots:0}}

Salva il programma seguente come `copy_robot.py`, poi esegui `python3 copy_robot.py`. Si ferma se `my_robot` esiste già.

{{CODE:custom-robots:1}}

La struttura risultante comprende:

{{CODE:custom-robots:2}}

`my_robot` è l'identificativo scelto. L'avvio cerca `description/<namespace>/model.sdf` e la configurazione corrispondente.

## 2. Definire meccanica e risorse

Mantieni la meccanica copiata per il primo avvio. Quando modifichi lo scafo, controlla insieme:

| Percorso SDF | Unità | Verifica |
|---|---|---|
| `model/@name`, nomi di link e giunti | identificativo | Corrispondenza con plugin e configurazione ROS |
| `link/inertial/mass` | kg | Massa positiva per ogni link mobile |
| `link/inertial/pose` | m, rad | Centro di massa e orientamento del riferimento d'inerzia |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | Momenti rispetto ai suoi assi |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | Termini fuori diagonale; tensore completo fisicamente valido |
| `link/visual`, `link/collision` | m, rad | Scala e allineamento delle geometrie |
| `joint/axis`, impostazioni dei propulsori | secondo il plugin | Direzione degli attuatori e ripartizione delle forze |

STL fornisce la geometria. Definisci massa e dinamica nell'SDF e controlla unità e scala della mesh. Segui gli URI installati degli esempi. Scegli collisioni semplici quando il dettaglio visivo rende inutilmente costoso il contatto.

## 3. Collegare sistemi e ROS

Adatta idrodinamica, galleggiamento, assi dei propulsori e pose dei sensori al veicolo. I coefficienti devono rappresentarne le proprietà fisiche.

Consulta [Correnti](currents.md), [Aggiungere sensori](custom-sensors.md) e i riferimenti dei plugin collegati. Per latitudine e longitudine, imposta l'origine con le [coordinate sferiche](coordinates.md).

Adatta `config/<namespace>/robot_config.py`: aggiorna insieme nomi di entità e giunti, topic, sistemi di riferimento e porte. Il pacchetto installa già `description`, `meshes`, `config` e le impostazioni dei percorsi delle risorse. Modifica i sorgenti, non i file generati in `install/`.

## 4. Compilare e provare

Dallo spazio di lavoro con l'ambiente caricato:

{{CODE:custom-robots:3}}

Prova ogni asse con comandi piccoli e verifica anche l'avvio da fuori della directory sorgente:

- Percorsi delle risorse installate.
- Massa, inerzia e posa di equilibrio.
- Geometria di collisione e risposta al contatto.
- Sistemi di riferimento e contenuto dei messaggi.

In un altro terminale preparato, leggi il modello rinominato:

{{CODE:custom-robots:4}}

Dovresti ricevere posizione e velocità. Fai funzionare la copia, poi modifica la meccanica e infine sensori o propulsione. Confronta le prove con [Dati dei sensori](visualization.md); per risorse mancanti, consulta la [risoluzione dei problemi](troubleshooting.md).
