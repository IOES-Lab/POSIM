# Planeador Slocum

El modelo `glider_slocum` incluye cuerpo, propulsor, hidrodinámica e interfaces de IMU, navegación y presión. Puedes usarlo para experimentos con planeadores o como base de otro vehículo.

## Iniciar en el océano

En la [terminal POSIM preparada para tu instalación](terminals.md):

{{CODE:gliders:0}}

Para observarlo en un mundo vacío, utiliza `world_name:=empty.sdf z:=0.2`.

## Consultar las mediciones

{{CODE:gliders:1}}

Los tópicos Gazebo de IMU y NavSat están bajo `/model/glider_slocum/`. Consulta la configuración de puentes para elegir sus tópicos y tipos ROS. El modelo también carga el [plugin de presión](pressure.md) y el publicador de odometría de Gazebo.

Con el vehículo y el entorno activos, conecta tu nodo de control para realizar inmersiones o misiones de planeo.

## Adaptar el modelo

Lee el [SDF de Slocum](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) junto con su configuración de puentes. Modifica de forma coherente masa, inercia, geometría de colisión y coeficientes hidrodinámicos. Comprueba los sistemas de referencia antes de conectar un estimador o controlador.

Conserva el modelo original para comparar. [Añadir un robot](custom-robots.md) explica cómo crear otra descripción; [ROS](ros.md), cómo observarla y controlarla.

## Ver el planeador en movimiento

{{MEDIA:gliders}}

Imágenes y vídeos: POSIM Notion Wiki. Consulta la atribución a la documentación DAVE en [Citas y licencias](citation.md). Los comandos y tópicos vigentes son los de esta página.
