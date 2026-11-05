# Contribuire a POSIM

Puoi contribuire al codice e alla documentazione. Parti da una modifica mirata e riproducibile, mantenendo coerenti modelli, controllori e guide.

## Lavorare su un branch

{{CODE:contributing:0}}

Per modificare la simulazione, usa l'[ambiente di installazione](requirements.md) adatto. Non includere nel commit cartelle generate di compilazione o installazione, né credenziali locali.

## Verificare il comportamento modificato

Scegli un esempio minimo pertinente. Controlla risoluzione delle risorse, inserimento del modello, avanzamento del tempo, dati ricevuti, controllore quando necessario e chiusura regolare. Registra revisione, renderer, architettura e comando.

Le modifiche a sensori o dinamica richiedono un confronto numerico adeguato. Una schermata mostra l'aspetto, ma non dimostra l'accuratezza fisica. Conserva l'attribuzione originale quando modifichi codice ereditato.

## Aggiornare tutte le otto lingue

Il sito supporta inglese, coreano, spagnolo, francese, tedesco, italiano, giapponese e cinese. L'inglese usa `/`; le altre lingue usano `/ko/`, `/es/`, `/fr/`, `/de/`, `/it/`, `/ja/` e `/zh/`.

Aggiorna i file corrispondenti in `website/content/<lingua>`. Mantieni coerenti comandi, unità, prerequisiti e limiti, usando frasi naturali e termini consolidati della robotica. I segnaposto condivisi `CODE` e `MEDIA` riutilizzano esempi eseguibili e immagini. Ogni lingua deve avere tutte le pagine; un testo mancante non viene sostituito con l'inglese.

Quando cambia una funzione pubblica:

- Aggiorna il [catalogo delle funzioni](features.md) e la guida d'uso insieme al codice.
- Descrivi installazione e attivazione, valori predefiniti, unità, ingressi e uscite, ambito della configurazione, un esempio osservabile e limiti del modello. Includi effetti facoltativi e adattatori.
- Registra le nuove pagine in `website/scripts/pages.mjs` e i titoli in `website/i18n/<lingua>.json`. Aggiorna anche i README nelle varie lingue.

L'ordine delle pagine è comune; i cataloghi di mondi e oggetti vengono generati dai sorgenti. Descrivi il prodotto attuale. Conserva note interne, piani e rapporti di validazione fuori dal repository; usa issue e pull request per discutere modifiche e risultati.

## Controllare il sito

Dalla cartella `website`, con Node.js 24 e pnpm 10.11.0:

{{CODE:contributing:1}}

Apri `http://127.0.0.1:4174`. Verifica collegamenti, copia dei comandi, ricerca, cambio lingua e navigazione mobile. Pubblicazione e manutenzione sono descritte in [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md).

## Aprire una pull request

Spiega il problema, il comportamento risultante e le verifiche eseguite. Per modifiche alla simulazione, includi comandi e log pertinenti; per l'impaginazione, viste desktop e mobile. Distingui gli aspetti verificati da quelli ancora da provare.

Usa le [issue GitHub](https://github.com/IOES-Lab/POSIM/issues) per problemi riproducibili e le [pull request](https://github.com/IOES-Lab/POSIM/pulls) per la revisione. IOES-Lab mantiene il progetto presso la [Korea Maritime & Ocean University](https://lab.wschoi.com).
