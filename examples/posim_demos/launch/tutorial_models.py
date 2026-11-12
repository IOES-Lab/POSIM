"""Small tutorial configurations derived from the shipped robot descriptions."""

from pathlib import Path
import xml.etree.ElementTree as ET

ET.register_namespace("gz", "http://gazebosim.org")


def camera_stream(sensor):
    stream = ET.SubElement(sensor, "plugin", name="GstCameraPlugin", filename="GstCameraPlugin")
    for name, value in [
        ("udp_host", "127.0.0.1"),
        ("udp_port", "5600"),
        ("use_basic_pipeline", "true"),
        ("use_cuda", "false"),
        # Each low-rate frame must be independently decodable. Waiting for a
        # long GOP can outlast QGC's receiver timeout on a slow simulation.
        ("keyframe_interval", "1"),
    ]:
        ET.SubElement(stream, name).text = value


def underwater_model(robot_share, output, sonar=False):
    """Keep model dynamics and interfaces, with a modest optional sonar workload."""
    robot = "bluerov2_heavy_multibeam_sonar" if sonar else "bluerov2"
    tree = ET.parse(Path(robot_share) / "description" / robot / "model.sdf")
    model = tree.getroot().find("model")
    base = model.find("link[@name='base_link']")
    # Neutral trim keeps an unarmed tutorial vehicle beside the inspection
    # targets. The standard model is unchanged; this is the tutorial's ballast.
    size = base.findtext("collision/geometry/box/size")
    volume = 1.0
    for length in size.split():
        volume *= float(length)
    other_mass = sum(
        float(link.findtext("inertial/mass", "0"))
        for link in model.findall("link")
        if link is not base
    )
    base.find("inertial/mass").text = str(1025.0 * volume - other_mass)
    camera = model.find(".//sensor[@type='camera']")
    camera.find("always_on").text = "true"
    camera.find("topic").text = "/tutorial/camera"
    # Let underwater haze reach its background colour before clipping the
    # seabed. This is independent of the sonar's ten-metre acoustic range.
    camera.find("camera/clip/far").text = "100"
    camera_stream(camera)
    if sonar:
        sensor = model.find(".//sensor[@name='multibeam_sonar']")
        sensor.find("update_rate").text = "5"
        sensor.find("visualize").text = "false"
        config = sensor.find("{http://gazebosim.org}multibeam_sonar")
        config.find("ray/scan/horizontal/beams").text = "128"
        config.find("ray/scan/vertical/rays").text = "64"
        config.find("spec/bandwidth").text = "15000"
        config.find("spec/writeLog").text = "false"
        config.find("spec/debugFlag").text = "false"
    ET.indent(tree)
    tree.write(output, encoding="utf-8", xml_declaration=True)
    return robot


def surface_sensors(model_file):
    """Add the tutorial's camera stream and planar lidar to the external WAM-V."""
    tree = ET.parse(model_file)
    link = tree.find("model/link[@name='base_link']")
    camera = link.find("sensor[@type='camera']")
    camera.find("topic").text = "/tutorial/camera"
    camera.find("always_on").text = "true"
    camera_stream(camera)
    lidar = ET.SubElement(link, "sensor", name="lidar", type="gpu_lidar")
    ET.SubElement(lidar, "pose").text = "1.8 0 1.5 0 0 0"
    ET.SubElement(lidar, "topic").text = "/tutorial/lidar"
    ET.SubElement(lidar, "update_rate").text = "5"
    ET.SubElement(lidar, "always_on").text = "true"
    ray = ET.SubElement(lidar, "lidar")
    horizontal = ET.SubElement(ET.SubElement(ray, "scan"), "horizontal")
    for name, value in [
        ("samples", "360"),
        ("resolution", "1"),
        ("min_angle", "-3.14159265359"),
        ("max_angle", "3.14159265359"),
    ]:
        ET.SubElement(horizontal, name).text = value
    limits = ET.SubElement(ray, "range")
    for name, value in [("min", ".2"), ("max", "40"), ("resolution", ".01")]:
        ET.SubElement(limits, name).text = value
    ET.indent(tree)
    tree.write(model_file, encoding="utf-8", xml_declaration=True)


def underwater_world(world_file, output, sonar=False):
    """Only show the Gazebo sonar image panel when the sensor is present."""
    tree = ET.parse(world_file)
    if sonar:
        panel = ET.SubElement(
            tree.find("world/gui"), "plugin", filename="ImageDisplay", name="Sonar"
        )
        gui = ET.SubElement(panel, "gz-gui")
        ET.SubElement(gui, "title").text = "Sonar"
        ET.SubElement(gui, "property", type="string", key="state").text = "docked"
        ET.SubElement(panel, "topic").text = "/tutorial/sonar"
        ET.SubElement(panel, "topic_picker").text = "false"
        ET.SubElement(panel, "show_depth_flip").text = "false"
    ET.indent(tree)
    tree.write(output, encoding="utf-8", xml_declaration=True)


def vehicle_description(model_file, model_roots):
    """Use the simulated vehicle's meshes and mounting poses in RViz.

    This display copy fixes the propellers at their rest angles; the simulation
    retains the original joints and dynamics.
    """
    tree = ET.parse(model_file)
    model = tree.find("model")
    pose = model.find("pose")
    if pose is not None:
        model.remove(pose)
    for plugin in list(model.findall("plugin")):
        model.remove(plugin)
    for link in model.findall("link"):
        for sensor in list(link.findall("sensor")):
            link.remove(sensor)
    for joint in model.findall("joint"):
        joint.set("type", "fixed")
        axis = joint.find("axis")
        if axis is not None:
            joint.remove(axis)
    for uri in model.findall(".//mesh/uri"):
        if uri.text.startswith("model://"):
            relative = uri.text.removeprefix("model://")
            mesh = next(
                (
                    Path(root) / relative
                    for root in model_roots
                    if (Path(root) / relative).is_file()
                ),
                None,
            )
            if mesh is None:
                raise FileNotFoundError(f"Tutorial vehicle mesh not found: {uri.text}")
            uri.text = mesh.resolve().as_uri()
    return ET.tostring(tree.getroot(), encoding="unicode")
