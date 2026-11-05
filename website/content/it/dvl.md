# Velocimetro Doppler (DVL)

Un DVL stima la velocità rispetto al fondale o all'acqua circostante. POSIM usa il sistema DVL di Gazebo e un bridge ROS che produce `posim_interfaces/msg/DVL`.

## Avviare il sensore

Nel [terminale A preparato per la tua installazione](terminals.md):

{{CODE:dvl:0}}

Nel terminale B preparato, leggi prima Gazebo, interrompi la lettura con **Ctrl+C**, poi leggi ROS:

{{CODE:dvl:1}}

Per un altro sensore, usa il topic definito nel suo modello.

## Integrazione SDF

Inserisci questi sistemi direttamente in `<world>`:

{{CODE:dvl:2}}

Il bridge POSIM va nel `<model>` del sensore. Il suo topic deve coincidere con il `<topic>` del DVL in `<link>`:

{{CODE:dvl:3}}

Il [SDF completo del DVL500-300](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf) definisce il sensore `type="custom" gz:type="dvl"`, quattro fasci e le modalità di rilevamento. Il [mondo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world) fornisce la geometria del fondale.

## Parametri e unità

`DVLBridge` accetta un solo parametro: `topic`, predefinito `/dvl/velocity`. Riceve da Gazebo e pubblica in ROS su questo percorso. Gli altri campi appartengono al sensore Gazebo; la tabella riporta l'esempio DVL500-300.

| Campo SDF del sensore | Significato | Esempio |
|---|---|---|
| `topic`, `update_rate` | Percorso e frequenza richiesta, Hz | `/dvl/velocity`, 8 |
| `gz:dvl/type` | Disposizione dei trasduttori | `phased_array` |
| `arrangement`: `aperture`, `rotation`, `tilt` | Angoli dei fasci; `degrees="true"` usa gradi | Apertura 3°, rotazioni −135/135/45/−45°, inclinazione 25° |
| `minimum_range`, `maximum_range` | Portata acustica, m | 0.3, 200 |
| `reference_frame` | Posizione in m, orientamento in rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | Selezione del riferimento di misura | Entrambi `best` |
| Tracking: `noise/stddev` | Deviazione standard del rumore di velocità, m/s | Fondale 0.005; acqua 0.015 |
| Massa d'acqua: `water_velocity` | Componenti x/y/z, m/s | 0/0/0 |
| Massa d'acqua: `boundaries/near`, `boundaries/far`, `bins` | Limiti in m e numero di celle | 10, 100, 10 |
| Tracking: `visualize` | Mostrare la geometria di campionamento | true |

L'[implementazione del bridge](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc) descrive la conversione. Il sensore di profondità usa il rendering Gazebo: completa la [preparazione dell'ambiente](requirements.md).

## Modelli disponibili

| Descrizione | Famiglia |
|---|---|
| `nortek_dvl500_300`, `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300`, `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000`, `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL con [sonar CUDA](sonar-tuning.md) |

Controlla campionamento e geometria nel SDF di ogni modello.

## Sistema di riferimento e covarianza

L'esempio converte ENU nel riferimento avanti/dritta/basso tramite `<reference_frame>`. Il bridge conserva il nome del riferimento Gazebo, sostituendo `::` con `/`. Usa lo stesso riferimento nello stimatore e distingui le velocità rispetto al fondale e all'acqua.

La velocità complessiva e ogni fascio includono una covarianza ROS del twist 6×6. Il blocco superiore sinistro 3×3 contiene la covarianza della velocità lineare, in m²/s². Le altre voci sono zero perché il DVL non stima la velocità angolare: non sono una misura con incertezza nulla.

## Esaminare le osservazioni

Consulta `ros2 interface show posim_interfaces/msg/DVL`. Verifica velocità, riferimento e stato a robot fermo, poi durante un movimento controllato. Fondale, portata e intersezioni dei fasci influenzano la misura. Abilita `visualize` nel modello per osservare i fasci.

## Fasci e uscita DVL

{{MEDIA:dvl}}

Immagini: wiki Notion POSIM; attribuzione DAVE in [Citazioni e licenze](citation.md). Usa i comandi e i topic di questa pagina.
