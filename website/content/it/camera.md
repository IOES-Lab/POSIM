# Camera subacquea

`UnderwaterCamera` usa la profondità per attenuare i colori RGB con la distanza. Regola attenuazione e sfondo in base all'acqua da rappresentare. Per gli effetti condivisi dall'intera scena, consulta il [rendering subacqueo](underwater-rendering.md).

<a id="run-the-example"></a>
## Eseguire l'esempio

Il mondo della telecamera attiva piccole onde ed [effetti subacquei nativi](underwater-rendering.md). L'avvio del sensore imposta `POSIM_UNDERWATER_PLUGIN` per inoltrare i pixel RGB già elaborati all'uscita compatibile senza una seconda attenuazione. Per confrontare il filtro indipendente basato sulla profondità, usa un mondo senza il sistema di scena e lascia questa variabile non impostata.

Esegui l’esempio nel [terminale A preparato](terminals.md).

{{CODE:camera:0}}

Nel terminale B preparato:

{{CODE:camera:1}}

Aggiungi una visualizzazione Image in RViz. Se il topic compare ma le immagini non arrivano più, verifica il [trasporto delle immagini grandi](troubleshooting.md#image-topics-appear-but-frames-stall).

## Configurare il plugin

Inserisci questo sensore RGBD in un `<link>`. Il mondo deve caricare `gz-sim-sensors-system` con `<render_engine>ogre2</render_engine>`, come il [mondo della camera](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world). Il plugin legge colore/profondità da Gazebo e pubblica direttamente il topic ROS `/underwater_camera/simulated_image`.

{{CODE:camera:2}}

| Parametro SDF | Significato | Predefinito |
|---|---|---|
| `attenuationR` | Attenuazione rossa per metro, finita e ≥ 0 | `1/30` |
| `attenuationG` | Attenuazione verde per metro, finita e ≥ 0 | `1/30` |
| `attenuationB` | Attenuazione blu per metro, finita e ≥ 0 | `1/30` |
| `backgroundR` | Sfondo rosso, intero da 0 a 255 | `0` |
| `backgroundG` | Sfondo verde, intero da 0 a 255 | `0` |
| `backgroundB` | Sfondo blu, intero da 0 a 255 | `0` |

## Topic e geometria della camera

`<topic>` deve produrre un nome ROS valido. Se manca, viene usato il percorso Gazebo `/world/<world>/model/<model>/link/<link>/sensor/<sensor>`, con `/simulated_image` aggiunto per ROS. I modelli annidati includono i percorsi dei modelli superiori.

La conversione da profondità a distanza lungo il raggio usa prima la proiezione della lente specificata, poi i parametri intrinseci espliciti, infine dimensioni dell'immagine e campo visivo orizzontale. Le coordinate del punto principale possono essere zero. Le focali devono essere positive, i valori della lente finiti e i piani di clipping rispettare `0 < near < far`, in metri. Impostazioni non valide generano un errore e disattivano l'uscita.

## Sincronizzazione e valori di esempio

Colore e profondità vengono abbinati tramite il timestamp di acquisizione, indipendentemente dall'ordine di arrivo. ROS conserva timestamp e identificatore del sistema di riferimento dell'immagine a colori. Quando il buffer è pieno, i fotogrammi senza corrispondenza vengono scartati. Il reset del tempo simulato scarta i fotogrammi in attesa e permette ai timestamp di ripartire.

L'esempio richiede **320 × 240 a 10 Hz**, attenuazione RGB **0.8/0.5/0.2** e sfondo **85/107/47**. Il SDF sostituisce i valori predefiniti del codice.

## Regolare e confrontare

Posiziona un oggetto riconoscibile a distanze note. Mantieni fissi posa, luce, materiali e camera mentre modifichi un canale. Verifica anche timestamp e frequenza ricevuta.

L'attenuazione è esponenziale per canale, con miscelazione dello sfondo. Per un confronto senza attenuazione, imposta a zero **tutti e sei i parametri** della tabella. Omettere i tag ripristina i valori predefiniti.

Il [renderer di scena](underwater-rendering.md) applica effetti comuni a superfici PBR e camere. Con il sistema nativo attivo, `POSIM_UNDERWATER_PLUGIN` lascia passare i pixel già colorati per evitare un secondo filtraggio. Il [modello](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf) definisce collegamenti e topic.

## Particelle in sospensione

Il renderer può mostrare particelle nelle immagini a colori RGB/RGBD. Sono inizialmente disattivate: attivale tramite il [servizio per l'aspetto della scena](underwater-rendering.md#turn-on-marine-snow).

- Le camere condividono le stesse posizioni delle particelle nel mondo. Queste si spostano alla deriva in modo uniforme con il tempo simulato, indipendentemente dalle correnti.
- Le superfici opache le nascondono. Sono visibili tra **0.05 e 3 m**, entro i limiti della scena e del livello medio dell'acqua.
- Profondità numerica e raggi acustici conservano i propri valori.

Si tratta di effetti ottici procedurali. Per studiare la diffusione calibrata serve un modello ottico adatto. Nella navigazione visiva, considera le particelle osservazioni in movimento e mantieni le impostazioni costanti nei confronti.

## Confrontare le immagini subacquee

{{MEDIA:camera}}

Immagini: wiki Notion POSIM; attribuzione DAVE in [Citazioni e licenze](citation.md). Usa comandi e topic nei blocchi di questa pagina.
