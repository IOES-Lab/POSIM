# Oggetti e scenari operativi

POSIM include modelli per manipolazione, ispezione e interazione. `posim_object_models` installa descrizioni, mesh e configurazioni. Esegui i comandi nel [terminale POSIM della tua installazione](terminals.md), anche con WSL, Docker o RDP.

## Catalogo locale

Le directory seguenti provengono da `models/posim_object_models/description`. Aprile per esaminare SDF, link, giunti e risorse. I mondi di esempio possono caricare anche oggetti da Gazebo Fuel.

{{OBJECT_CATALOG}}

## Partire da una scena pronta

{{CODE:objects:0}}

Altri scenari sono `posim_electrical_mating` e `posim_bimanual_example`. Usano risorse Fuel come un pannello di controllo del blowout preventer, connettori maschio e femmina e vasi sommersi. Controlla URI e posizioni nell'SDF prima di collocare il robot o studiare contatti e giunti.

## Inserire un oggetto in un mondo

Sostituisci l'URI con il nome effettivo della risorsa scelta:

{{CODE:objects:1}}

Le posizioni sono in metri e gli angoli in radianti. Assegna un nome diverso a ogni istanza. Scegli il comportamento `static` in base all'uso: un bersaglio fisso di ispezione e un oggetto manipolabile richiedono dinamiche diverse.

La geometria visiva definisce l'aspetto; quella di collisione determina il contatto. Controlla scala, posa, massa, inerzia, limiti dei giunti e attrito. Inizia con un'interazione piccola e controllata, poi registra gli stati con [ROS 2](ros.md).

## Avviare un singolo oggetto

Con l'ambiente caricato, avvia `mossy_cinder_block`:

{{CODE:objects:2}}

Questo avvio crea un mondo proprio. Per combinare gli esempi, componi un solo mondo con elementi `<include>`. Vedi [Aggiungere un robot](custom-robots.md) e [Raccolta di mondi](worlds.md).

## Usare Fuel in locale

Scarica ed estrai un modello dal [catalogo Fuel](https://app.gazebosim.org/fuel/models). Tieni insieme `model.sdf`, `model.config`, `meshes/` e `materials/`, per esempio in `~/posim_models/my_object/`. Registra la directory superiore nel terminale che avvierà Gazebo:

{{CODE:objects:3}}

Apri **Resource Spawner** in Gazebo per posizionare il modello locale. Riutilizzalo in un SDF con `<uri>model://my_object</uri>`. Funziona anche l'URI HTTPS del frammento SDF fornito da Fuel, dopo il primo download. Conserva file e licenze per gli esperimenti offline.

## Pubblicare su Fuel

Accedi a [Gazebo Fuel](https://app.gazebosim.org/home) e crea un modello. Inserisci nome, descrizione e licenza; carica `model.sdf`, `model.config` e le mesh e i materiali referenziati. Scarica il risultato e verifica che non dipenda da percorsi locali esterni. Aggiungilo a una raccolta se necessario.

Scegli librerie e classi di plugin disponibili in Jetty. [Aggiungere un robot](custom-robots.md) spiega come collegare galleggiamento, idrodinamica, propulsione e sensori.

{{MEDIA:objects}}

Immagini: wiki Notion POSIM. [Citazioni e licenze](citation.md) riporta l'attribuzione della documentazione DAVE. Per comandi e nomi dei topic, usa i blocchi di questa pagina.
