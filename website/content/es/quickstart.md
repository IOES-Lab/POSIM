# Primera simulación

Inicia un REXROV en el océano, recibe sus mediciones y cierra la simulación. Para esta primera prueba, conserva el robot y el mundo indicados.

## Preparar las terminales

Completa la [instalación](requirements.md) y abre [dos terminales con ROS y POSIM cargados](terminals.md). La terminal A ejecuta el simulador; la B consulta sus datos. Con Docker, Gazebo aparece en el escritorio Linux, WSLg o RDP que configuraste.

## 1. Iniciar el robot y el mundo

En la **terminal A**:

{{CODE:quickstart:0}}

REXROV aparece cinco metros por debajo de z = 0 y se activan sus puentes ROS. Este comando también inicia el mundo: ejecútalo como una sola simulación y deja A abierta.

## 2. Leer la posición

En la **terminal B**:

{{CODE:quickstart:1}}

Deben llegar muestras continuamente. Una posición casi constante puede ser normal mientras el robot se estabiliza. Pulsa **Ctrl+C en B** para cerrar el visor.

## 3. Comprobar el tiempo simulado

{{CODE:quickstart:2}}

El reloj debe avanzar. Pulsa Ctrl+C para dejar de observarlo. Si no hay mensajes o el tiempo está detenido, consulta [Solución de problemas](troubleshooting.md).

## 4. Terminar

Pulsa **Ctrl+C en A** y espera a que finalicen los procesos. Cerrar el visor de tópicos no detiene el simulador. Detén esta simulación antes de iniciar otra.

## Continuar

- [Ver y grabar sensores](visualization.md): visualizar imágenes y guardar observaciones.
- [Control con Python](python-control.md): leer el estado y accionar un propulsor.
- [ROV y BlueROV2](rovs.md): ArduSub, teclado y joystick.
- [Mundos disponibles](worlds.md): elegir otro escenario.
