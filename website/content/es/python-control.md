# Control con Python

Inicia el [ejemplo REXROV](rovs.md#start-rexrov) en la terminal A y mantenlo activo. En la [terminal B preparada](terminals.md), lee su posición y aplica un impulso breve a un propulsor.

## Suscribirse a la posición

Guarda este programa como `observe.py` y ejecuta `python3 observe.py` en el entorno ROS preparado.

{{CODE:python-control:0}}

Verás una posición aproximadamente cada segundo. Ctrl+C detiene el observador y deja la simulación activa.

## Enviar una orden limitada

Guarda el siguiente programa como `pulse_thruster.py` y ejecuta `python3 pulse_thruster.py`. Espera al suscriptor, aplica **5 N al propulsor 1** durante unos dos segundos del equipo anfitrión y envía cero al terminar. El eje de montaje determina la fuerza y el par resultantes.

{{CODE:python-control:1}}

La configuración de señales permite ejecutar la limpieza al pulsar Ctrl+C mientras el publicador aún está disponible. Si fuerzas la terminación del proceso, detén la simulación o envía órdenes neutras según [Control de propulsores](robot-control.md).

## Observar la respuesta

En otra terminal preparada, observa la velocidad angular durante el impulso:

{{CODE:python-control:2}}

La velocidad debe responder y volver hacia cero. Compara la odometría para distinguir el movimiento del actuador del movimiento del vehículo. Ctrl+C cierra el visor.

## Crear un controlador

Distribuye las fuerzas entre los ocho propulsores de REXROV usando el estado medido y tu ley de control. Respeta los tópicos, ejes y unidades. Usa tiempo simulado para la lógica del experimento y devuelve a neutro todos los actuadores utilizados al terminar.

BlueROV2 sigue la ruta [MAVROS/ArduSub](robot-control.md). Para comparar ensayos, [graba las observaciones](visualization.md).
