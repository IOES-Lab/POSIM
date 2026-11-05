# Pressione idrostatica

Il plugin calcola la pressione dalla profondità del modello e pubblica messaggi ROS `FluidPressure`. Può anche pubblicare una profondità stimata a partire dalla stessa pressione.

## Osservare REXROV

Avvia il robot nel [terminale A preparato per la tua installazione](terminals.md):

{{CODE:pressure:0}}

Nel terminale B preparato:

{{CODE:pressure:1}}

La pressione è espressa in **Pa**, la varianza in **Pa²** e la profondità in **m**.

## Integrare il plugin

Inserisci il blocco direttamente nel `<model>` da misurare. `namespace` identifica i suoi topic:

{{CODE:pressure:2}}

L'uscita è `/model/<namespace>/<topic>`; quella della profondità aggiunge `_depth`. Il plugin misura il modello a cui è associato. Il namespace può differire dal nome del modello.

## Parametri

| Parametro | Significato e valore predefinito |
|---|---|
| `namespace` | Obbligatorio; prefisso valido di topic ROS |
| `topic` | Nome dell'uscita; `sea_pressure` |
| `standard_pressure` | Pressione superficiale finita e non negativa, kPa; `101.325` |
| `kPa_per_meter` | Aumento idrostatico finito e positivo, kPa/m; `9.80638` |
| `estimate_depth_on` | Pubblicare la profondità calcolata; `true` |
| `update_rate` | Frequenza finita, Hz; `0`. Un valore non positivo pubblica a ogni avanzamento fisico |
| `saturation` | Valore accettato, `3000`; l'implementazione non lo usa per limitare la pressione |

La profondità è `max(0, -z)` nelle coordinate del mondo, incluse le trasformazioni superiori dei modelli annidati. Il riferimento è il piano z = 0, indipendente dalle onde. Il calcolo usa kPa e la pubblicazione converte in Pa.

## Tempo e validità

Pressione ROS, profondità e pressione Gazebo condividono lo stesso timestamp simulato. La pausa o un timestamp ripetuto non producono nuove misure; la pubblicazione riprende quando il tempo avanza o torna indietro.

Una configurazione non valida lascia inattivo il plugin. Pose mancanti o non finite e calcoli in overflow non producono campioni.

La varianza pubblicata è fissa: **9 000 000 Pa²**. Non viene aggiunto rumore casuale alla pressione. Tienine conto nell'estimatore. Il [codice del plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc) mostra i calcoli.

## Confrontare due profondità

Misura due posizioni z note senza cambiare la pendenza della pressione. La differenza attesa è l'aumento per metro moltiplicato per la variazione di profondità. La profondità derivata usa lo stesso calcolo e non è una misura indipendente. Imposta coerentemente unità e pressione di riferimento dell'estimatore.

## Uscite di pressione e profondità

{{MEDIA:pressure}}

Immagini: wiki Notion POSIM; attribuzione DAVE in [Citazioni e licenze](citation.md). Usa comandi e nomi dei topic presenti nei blocchi di questa pagina.
