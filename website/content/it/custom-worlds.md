# Creare un mondo

Parti da un mondo piccolo con un sensore di pressione a profondità nota. Potrai modificare l'ambiente e misurare il risultato prima di aggiungere terreno e robot mobile.

## Preparare l'ambiente di lavoro

Usa i [terminali Linux preparati](terminals.md) su Ubuntu nativo, WSL o Docker. Su macOS apri il terminale del desktop RDP. Modifica e compila in quell'ambiente; `POSIM_WS` indica il suo spazio di lavoro.

## Salvare il mondo completo

Salva il contenuto seguente come `calm_water.sdf`. Usa geometrie semplici e plugin installati, senza mesh da scaricare.

{{CODE:custom-worlds:0}}

Il mondo comprende:

- **Fisica:** passo massimo di 0,001 s, fattore di tempo reale desiderato pari a 1 e gravità verso il basso.
- **Luci e scena:** sole direzionale, luce ambientale e colore di sfondo.
- **Fondale:** parallelepipedo fisso a z = −20 m, con geometrie visiva e di collisione coincidenti.
- **Stazione di pressione:** indicatore fisso a z = −2 m. Il plugin di [pressione](pressure.md), inserito nel modello, pubblica pressione e profondità stimata.

Il riferimento della pressione è il piano z = 0 del mondo. Aggiungi onde, galleggiamento e movimento dopo aver verificato questo caso di base.

## Validare e avviare

Nel terminale A, dalla directory del file:

{{CODE:custom-worlds:1}}

La validazione deve riuscire, poi si apre Gazebo. `-r` avvia il tempo simulato. Seleziona `pressure_station` nell'albero delle entità e centra la vista sul modello.

In un altro terminale con lo stesso ambiente caricato:

{{CODE:custom-worlds:2}}

I valori attesi sono circa **120 938 Pa** e **2 m**. Il plugin pubblica direttamente in ROS; non serve un bridge aggiuntivo.

## Cambiare la profondità

1. Ferma il terminale A con Ctrl+C.
2. Cambia la posa della stazione da `0 0 -2 0 0 0` a `0 0 -4 0 0 0`.
3. Valida e riavvia il mondo.
4. Ripeti le due letture: dovresti ottenere circa **140 551 Pa** e **4 m**.

Mantieni invariati i coefficienti di pressione. L'aumento atteso è **19 612,76 Pa**. Conserva entrambi i file insieme alle osservazioni.

## Individuare le impostazioni

| Percorso SDF | Unità | Funzione |
|---|---|---|
| `world/physics/max_step_size` | s | Passo massimo: `0.001` |
| `world/physics/real_time_factor` | — | Rapporto desiderato fra tempo simulato e reale: `1.0` |
| `world/gravity` | m/s² | Vettore di gravità |
| `world/scene`, `world/light` | RGBA / direzione | Sfondo e illuminazione |
| `model/pose` | m, rad | Posizione e orientamento |
| `model/static` | booleano | Mantenere fissi stazione e fondale |
| `model/plugin` | secondo il plugin | Parametri della [pressione](pressure.md) |

La velocità effettiva dipende dal carico. La definizione del sensore e il relativo sistema del mondo hanno ambiti diversi. [Aggiungere sensori](custom-sensors.md) integra un'IMU e il suo bridge ROS in questo file.

## Ampliare il mondo

- [Terreni](heightmaps.md): sostituire il fondale e verificare le collisioni.
- [Aggiungere un robot](custom-robots.md): meccanica, propulsione e ROS.
- [Correnti](currents.md): flusso del mondo e risposta del modello.
- [Coordinate sferiche](coordinates.md): origine geografica.
- [Effetti subacquei](underwater-rendering.md): rendering.

Salva i mondi distribuibili in `models/posim_worlds/worlds` e le risorse nello stesso pacchetto. Ricompila, carica l'ambiente e usa URI delle risorse installate quando condividi il mondo.
