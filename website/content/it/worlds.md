# Raccolta di mondi

I mondi si trovano in `models/posim_worlds/worlds`. Scegli il nome del file senza l'estensione `.world`.

## Avviare un mondo

Usa il [terminale POSIM della tua installazione](terminals.md): Ubuntu nativo, WSL, Docker o il desktop RDP.

{{CODE:worlds:0}}

Il nome del file può essere diverso da quello interno. `posim_ocean_waves.world`, per esempio, definisce `oceans_waves`, usato nei percorsi di topic e servizi Gazebo. Controlla l'elemento `<world name="…">` prima di comporre un percorso.

## Scegliere in base all'esperimento

| Esperimento | Mondo |
|---|---|
| Primo veicolo | `posim_ocean_waves` |
| Effetti della telecamera subacquea | `camera_tutorial` |
| Misure DVL | `dvl_world` |
| Interrogazione USBL | `usbl_tutorial` |
| Configurazione delle correnti | `ocean_current_plugin` |
| Scenario costiero e geografico | `posim_Santorini` |
| Sonar | `posim_multibeam_sonar` e mondi oceanici con sonar nel nome |
| Manipolazione | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

I mondi sonar richiedono il relativo [ambiente di esecuzione](sonar-tuning.md). Al primo avvio, i riferimenti Fuel possono scaricare modelli.

## File SDF

Apri un file per esaminare sistemi, modelli, origine, luci e telecamere.

| SDF del mondo |
|---|
{{WORLD_CATALOG}}

## Comporre un nuovo ambiente

Copia un SDF adatto in `models/posim_worlds/worlds`, assegnando nuovi nomi al file e al mondo. Usa URI che risolvano le risorse installate. Ricompila e carica l'ambiente, poi avvia il nuovo file. Vedi [Terreni da mappe di elevazione](heightmaps.md) e [Oggetti](objects.md).

## Attendere i download Fuel

`posim_integrated` e `posim_ocean_models` usano modelli remoti. Attendi il primo download senza avviare ripetutamente la simulazione durante il caricamento. `verbose:=true` mostra i messaggi delle risorse.

In un altro terminale dello stesso ambiente, richiedi la scena caricata:

{{CODE:worlds:1}}

Controlla i modelli attesi nella risposta, l'avanzamento dell'orologio e le misure dei sensori. Se il download si ferma, controlla il messaggio di caricamento e la rete del computer o contenitore che esegue Gazebo.

## Consultare gli argomenti di avvio

{{CODE:worlds:2}}

| Argomento | Funzione | Valore iniziale per l'avvio dei sensori |
|---|---|---|
| `namespace` | Cartella del modello installato | Vuoto; specificarlo |
| `world_name` | File del mondo | `empty.sdf` |
| `paused` | Avvio in pausa | `true` |
| `gui` / `headless` | Mostrare / nascondere l'interfaccia Gazebo | `true` / `false` |
| `use_sim_time` | Orologio simulato nei nodi ROS | `true` |
| `x`, `y`, `z` | Posizione iniziale, m | `0.0` ciascuno |
| `roll`, `pitch`, `yaw` | Orientamento iniziale, rad | `0.0` ciascuno |
| `debug` / `verbosity_level` | Registro dettagliato / livello | `false` / `1` |
| `use_ned_frame` | Sistema NED | `false` |

Usa `paused:=false` per ottenere dati dai sensori. Per robot e mondi, consulta `--show-args` del relativo file di avvio.
