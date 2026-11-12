"""WAM-V, forward camera and lidar among anchored channel buoys."""

from pathlib import Path
import os
import sys
import tempfile

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
    RegisterEventHandler,
)
from launch.conditions import IfCondition
from launch.event_handlers import OnShutdown
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

sys.path.insert(0, str(Path(__file__).parent))
from tutorial_models import surface_sensors, vehicle_description  # noqa: E402


def setup(context):
    share = Path(get_package_share_directory("posim_demos"))
    worlds = Path(get_package_share_directory("posim_worlds"))
    sys.path.insert(0, str(share / "surface"))
    from wamv import make_model

    temporary = tempfile.TemporaryDirectory(prefix="posim-surface-tutorial-")
    model = Path(temporary.name) / "wamv.sdf"
    make_model(model)
    surface_sensors(model)

    def cleanup(_context):
        temporary.cleanup()

    simulation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(str(share / "surface/wamv.launch.py")),
        launch_arguments={
            "world_name": "shallow_surface",
            "world_file": str(worlds / "worlds/shallow_surface.world"),
            "model_file": str(model),
            "headless": LaunchConfiguration("headless"),
        }.items(),
    )
    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/tutorial/camera@sensor_msgs/msg/Image[gz.msgs.Image",
            "/tutorial/lidar@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan",
            "/tutorial/lidar/points@sensor_msgs/msg/PointCloud2[gz.msgs.PointCloudPacked",
        ],
        parameters=[{"use_sim_time": True}],
    )
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", str(share / "rviz/surface.rviz")],
        parameters=[{"use_sim_time": True}],
        condition=IfCondition(LaunchConfiguration("rviz")),
    )
    lidar_frame = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments=[
            "--x",
            "1.8",
            "--y",
            "0",
            "--z",
            "1.5",
            "--frame-id",
            "wamv/base_link",
            "--child-frame-id",
            "wamv/base_link/lidar",
        ],
    )
    vehicle = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[
            {
                "robot_description": vehicle_description(
                    model,
                    [
                        Path(os.environ.get("ASV_WAVE_SIM_ROOT", "/opt/asv_wave_sim"))
                        / "gz-waves-models/models"
                    ],
                ),
                "frame_prefix": "wamv/",
                "use_sim_time": True,
            }
        ],
    )
    return [
        RegisterEventHandler(OnShutdown(on_shutdown=[OpaqueFunction(function=cleanup)])),
        simulation,
        bridge,
        lidar_frame,
        vehicle,
        rviz,
    ]


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument("headless", default_value="false", choices=["true", "false"]),
            DeclareLaunchArgument("rviz", default_value="true", choices=["true", "false"]),
            OpaqueFunction(function=setup),
        ]
    )
