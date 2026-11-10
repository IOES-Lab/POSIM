# Posizionamento USBL

Il posizionamento acustico a base ultracorta (USBL) localizza dispositivi sott'acqua. Questo esempio collega un ricetrasmettitore a due transponder. Puoi interrogarli tutti o sceglierne uno tramite ID.

## Avviare il mondo di esempio

Nel [terminale A preparato per la tua installazione](terminals.md):

{{CODE:usbl:0}}

Il mondo contiene già i tre dispositivi. Nel terminale B preparato, ricevi le posizioni:

{{CODE:usbl:1}}

Entrambi i messaggi includono `transponder_id`, ma i campi hanno significati diversi:

| Uscita | x | y | z |
|---|---|---|---|
| `transponder_location` | Angolo orizzontale, gradi | Distanza, m | Elevazione, gradi |
| `transponder_location_cartesian` | x relativa, m | y relativa, m | z relativa, m |

Si usa la differenza tra le posizioni nel mondo. L'angolo orizzontale vale 0° verso +x e 90° verso +y; l'elevazione positiva punta a +z. Considera questi assi quando combini dati nel riferimento del veicolo.

## Integrazione SDF

Inserisci il ricetrasmettitore nel modello `box`. Ogni ID corrisponde al modello nella stessa posizione di `transponder_attached_object`:

{{CODE:usbl:2}}

Inserisci questo transponder in `sphere` e un altro in `sphere2` con `transponder_ID` uguale a `2`. Nome, ID e namespace del ricetrasmettitore devono coincidere ai due estremi:

{{CODE:usbl:3}}

## Parametri

| Parametro | Dispositivo | Significato e predefinito |
|---|---|---|
| `namespace` | Entrambi | Namespace obbligatorio, ad esempio `USBL` |
| `transceiver_device`, `transceiver_ID` | Entrambi | Nome e ID obbligatori per i percorsi |
| `transponder_device`, `transponder_ID` | Entrambi | Nome e ID obbligatori; il ricetrasmettitore accetta ID separati da virgole |
| `transponder_attached_object` | Ricetrasmettitore | Modelli obbligatori separati da virgole, nello stesso ordine degli ID |
| `transceiver_model` | Transponder | Modello obbligatorio del ricetrasmettitore per la posizione |
| `sound_speed` | Entrambi | Velocità di riferimento a 10 °C e z = 0, m/s; `1540`, finita e positiva |
| `enable_ping_scheduler` | Ricetrasmettitore | Booleano obbligatorio; `false` usa ping manuali |
| `ping_frequency` | Ricetrasmettitore | Frequenza nel tempo simulato, Hz; `1`, esempio `0.5`; finita e positiva |
| `interrogation_mode` | Ricetrasmettitore | `common` o `individual`; predefinito `common` |
| `mu`, `sigma` | Transponder | Media e deviazione standard del rumore di posizione, m; `0` e `1`, esempio `0` e `0`; finite, sigma ≥ 0 |

Usa modelli esistenti, ID univoci e liste ID/modelli della stessa lunghezza. Gli ID sono interi decimali da **0 a 2147483647**, senza segno né zeri iniziali. Ogni plugin richiede un modello con un link. Il [SDF completo](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world) contiene geometria e posizionamento.

Una configurazione non valida lascia il plugin inattivo. Canali o destinatari di comando sconosciuti vengono ignorati; un canale sconosciuto mantiene la selezione. Il primo transponder è il canale iniziale. I ping programmati si fermano con la simulazione e vengono emessi al massimo una volta per aggiornamento.

## Tempi di propagazione

Ogni interrogazione comprende due tratte: arrivo della richiesta al transponder e ritorno della posizione al ricetrasmettitore. Il ritardo di ciascuna è distanza divisa per la velocità del suono del dispositivo interessato.

- I ritardi frazionari seguono il tempo simulato, si fermano con il mondo e vengono annullati quando il tempo torna indietro.
- Ogni tratta conserva un solo campione in attesa per transponder; richieste o risposte aggiuntive vengono ignorate.
- Per ricevere ogni risposta, scegli un intervallo tra ping superiore al tempo di andata e ritorno previsto.

La velocità effettiva è `sound_speed + 0.017 × depth + 4 × (temperature − 10)`. Le unità sono m/s, profondità in m sotto z = 0 e temperatura in °C. Sopra l'acqua la profondità è zero. La temperatura predefinita è 10 °C.

Pubblica `std_msgs/msg/Float64` su `/USBL/transceiver_manufacturer_168/temperature` o `/USBL/transponder_manufacturer_1/temperature`. Il cambio vale per le tratte successive. Temperature non finite o modifiche che producono velocità non positiva o non finita vengono ignorate.

L'approssimazione segue la [panoramica DOSITS sulla velocità del suono](https://dosits.org/decision-makers/tutorials/science/sound-speed/). Le distanze sono rettilinee, senza rifrazione, salinità, percorsi multipli o interferenze. Il ritorno usa la posizione ricevuta, incluso il rumore configurato.

## Interrogare tutti i transponder

{{CODE:usbl:4}}

`common` invia un broadcast per ping programmato e può restituire più ID sullo stesso topic. Conserva l'ID di ogni risposta. Il ricetrasmettitore mantiene il modo corrente per i transponder che si collegano dopo.

## Scegliere un transponder

{{CODE:usbl:5}}

Cambiare canale seleziona `individual`; ogni ping programmato è diretto a quell'ID. Torna a `common` per interrogare tutti i dispositivi.

## Disposizione e modalità di interrogazione

{{MEDIA:usbl}}

Immagini: wiki Notion POSIM; attribuzione DAVE in [Citazioni e licenze](citation.md). Usa i comandi e i topic di questa pagina.
