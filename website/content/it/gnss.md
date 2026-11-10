# GNSS marino

`posim_gz_gnss` simula un ricevitore satellitare sul link dell'antenna del modello. Converte la sua posizione nel mondo in coordinate geografiche, aggiunge errori gaussiani configurabili e pubblica posizione, velocità, timestamp e stato in un unico messaggio. Comprende perdita di ricezione sott'acqua, blocco esplicito, riacquisizione e latenza.

## Compilare e montare il ricevitore

Nel [terminale A preparato per la tua installazione](terminals.md); su macOS, usa il terminale Linux del desktop RDP:

{{CODE:gnss:0}}

Il modello deve avere un link fisico per l'antenna, normalmente `gnss_link`, posizionato e collegato al veicolo. Il mondo richiede [coordinate WGS 84](coordinates.md). Inserisci nel SDF del modello:

{{CODE:gnss:1}}

Adatta link e topic al robot. La guida [Creare un robot](custom-robots.md) spiega come preparare e avviare il modello.

I [profili](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json) indicano pose rispetto a `base_link`: WAM-V a z = 1.8 m, BlueROV2 a 0.3 m e REXROV a 0.8 m. WAM-V abilita il ricevitore; i profili subacquei lo disabilitano inizialmente. I profili contengono dati di montaggio: link e plugin vanno inseriti nel modello. WAM-V richiede anche il [modello esterno Wave Sim](surface.md).

## Parametri

| Parametro SDF | Unità o significato | Predefinito |
|---|---|---|
| `update_rate` | Campioni per secondo simulato; >0 fino a 100 Hz | 5 |
| `latency` | Ritardo di consegna in secondi simulati; 0–10 s | 0.2 |
| `reacquire_seconds` | Tempo continuo di acquisizione fuori dall'acqua, s | 3 |
| `dry_margin` | Distanza sopra l'acqua necessaria per acquisire, m | 0.05 |
| `horizontal_stddev` | Deviazione standard della posizione orizzontale, m | 0.7 |
| `vertical_stddev` | Deviazione standard della posizione verticale, m | 1.2 |
| `velocity_stddev` | Deviazione standard della velocità, m/s | 0.1 |
| `water_level` | z medio dell'acqua senza topic delle onde, m | 0 |
| `seed` | Seme casuale del ricevitore | 42 |
| `enabled` | Ricevitore abilitato | true |
| `link_name` | Link dell'antenna nel modello | `gnss_link` |
| `topic` | Topic Gazebo delle misure | `/model/<model-name>/gnss` |
| `wave_topic` | Topic facoltativo dell'altezza dell'acqua | Vuoto |

I valori numerici devono essere finiti. Latenza, tempo di acquisizione, margine e deviazioni standard devono essere non negativi.

## Altezza locale dell'acqua

Senza `wave_topic`, si usa il piano medio dell'acqua. Con un topic configurato, servono campioni locali recenti; dati mancanti o non validi producono `surface unavailable`.

Il messaggio `gz.msgs.Double_V` contiene, nell'ordine: schema `1`, dimensione della griglia, dimensioni periodiche X/Y, velocità e angolo del vento, ripidezza dell'onda, tempo simulato e altezze per righe.

- Dimensione della griglia: intero da 2 a 256; il numero delle altezze è il suo quadrato.
- Dimensioni X/Y: finite e positive.
- Timestamp: finito, non più vecchio di **0.6 secondi simulati**, mai futuro.

Configura separatamente un publisher compatibile. I metadati del vento non controllano la ricezione.

## Leggere misure e blocco

Dopo aver avviato il modello configurato, leggi il topic nel terminale B preparato:

{{CODE:gnss:2}}

Ogni messaggio contiene 15 numeri in doppia precisione. Gli indici partono da zero. Conserva il timestamp di acquisizione anche quando la posizione arriva in ritardo.

| Indice | Campo e unità |
|---|---|
| 0–3 | Schema `1`, epoca di reset, acquisizione in secondi simulati, stato |
| 4–6 | Latitudine in gradi, longitudine in gradi, altitudine in m |
| 7–9 | Velocità verso est, nord e alto, m/s |
| 10–12 | Varianze di posizione orizzontale/verticale in m² e velocità in (m/s)² |
| 13–14 | z dell'antenna nel mondo e altezza locale dell'acqua, m |

Stati: `0 disabled` (disabilitato), `1 submerged` (sommerso), `2 acquiring` (acquisizione), `3 fix` (posizione valida), `4 blocked` (bloccato), `5 surface unavailable` (superficie non disponibile).

Una misura non valida contiene coordinate e velocità `NaN`. Contatto con l'acqua o blocco eliminano le misure in attesa. La riacquisizione richiede distanza continua sopra il margine. Il reset cancella timer e campioni ritardati e incrementa l'epoca.

Per bloccare l'antenna; usa `false` per liberarla:

{{CODE:gnss:3}}

## Interpretare e verificare

Confronta un'antenna ferma fuori dall'acqua, sommersa, bloccata e ripristinata. Controlla acquisizione, ritardo, epoca dopo il reset e timestamp crescenti. Scarta posizioni obsolete o future e gestisci gli stati non validi. L'uscita usa Gazebo; per ROS `NavSatFix`, autopilota o stimatore di navigazione serve un adattatore al formato atteso.

Il modello si basa sulla posa dell'antenna. Non calcola geometria satellitare, propagazione RF, percorsi multipli fisici o posizionamento acustico. Le precisioni impostate sono dati della simulazione, non garanzie del produttore. Vedi [implementazione](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc) e [guida del pacchetto](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss).
