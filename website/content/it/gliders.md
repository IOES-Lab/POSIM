# Glider Slocum

Il modello `glider_slocum` comprende corpo, propulsore, idrodinamica e interfacce IMU, navigazione e pressione. Usalo per esperimenti con glider o come punto di partenza per un altro veicolo.

## Avviare nell'oceano

Nel [terminale POSIM preparato per la tua installazione](terminals.md):

{{CODE:gliders:0}}

Per esaminare il modello in un mondo vuoto, usa `world_name:=empty.sdf z:=0.2`.

## Leggere le osservazioni

{{CODE:gliders:1}}

I topic Gazebo IMU e NavSat si trovano sotto `/model/glider_slocum/`. Consulta la configurazione dei bridge per scegliere i corrispondenti topic e tipi ROS. Il modello carica anche il [plugin di pressione](pressure.md) e il publisher di odometria di Gazebo.

Dopo aver avviato veicolo e ambiente, collega il tuo nodo di controllo per missioni di immersione o planata.

## Adattare il modello

Leggi il [file SDF di Slocum](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) insieme alla configurazione dei bridge. Modifica in modo coerente massa, inerzia, geometria di collisione e coefficienti idrodinamici. Verifica le convenzioni dei sistemi di riferimento prima di collegare uno stimatore o un controllore.

Conserva il modello originale per confrontare i risultati. [Aggiungere un robot](custom-robots.md) spiega come creare una nuova descrizione; [ROS](ros.md), come osservarla e controllarla.

## Vedere il glider in movimento

{{MEDIA:gliders}}

Immagini e video: POSIM Notion Wiki. L'attribuzione della documentazione DAVE è in [Citazioni e licenze](citation.md). Usa i comandi e i nomi dei topic presenti in questa pagina.
