"""BlueROV inspection at four metres, with optional multibeam sonar."""

from pathlib import Path
import sys
import tempfile

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
    RegisterEventHandler,
    SetEnvironmentVariable,
)
from launch.conditions import IfCondition
from launch.event_handlers import OnShutdown
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, str(Path(__file__).parent))
from tutorial_models import underwater_model, underwater_world, vehicle_description  # noqa: E402


def setup(context):
    share = Path(get_package_share_directory("posim_demos"))
    robots = Path(get_package_share_directory("posim_robot_models"))
    sonar = LaunchConfiguration("sonar").perform(context).lower() == "true"
    temporary = tempfile.TemporaryDirectory(prefix="posim-underwater-tutorial-")
    folder = Path(temporary.name)
    model = folder / "robot.sdf"
    robot = underwater_model(robots, model, sonar)
    world = folder / "world.sdf"
    underwater_world(
        Path(get_package_share_directory("posim_worlds")) / "worlds/shallow_underwater.world",
        world,
        sonar,
    )
    preset = share / "rviz" / ("underwater_sonar.rviz" if sonar else "underwater.rviz")
    rviz_file = folder / "tutorial.rviz"
    rviz_file.write_text(preset.read_text().replace("ROBOT", robot))

    def cleanup(_context):
        temporary.cleanup()

    simulation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(str(share / "launch/posim_robot.launch.py")),
        launch_arguments={
            "namespace": robot,
            "model_file": str(model),
            "world_name": "shallow_underwater",
            "world_file": str(world),
            "x": "0",
            "z": "-3.1",
            "paused": "false",
            "gui": LaunchConfiguration("gui"),
            "use_teleop": "false",
            "use_web_joystick": "false",
            "open_qgc": "false",
            "open_virtual_joystick": "false",
            "qgc_video": "false",  # The tutorial SDF already contains its sender.
        }.items(),
    )
    camera = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=["/tutorial/camera@sensor_msgs/msg/Image[gz.msgs.Image"],
        parameters=[{"use_sim_time": True}],
    )
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", str(rviz_file)],
        parameters=[{"use_sim_time": True}],
        condition=IfCondition(LaunchConfiguration("rviz")),
    )
    vehicle = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[
            {
                "robot_description": vehicle_description(
                    model, [robots, get_package_share_directory("posim_sensor_models")]
                ),
                "frame_prefix": robot + "/",
                "use_sim_time": True,
            }
        ],
    )
    actions = [
        RegisterEventHandler(OnShutdown(on_shutdown=[OpaqueFunction(function=cleanup)])),
        SetEnvironmentVariable(
            "POSIM_SONAR_COMPUTE_BACKEND", LaunchConfiguration("compute_backend")
        ),
        simulation,
        camera,
        vehicle,
        rviz,
    ]
    if sonar:
        actions.append(
            Node(
                package="posim_demos",
                executable="sonar_display.py",
                remappings=[
                    ("sonar_image", f"/model/{robot}/multibeam_sonar/sonar_image"),
                    ("sonar_rgb", "/tutorial/sonar_rgb"),
                ],
                parameters=[{"use_sim_time": True}],
                condition=IfCondition(LaunchConfiguration("gui")),
            )
        )
        actions.append(
            Node(
                package="ros_gz_bridge",
                executable="parameter_bridge",
                arguments=["/tutorial/sonar@sensor_msgs/msg/Image]gz.msgs.Image"],
                remappings=[("/tutorial/sonar", "/tutorial/sonar_rgb")],
                condition=IfCondition(LaunchConfiguration("gui")),
            )
        )
    return actions


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument("sonar", default_value="false", choices=["true", "false"]),
            DeclareLaunchArgument(
                "compute_backend", default_value="auto", choices=["auto", "cpu", "wgpu", "cuda"]
            ),
            DeclareLaunchArgument("gui", default_value="true", choices=["true", "false"]),
            DeclareLaunchArgument("rviz", default_value="true", choices=["true", "false"]),
            OpaqueFunction(function=setup),
        ]
    )
