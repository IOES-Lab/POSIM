"""Optional external-wave WAM-V + ArduRover demo; does not change BlueROV defaults."""

from pathlib import Path
import sys
import xml.etree.ElementTree as ET
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    ExecuteProcess,
    RegisterEventHandler,
)
from launch.event_handlers import OnProcessExit
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, str(Path(__file__).parent))
from wamv import make_model  # noqa: E402 - sibling module after launch loader path
from waves import prepare  # noqa: E402 - sibling module after launch loader path


def setup(context):
    folder = Path("/tmp/posim-surface")
    folder.mkdir(exist_ok=True)
    sdf = ET.Element("sdf", version="1.10")
    world = ET.SubElement(sdf, "world", name="wwos_gebco")
    physics = ET.SubElement(world, "physics", name="default", type="ignored")
    ET.SubElement(physics, "max_step_size").text = ".005"
    ET.SubElement(physics, "real_time_factor").text = "1"
    for filename, name in [
        ("physics", "Physics"),
        ("user-commands", "UserCommands"),
        ("scene-broadcaster", "SceneBroadcaster"),
        ("imu", "Imu"),
        ("sensors", "Sensors"),
    ]:
        p = ET.SubElement(
            world,
            "plugin",
            filename="gz-sim-" + filename + "-system",
            name="gz::sim::systems::" + name,
        )
        if name == "Sensors":
            ET.SubElement(p, "render_engine").text = "ogre2"
    scene = ET.SubElement(world, "scene")
    ET.SubElement(scene, "ambient").text = ".4 .4 .4 1"
    sun = ET.SubElement(world, "light", name="sun", type="directional")
    ET.SubElement(sun, "diffuse").text = "1 1 1 1"
    ET.SubElement(sun, "direction").text = ".2 .2 -1"
    world.append(
        ET.fromstring(
            '<model name="floor"><static>true</static><pose>0 0 -10 0 0 0</pose><link name="floor"><collision name="floor"><geometry><plane><normal>0 0 1</normal><size>1000 1000</size></plane></geometry></collision><visual name="floor"><geometry><plane><normal>0 0 1</normal><size>1000 1000</size></plane></geometry><material><diffuse>.6 .5 .3 1</diffuse></material></visual></link></model>'
        )
    )
    path = folder / "world.sdf"
    ET.ElementTree(sdf).write(path)
    path = prepare(path, 0, 0)
    args = ["gz", "sim", "--force-version", "10", str(path), "-r"]
    if LaunchConfiguration("headless").perform(context) == "true":
        args += ["-s", "--headless-rendering"]
    server = ExecuteProcess(cmd=args, output="screen")
    create = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-world",
            "wwos_gebco",
            "-file",
            make_model(folder / "wamv.sdf"),
            "-name",
            "wamv",
            "-z",
            "0",
        ],
        output="screen",
    )
    imu = "/world/wwos_gebco/model/wamv/link/imu_link/sensor/imu_sensor/imu"
    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/model/wamv/odometry@nav_msgs/msg/Odometry[gz.msgs.Odometry",
            imu + "@sensor_msgs/msg/Imu[gz.msgs.IMU",
        ],
        remappings=[(imu, "/model/wamv/imu")],
        output="screen",
    )
    wait = ExecuteProcess(
        cmd=[
            "bash",
            "-c",
            f"until timeout 2 gz topic -e -t {imu} -n 1 >/dev/null 2>&1; do :; done",
        ],
        output="screen",
    )
    sitl = ExecuteProcess(
        cmd=[
            "ardurover",
            "--speedup",
            "1",
            "-w",
            "--model",
            "JSON:127.0.0.1",
            "--defaults",
            str(Path(__file__).with_name("rover.parm")),
            "-IO",
            "--home",
            "35.07446,129.08468,0,90",
        ],
        output="screen",
    )
    mavros = Node(
        package="mavros",
        executable="mavros_node",
        parameters=[str(Path(__file__).with_name("mavros.yaml")), {"use_sim_time": True}],
        output="screen",
    )
    navigation = ExecuteProcess(
        cmd=["python3", str(Path(__file__).with_name("external_navigation.py"))], output="screen"
    )
    return [
        navigation,
        server,
        create,
        RegisterEventHandler(OnProcessExit(target_action=create, on_exit=[bridge, wait])),
        RegisterEventHandler(OnProcessExit(target_action=wait, on_exit=[sitl, mavros])),
    ]


def generate_launch_description():
    return LaunchDescription(
        [DeclareLaunchArgument("headless", default_value="true"), OpaqueFunction(function=setup)]
    )
