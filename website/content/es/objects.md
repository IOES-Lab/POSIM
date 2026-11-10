# Objetos y escenarios de tareas

POSIM incluye modelos para manipulación, inspección e interacción. `posim_object_models` instala sus descripciones, mallas y configuración. Ejecuta los comandos en la [terminal POSIM de tu instalación](terminals.md), también en WSL, Docker o RDP.

## Catálogo local

Los directorios siguientes proceden de `models/posim_object_models/description`. Ábrelos para examinar SDF, eslabones, articulaciones y recursos. Los mundos de ejemplo también pueden usar objetos de Gazebo Fuel.

{{OBJECT_CATALOG}}

## Empezar con una escena preparada

{{CODE:objects:0}}

Otros escenarios son `posim_electrical_mating` y `posim_bimanual_example`. Usan recursos Fuel como un panel de prevención de reventones, conectores macho y hembra y jarrones sumergidos. Consulta sus URI y posiciones en el SDF antes de colocar el robot o probar contactos y articulaciones.

## Añadir un objeto a un mundo

Sustituye la URI por el nombre real del recurso seleccionado:

{{CODE:objects:1}}

La posición se expresa en metros y los ángulos en radianes. Asigna un nombre distinto a cada instancia. Decide si el objeto será `static`: un blanco fijo de inspección y un objeto manipulable necesitan dinámicas diferentes.

La geometría visual define la apariencia; la de colisión, el contacto. Revisa escalas, posiciones, masa, inercia, límites de articulaciones y fricción. Prueba una interacción pequeña y controlada antes de interpretar resultados. Registra los estados mediante [ROS 2](ros.md).

## Lanzar un objeto individual

Con el entorno cargado, inicia `mossy_cinder_block`:

{{CODE:objects:2}}

Este lanzamiento crea su propio mundo. Para combinarlo con otro ejemplo, añade elementos `<include>` al SDF de un único mundo. Consulta [Añadir un robot](custom-robots.md) y [Mundos](worlds.md).

## Usar modelos de Fuel localmente

Descarga y extrae un modelo del [catálogo Fuel](https://app.gazebosim.org/fuel/models). Coloca juntos `model.sdf`, `model.config`, `meshes/` y `materials/`, por ejemplo en `~/posim_models/my_object/`. Registra el directorio padre en la terminal que iniciará Gazebo:

{{CODE:objects:3}}

En Gazebo, abre **Resource Spawner** para colocar el modelo local. Para reutilizarlo en un SDF, usa `<uri>model://my_object</uri>`. También puedes usar la URI HTTPS del fragmento SDF de Fuel; necesitará una primera descarga. Conserva archivos y licencias para trabajar sin conexión.

## Publicar en Fuel

Inicia sesión en [Gazebo Fuel](https://app.gazebosim.org/home) y crea un modelo. Indica nombre, descripción y licencia; sube `model.sdf`, `model.config` y las mallas y materiales referenciados. Descarga el resultado y comprueba que no dependa de rutas locales externas. Puedes añadirlo a una colección.

Usa nombres de bibliotecas y clases de plugins disponibles en Jetty. [Añadir un robot](custom-robots.md) muestra cómo conectar flotabilidad, hidrodinámica, propulsión y sensores.

{{MEDIA:objects}}

Imágenes: wiki Notion de POSIM. La [página de citas y licencias](citation.md) recoge la atribución de la documentación DAVE. Para comandos y tópicos, usa los bloques de esta página.
