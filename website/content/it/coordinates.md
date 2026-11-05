# Coordinate sferiche

Il plugin ROS `SphericalCoords` gestisce l'origine geografica del mondo e converte le posizioni cartesiane locali in latitudine, longitudine e altitudine, e viceversa.

## Avviare un mondo configurato

Nel [terminale A preparato per la tua installazione](terminals.md):

{{CODE:coordinates:0}}

Nel terminale B preparato:

{{CODE:coordinates:1}}

Il SDF di esempio imposta latitudine **35.074823°** e longitudine **129.084798°**. Leggi la risposta per conoscere l'origine del mondo in esecuzione.

## Integrazione SDF e parametri

Inserisci entrambi i blocchi direttamente in `<world>`:

{{CODE:coordinates:2}}

Il plugin legge le coordinate del mondo e non aggiunge parametri SDF propri. I quattro percorsi dei servizi sono fissi sotto `/gz/`. Fuori da un mondo, oppure senza configurazione geografica, segnala un errore e lascia disattivati i servizi.

| Campo del mondo | Significato | Esempio |
|---|---|---|
| `surface_model` | Ellissoide di riferimento | `EARTH_WGS84` |
| `world_frame_orientation` | Convenzione degli assi | `ENU` |
| `latitude_deg`, `longitude_deg` | Origine, gradi | `35.074823`, `129.084798` |
| `elevation` | Altitudine dell'origine, m | `0` |
| `heading_deg` | Scostamento di rotta, gradi | `0` |

Definisci un'origine esplicita. Vedi il [plugin](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc) e il [mondo di esempio](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world).

## Servizi

I tipi appartengono a `posim_interfaces/srv`. Gli angoli sono in gradi; coordinate locali e altitudine in metri.

| Suffisso del servizio sotto `/gz/` | Tipo | Richiesta |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | Vuota |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

Tutti i valori devono essere finiti. La latitudine va da −90° a 90°, la longitudine da −180° a 180°. Un aggiornamento dell'origine non valido restituisce `success: false` e conserva l'origine. Una conversione fallita restituisce coordinate `NaN`: controllale prima di usarle in un controllore.

## Convertire una posizione locale

{{CODE:coordinates:3}}

La risposta contiene latitudine, longitudine e altitudine. Passale al servizio inverso e confronta `output` con il vettore iniziale per verificare origine e convenzione degli assi.

## Cambiare l'origine

{{CODE:coordinates:4}}

Il cambio modifica la corrispondenza tra posizioni locali e geografiche. Mantieni coerenti terreno, orientamento del mondo, riferimento altimetrico e controllori. Le conversioni usano Gazebo: considera rotta e orientamento impostati nel SDF.
