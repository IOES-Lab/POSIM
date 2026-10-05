# Objects and task scenes

POSIM includes object descriptions for manipulation, inspection and interaction. Models are installed by the `posim_object_models` package alongside their mesh and configuration resources.

## Explore the catalog

Local descriptions are installed with POSIM. Other task objects are referenced from Gazebo Fuel by the example worlds. The local catalog is generated from `models/posim_object_models/description` in the current checkout. Open a directory to inspect its SDF, links, joints and resources.

{{OBJECT_CATALOG}}

## Start with a composed scene

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_plug_and_socket headless:=true
```

Other task scenes include `posim_electrical_mating` and `posim_bimanual_example`. Their Fuel includes provide a blow-out preventer panel, male/female plugs and sunken-vase inspection targets. The world SDF records each asset URI and placement. Read it before selecting robot placement, contact or joint-control experiments.

## Place an object in your own world

Use a model URI that resolves through the sourced package resources. A minimal world include has this structure; replace the URI with a catalog model's actual resource name:

```xml
<include>
  <uri>model://YOUR_OBJECT</uri>
  <name>task_object_1</name>
  <pose>3 0 -5 0 0 0</pose>
</include>
```

Positions are in meters and orientation angles are in radians. Assign a distinct entity name for each instance. Choose `static` behavior deliberately: a fixed inspection target and an object manipulated through contact need different dynamics.

## Check an interaction

Visual geometry shows appearance; collision geometry determines contact. Inspect both, including their scale and pose. Check mass, inertia, joint limits and friction before interpreting a manipulation result. Confirm object placement and contact with a short controlled experiment, then record the robot and object states using [ROS 2](ros.md).

Use [Add a robot](custom-robots.md) for model/resource conventions and [World library](worlds.md) for scene composition.
