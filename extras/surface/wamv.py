"""
Runtime configuration overlay for external Wave Sim's WAM-V assets.

The upstream model, meshes and GPL wave implementation stay in its separately
pinned checkout. This file contains only vehicle integration configuration.
"""

from pathlib import Path
import os
import xml.etree.ElementTree as ET


def make_model(out, namespace="wamv", root=None):
    root = Path(root or os.getenv("ASV_WAVE_SIM_ROOT", "/opt/asv_wave_sim"))
    source = root / "gz-waves-models/models/wam-v/model.sdf"
    if not source.is_file():
        raise RuntimeError("external_wave_sim_wamv_dependency_missing")
    tree = ET.parse(source)
    model = tree.getroot().find("model")
    model.set("name", namespace)
    for item in model.iter():
        if item.text and "wam-v" in item.text and not item.text.strip().startswith("model://"):
            item.text = item.text.replace("wam-v", namespace)
    for joint in model.findall("joint"):
        if joint.get("name") in ("left_engine_joint", "right_engine_joint", "imu_joint"):
            joint.set("type", "fixed")
            axis = joint.find("axis")
            joint.remove(axis) if axis is not None else None
    for plugin in list(model.findall("plugin")):
        if plugin.get("name") in (
            "ArduPilotPlugin",
            "gz::sim::systems::ApplyJointForce",
            "gz::sim::systems::JointStatePublisher",
        ):
            model.remove(plugin)
    imu = model.find("link[@name='imu_link']/sensor")
    imu.find("update_rate").text = "200"
    # ArduPilotPlugin computes the scoped IMU topic; an explicit custom topic
    # would leave its JSON sensor stream empty and deadlock lockstep startup.
    ap = ET.SubElement(model, "plugin", name="ArduPilotPlugin", filename="libArduPilotPlugin.so")
    for tag, value in [
        ("fdm_addr", "127.0.0.1"),
        ("fdm_port_in", "9002"),
        ("connectionTimeoutMaxCount", "5"),
        ("lock_step", "1"),
        ("modelXYZToAirplaneXForwardZDown", "0 0 0 3.14159265359 0 0"),
        ("gazeboXYZToNED", "0 0 0 3.14159265359 0 1.57079632679"),
        ("imuName", "imu_sensor"),
    ]:
        ET.SubElement(ap, tag).text = value
    for channel, side in [(0, "left"), (2, "right")]:
        control = ET.SubElement(ap, "control", channel=str(channel))
        for tag, value in [
            ("jointName", side + "_propeller_joint"),
            ("servo_min", "1000"),
            ("servo_max", "2000"),
            ("type", "COMMAND"),
            ("cmd_topic", f"/model/{namespace}/joint/{side}_propeller_joint/cmd_thrust"),
            ("offset", "-.5"),
            ("multiplier", "500"),
        ]:
            ET.SubElement(control, tag).text = value
    odom = ET.SubElement(
        model,
        "plugin",
        filename="gz-sim-odometry-publisher-system",
        name="gz::sim::systems::OdometryPublisher",
    )
    for tag, value in [
        ("dimensions", "3"),
        ("odom_frame", "map"),
        ("robot_base_frame", "base_link"),
        ("odom_publish_frequency", "20"),
        ("odom_topic", f"/model/{namespace}/odometry"),
    ]:
        ET.SubElement(odom, tag).text = value
    # A forward camera remains visible in the discovered sensor tree.
    link = model.find("link[@name='base_link']")
    sensor = ET.SubElement(link, "sensor", name="forward_camera", type="camera")
    for tag, value in [
        ("pose", "1.8 0 1.5 0 .08 0"),
        ("always_on", "false"),
        ("update_rate", "5"),
        ("topic", f"/model/{namespace}/camera"),
    ]:
        ET.SubElement(sensor, tag).text = value
    camera = ET.SubElement(sensor, "camera")
    ET.SubElement(camera, "horizontal_fov").text = "1.05"
    im = ET.SubElement(camera, "image")
    ET.SubElement(im, "width").text = "640"
    ET.SubElement(im, "height").text = "480"
    clip = ET.SubElement(camera, "clip")
    ET.SubElement(clip, "near").text = ".1"
    ET.SubElement(clip, "far").text = "80"
    ET.indent(tree)
    tree.write(out, encoding="utf-8", xml_declaration=True)
    return str(out)
