import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    LogInfo,
    OpaqueFunction,
    RegisterEventHandler,
)
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit, OnShutdown
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    use_sim_time = LaunchConfiguration("use_sim_time")
    namespace = LaunchConfiguration("namespace")
    x = LaunchConfiguration("x")
    y = LaunchConfiguration("y")
    z = LaunchConfiguration("z")
    roll = LaunchConfiguration("roll")
    pitch = LaunchConfiguration("pitch")
    yaw = LaunchConfiguration("yaw")
    use_ned_frame = LaunchConfiguration("use_ned_frame")
    use_teleop = LaunchConfiguration("use_teleop")
    use_web_joystick = LaunchConfiguration("use_web_joystick")
    joystick_ws_host = LaunchConfiguration("joystick_ws_host")
    joystick_ws_port = LaunchConfiguration("joystick_ws_port")
    zoom_camera = LaunchConfiguration("zoom_camera")
    zoom_camera_delay = LaunchConfiguration("zoom_camera_delay")
    open_qgc = LaunchConfiguration("open_qgc")
    open_virtual_joystick = LaunchConfiguration("open_virtual_joystick")
    virtual_joystick_url = LaunchConfiguration("virtual_joystick_url")
    ui_launch_delay = LaunchConfiguration("ui_launch_delay")
    world_name = LaunchConfiguration("world_name")

    description_file = PathJoinSubstitution(
        [
            FindPackageShare("posim_robot_models"),
            "description",
            namespace,
            "model.sdf",
        ]
    )

    cleanup_handlers = []
    cleanup_actions = []
    if LaunchConfiguration("qgc_video").perform(context) == "true":
        port = int(LaunchConfiguration("qgc_video_port").perform(context))
        if not 1 <= port <= 65535:
            raise ValueError("qgc_video_port must be between 1 and 65535")
        tree = ET.parse(description_file.perform(context))
        camera = tree.find(".//sensor[@type='camera']")
        if camera is None:
            raise ValueError("qgc_video requires a robot with a camera sensor")
        if any(plugin.get("filename") == "GstCameraPlugin" for plugin in camera.findall("plugin")):
            raise ValueError("The camera already defines GstCameraPlugin; configure it in the SDF")
        plugin = ET.SubElement(camera, "plugin", name="GstCameraPlugin", filename="GstCameraPlugin")
        for name, value in (
            ("udp_host", "127.0.0.1"),
            ("udp_port", str(port)),
            ("use_basic_pipeline", "true"),
            ("use_cuda", "false"),
        ):
            ET.SubElement(plugin, name).text = value
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", prefix="posim-qgc-", suffix=".sdf", delete=False
        ) as model_file:
            temporary_model = Path(model_file.name)
            try:
                tree.write(model_file, encoding="unicode")
            except Exception:
                temporary_model.unlink(missing_ok=True)
                raise
        description_file = str(temporary_model)

        def remove_temporary_model(_context):
            temporary_model.unlink(missing_ok=True)

        cleanup_actions = [OpaqueFunction(function=remove_temporary_model)]
        cleanup_handlers = [
            RegisterEventHandler(
                OnShutdown(on_shutdown=[OpaqueFunction(function=remove_temporary_model)])
            )
        ]

    tf2_spawner = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="world_to_world_ned",
        arguments=[
            "--roll",
            "1.57",
            "--yaw",
            "3.14",
            "--frame_id",
            "world",
            "--child_frame_id",
            "world_ned",
        ],
        output="both",
        condition=IfCondition(use_ned_frame),
        parameters=[{"use_sim_time": use_sim_time}],
    )

    gz_spawner = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-name",
            namespace,
            "-file",
            description_file,
            "-x",
            x,
            "-y",
            y,
            "-z",
            z,
            "-R",
            roll,
            "-P",
            pitch,
            "-Y",
            yaw,
        ],
        output="both",
        parameters=[{"use_sim_time": use_sim_time}],
    )

    nodes = [tf2_spawner, gz_spawner]

    # Include robot_config.py based on the model name
    robot_config = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("posim_robot_models"),
                        "config",
                        namespace,
                        "robot_config.py",
                    ]
                )
            ]
        ),
        launch_arguments={
            "namespace": namespace,
            "use_teleop": use_teleop,
            "use_web_joystick": use_web_joystick,
            "joystick_ws_host": joystick_ws_host,
            "joystick_ws_port": joystick_ws_port,
            "zoom_camera": zoom_camera,
            "zoom_camera_delay": zoom_camera_delay,
            "open_qgc": open_qgc,
            "open_virtual_joystick": open_virtual_joystick,
            "virtual_joystick_url": virtual_joystick_url,
            "ui_launch_delay": ui_launch_delay,
            "world_name": world_name,
        }.items(),
    )

    event_handlers = [
        RegisterEventHandler(
            OnProcessExit(
                target_action=gz_spawner,
                on_exit=cleanup_actions + [LogInfo(msg="Robot Model Uploaded"), robot_config],
            )
        )
    ]

    return cleanup_handlers + nodes + event_handlers


def generate_launch_description():
    args = [
        DeclareLaunchArgument(
            "gui",
            default_value="true",
            description="Flag to indicate whether to use simulation",
        ),
        DeclareLaunchArgument(
            "use_sim_time",
            default_value="true",
            description="Flag to indicate whether to use sim time",
        ),
        DeclareLaunchArgument(
            "namespace",
            default_value="bluerov2",
            description="Namespace",
        ),
        DeclareLaunchArgument(
            "x",
            default_value="0",
            description="Initial x position",
        ),
        DeclareLaunchArgument(
            "y",
            default_value="0",
            description="Initial y position",
        ),
        DeclareLaunchArgument(
            "z",
            default_value="0.0",
            description="Initial z position",
        ),
        DeclareLaunchArgument(
            "roll",
            default_value="0.0",
            description="Initial roll",
        ),
        DeclareLaunchArgument(
            "pitch",
            default_value="0.0",
            description="Initial pitch",
        ),
        DeclareLaunchArgument(
            "yaw",
            default_value="0.0",
            description="Initial yaw",
        ),
        DeclareLaunchArgument(
            "use_ned_frame",
            default_value="false",
            description="Use North-East-Down frame",
        ),
        DeclareLaunchArgument(
            "use_teleop",
            default_value="true",
            description="Launch BlueROV teleop nodes",
        ),
        DeclareLaunchArgument(
            "use_web_joystick",
            default_value="true",
            description="Launch websocket joystick bridge",
        ),
        DeclareLaunchArgument(
            "joystick_ws_host",
            default_value="0.0.0.0",
            description="Bind host for websocket joystick bridge",
        ),
        DeclareLaunchArgument(
            "joystick_ws_port",
            default_value="8765",
            description="Bind port for websocket joystick bridge",
        ),
        DeclareLaunchArgument(
            "qgc_video",
            default_value="false",
            choices=["true", "false"],
            description="Add an optional H.264 camera stream for QGroundControl",
        ),
        DeclareLaunchArgument(
            "qgc_video_port",
            default_value="5600",
            description="Local UDP destination port for the QGroundControl video stream",
        ),
        DeclareLaunchArgument(
            "open_qgc",
            default_value="false",
            description="Launch QGroundControl",
        ),
        DeclareLaunchArgument(
            "open_virtual_joystick",
            default_value="false",
            description="Open the virtual joystick page in Firefox",
        ),
        DeclareLaunchArgument(
            "virtual_joystick_url",
            default_value="http://127.0.0.1:8080/virtual_joystick.html",
            description="URL for the virtual joystick page",
        ),
        DeclareLaunchArgument(
            "ui_launch_delay",
            default_value="2.0",
            description="Delay (seconds) before launching QGC/Firefox",
        ),
        DeclareLaunchArgument(
            "zoom_camera",
            default_value="false",
            description="Zoom the GUI camera after launch",
        ),
        DeclareLaunchArgument(
            "zoom_camera_delay",
            default_value="2.0",
            description="Delay (seconds) before moving the GUI camera",
        ),
        DeclareLaunchArgument(
            "world_name",
            default_value="empty",
            description="Gazebo world entity name",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
