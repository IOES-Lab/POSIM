# Terreni da mappe di elevazione

Una mappa di elevazione descrive il terreno come una griglia regolare di quote. Configura insieme geometrie visiva e di collisione, affinché il fondale osservato coincida con la superficie di contatto.

## Ambiente di lavoro

Usa i [terminali Linux preparati](terminals.md) su Ubuntu nativo, WSL o Docker. Su macOS lavora nel desktop RDP. Modifica e compila lì; `POSIM_WS` indica il relativo spazio di lavoro.

## Consultare gli esempi

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) mostra uno scenario geografico con il modello Fuel `Santorini Scaled`. Il [fondale a livelli](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world) offre un altro esempio di composizione.

Per creare la geometria, segui il [tutorial Gazebo](https://gazebosim.org/api/sim/10/heightmap_dem.html) e il [riferimento SDFormat](http://sdformat.org/spec?ver=1.12&elem=geometry).

## Conservare le risorse nel pacchetto

Metti il mondo in `models/posim_worlds/worlds` e i dati in `models/posim_worlds/media`. Controlla le regole d'installazione CMake e i percorsi delle risorse del pacchetto. Usa URI portabili delle risorse installate.

Dentro una geometria visiva o di collisione, la struttura generale è:

{{CODE:heightmaps:0}}

Sostituisci l'URI con la tua risorsa. Scegli formato raster e campionamento supportati da renderer e motore fisico. Verifica la codifica delle quote prima di impostare dimensione verticale e spostamento.

## Preservare scala e coordinate

`size` esprime estensione orizzontale e scala verticale in metri; `pos` aggiunge uno spostamento. Un'immagine normalizzata non è automaticamente georeferenziata. Registra estensione originale, intervallo delle quote, riferimento del livello del mare ed eventuali riproiezioni o ricampionamenti.

Se servono posizioni geografiche, imposta `<spherical_coordinates>` nel mondo. Le [coordinate sferiche](coordinates.md) descrivono la conversione fra latitudine/longitudine e coordinate locali.

## Compilare e verificare

Ricompila e carica l'ambiente dopo aver aggiunto risorse. Avvia il mondo da fuori del repository e controlla:

- Rilievi riconoscibili e allineamento visivo/collisione.
- Profondità misurata dai sensori.
- Contatto tramite una prova controllata di caduta.
- Livello del mare e altezza iniziale del robot.

In Jetty, un `<heightmap><pos>` diverso da zero può spostare il terreno renderizzato senza spostare allo stesso modo la collisione. Anche trasferire lo spostamento alla posa del modello può lasciare disallineamenti. Verifica entrambe le soluzioni con renderer e motore fisico in uso. Se la differenza rimane, scegli una geometria o un motore che superi le prove di contatto e visualizzazione prima degli esperimenti di navigazione.
