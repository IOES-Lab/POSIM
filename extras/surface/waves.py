"""Configure the external wave model without copying its implementation."""

import os
from pathlib import Path
import xml.etree.ElementTree as ET


def prepare(world, x, y):
    world = Path(world)
    root = (
        Path(os.getenv("ASV_WAVE_SIM_ROOT", "/opt/asv_wave_sim"))
        / "gz-waves-models/world_models/waves"
    )
    source = root / "model.sdf"
    if not source.is_file():
        raise RuntimeError("external_wave_sim_dependency_missing")
    tree = ET.parse(world)
    w = tree.getroot().find("world")
    wave = ET.parse(source).getroot().find("model")
    # Bound the external mesh hydrodynamics work to 200 Hz; SITL still owns
    # steering/throttle. This setting is validated with the surface mission.
    w.find("physics/max_step_size").text = ".005"
    for p in list(w.findall("plugin")):
        if p.get("name") == "gz::sim::systems::Buoyancy":
            w.remove(p)
    ET.SubElement(wave, "pose").text = f"{round(x / 256) * 256} {round(y / 256) * 256} 0 0 0 0"
    for item in wave.iter():
        if (
            item.tag
            in ("uri", "albedo_map", "normal_map", "environment_map", "vertex", "fragment")
            and item.text
            and not item.text.startswith(("model://", "file://", "/"))
        ):
            item.text = str((root / item.text).resolve())
    for values in wave.iter("wave"):
        for tag, value in [
            ("cell_count", "64"),
            ("wind_speed", "3"),
            ("wind_angle_deg", "135"),
            ("steepness", "1"),
        ]:
            values.find(tag).text = value
    for p in wave.iter("plugin"):
        if p.get("name") == "gz::sim::systems::WavesVisual":
            p.find("tiles_x").text = "-1 1"
            p.find("tiles_y").text = "-1 1"
    w.append(wave)
    target = world.with_name("world-waves.sdf")
    tree.write(target, encoding="utf-8", xml_declaration=True)
    return target
