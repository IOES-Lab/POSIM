# Effetti della scena subacquea

`posim_gz_rendering` applica velatura lungo il percorso nell'acqua, assorbimento RGB, luce solare dipendente dalla profondità, caustiche e particelle in sospensione alle superfici PBR di Ogre2. Le particelle rappresentano la neve marina e sono inizialmente **disattivate**.

Scena Gazebo, camere RGB e colore RGBD condividono questi effetti. Profondità numerica e geometria di collisione restano invariate. Il filtro indipendente basato sulla profondità è descritto in [Camera subacquea](camera.md).

Il sistema installato trova automaticamente la libreria di rendering adiacente e gli shader in `share/posim/underwater`. `renderer_library` e `shader_directory` permettono di specificare percorsi alternativi. Gli avvii oceanici attivano automaticamente il passaggio diretto dei pixel; impostalo negli avvii personalizzati che usano entrambi i sistemi.

## Compilare e integrare il sistema

Prepara un ambiente di sviluppo Gazebo Jetty/Ogre2 compatibile con il server. Nel [terminale A preparato](terminals.md), compila dai sorgenti POSIM; su macOS usa il terminale Linux del desktop RDP:

{{CODE:underwater-rendering:0}}

CMake richiede i pacchetti di sviluppo Gazebo e le voci pkg-config `OGRE-Next` e `OGRE-Next-Hlms`. Il comando include il percorso del pacchetto ROS `gz_ogre_next_vendor` su Ubuntu. Usa librerie, header e shader della stessa compilazione.

Inserisci questo plugin in `<world>` e sostituisci `/absolute/underwater` con il prefisso di installazione precedente. Carica una sola istanza per processo Gazebo e assegna un `session_nonce` diverso a ogni sessione:

{{CODE:underwater-rendering:1}}

Linux usa `.so`, anche in Docker/RDP su macOS. Le librerie compilate direttamente per macOS usano `.dylib`. Per produrre immagini serve una camera attiva nella scena.

Se usi anche `UnderwaterCamera`, imposta `POSIM_UNDERWATER_PLUGIN` nell'ambiente del server sul percorso del sistema caricato. I pixel già colorati passano senza una seconda attenuazione. Impostalo solo quando il sistema di scena è installato e caricato.

## Effetti e valori predefiniti

| Impostazione | Effetto | Predefinito |
|---|---|---|
| Ottica dell'acqua | Interruttore generale degli effetti ottici | Attiva |
| Velatura subacquea | Miscelazione del colore secondo il percorso nell'acqua | Attiva |
| Assorbimento RGB | Perdita di colore secondo la lunghezza d'onda | Attivo |
| Attenuazione solare | Luce direzionale più debole in profondità | Attiva |
| Caustiche | Motivi luminosi animati sulle superfici sommerse | Attive |
| Particelle / neve marina | Punti luminosi procedurali sparsi | Disattivate |
| Texture del fondale | Dettagli delle mappe PBR del fondale | Attiva |
| Griglia del fondale | Passo 10 m, regolabile da 1 a 500 m | Disattiva |
| Isobate | Passo 5 m, regolabile da 1 a 50 m | Disattive |

Disattivare l'ottica conserva le scelte di texture, griglia e isobate. Le sovrapposizioni riconoscono visual chiamati `seabed` o `shallow`, inclusi i nomi completi Gazebo. Fornisci le texture negli asset del terreno; vedi [Terreno da mappa di altezza](heightmaps.md).

<a id="turn-on-marine-snow"></a>
## Attivare la neve marina

A mondo avviato, leggi lo stato nel terminale B preparato:

{{CODE:underwater-rendering:2}}

I primi 13 valori seguono questo ordine; gli indici partono da zero:

| Indice | Valore |
|---|---|
| 0–1 | Schema `1`, revisione |
| 2–5 | Ottica, texture del fondale, griglia, isobate |
| 6–10 | Velatura, assorbimento, attenuazione solare, caustiche, particelle |
| 11–12 | Passo di griglia e isobate, m |

Su un sistema appena avviato con revisione `0`, questa richiesta attiva le particelle e conserva gli altri valori predefiniti:

{{CODE:underwater-rendering:3}}

Per le modifiche successive mantieni le impostazioni desiderate e aumenta la revisione di uno. I booleani accettano `0` o `1`. Puoi ripetere una richiesta identica.

Dopo il rendering, rileggi `/state`: aggiunge `ready`, numero di camere e `renderer_failed` agli indici 13–15. Verifica revisione richiesta, `ready = 1` e `renderer_failed = 0`. La risposta di `/set` conferma soltanto la ricezione.

Per attivare le particelle all'avvio, aggiungi questo tag nel plugin al posto della prima richiesta:

{{CODE:underwater-rendering:4}}

WWW-POSIM usa questo renderer pubblico e aggiunge un proprio pannello per regolare l'aspetto della scena, la gestione delle sessioni e le preferenze salvate.

## Limiti del modello e confronto

Il calcolo della lunghezza del percorso ottico nell'acqua usa il piano del livello medio del mare a **z = 0 m**, senza seguire le singole creste. Caustiche e particelle sono approssimazioni visive: non modellano concentrazioni calibrate né rumore ottico del sensore.

Le lampade locali mantengono l'illuminazione diretta. I raggi del cielo subiscono attenuazione subacquea; materiali non illuminati e shader propri della superficie marina conservano il loro rendering. Il lato inferiore delle onde dipende quindi dal materiale di superficie.

Confronta viste sopra/sott'acqua, RGB e colore RGBD, e ogni interruttore. Mantieni fissi posa e luce. Verifica profondità identica byte per byte, pausa degli effetti con il tempo simulato e segnalazione dei guasti. Il [pacchetto pubblico](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering) include esempi di verifica RGB/RGBD, shader e dettagli di compilazione.
