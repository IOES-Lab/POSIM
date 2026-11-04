"""Check generated worlds and ROS launch wiring without starting simulator processes."""

import importlib.util
import os
from pathlib import Path
import shlex
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

from launch import LaunchContext, LaunchDescription, LaunchService
from launch.actions import (
    DeclareLaunchArgument,
    EmitEvent,
    ExecuteProcess,
    LogError,
    OpaqueFunction,
    RegisterEventHandler,
)
from launch.event_handlers import OnProcessExit
from launch_ros.actions import Node

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("surface_launch", ROOT / "wamv.launch.py")
surface_launch = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(surface_launch)


class WorldNameTests(unittest.TestCase):
    def test_process_exit_gates_next_stage(self):
        for exit_code in (0, 4):
            with self.subTest(exit_code=exit_code), tempfile.TemporaryDirectory() as temporary:
                marker = Path(temporary) / "next-stage"
                first = ExecuteProcess(cmd=["bash", "-c", f"exit {exit_code}"])
                next_stage = ExecuteProcess(cmd=["touch", str(marker)])
                handler = RegisterEventHandler(
                    OnProcessExit(
                        target_action=first,
                        on_exit=lambda event, context: surface_launch.continue_on_success(
                            event, [next_stage], "model spawn"
                        ),
                    )
                )
                service = LaunchService(noninteractive=True)
                service.include_launch_description(LaunchDescription([handler, first]))
                service.run()
                self.assertEqual(marker.exists(), exit_code == 0)

    def test_failed_startup_does_not_start_the_next_stage(self):
        next_stage = [object(), object()]
        for stage in ("model spawn", "IMU readiness"):
            with self.subTest(stage=stage):
                self.assertIs(
                    surface_launch.continue_on_success(
                        SimpleNamespace(returncode=0), next_stage, stage
                    ),
                    next_stage,
                )
                failed = surface_launch.continue_on_success(
                    SimpleNamespace(returncode=1), next_stage, stage
                )
                self.assertEqual(len(failed), 2)
                self.assertIsInstance(failed[0], LogError)
                self.assertIsInstance(failed[1], EmitEvent)
                self.assertTrue(all(action not in failed for action in next_stage))

    def test_default_and_application_names_keep_world_spawn_and_imu_connected(self):
        for override, expected in (
            (None, "posim_surface"),
            ("harbour", "harbour"),
            ("wwos_gebco", "wwos_gebco"),
        ):
            with self.subTest(world=expected), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                waves = root / "gz-waves-models/world_models/waves"
                waves.mkdir(parents=True)
                (waves / "model.sdf").write_text(
                    '<sdf version="1.10"><model name="waves"><plugin '
                    'name="gz::sim::systems::WavesVisual" filename="waves">'
                    "<tiles_x>0 0</tiles_x><tiles_y>0 0</tiles_y><wave>"
                    "<cell_count>32</cell_count><wind_speed>1</wind_speed>"
                    "<wind_angle_deg>0</wind_angle_deg><steepness>1</steepness>"
                    "</wave></plugin></model></sdf>"
                )
                model = root / "gz-waves-models/models/wam-v"
                model.mkdir(parents=True)
                (model / "model.sdf").write_text(
                    '<sdf version="1.10"><model name="wam-v"><link name="base_link"/>'
                    '<link name="imu_link"><sensor name="imu_sensor" type="imu">'
                    "<update_rate>100</update_rate></sensor></link></model></sdf>"
                )
                output = root / "generated"

                def isolated_path(value):
                    return output if str(value) == "/tmp/posim-surface" else Path(value)

                context = LaunchContext()
                if override is not None:
                    context.launch_configurations["world_name"] = override
                with patch.dict(os.environ, {"ASV_WAVE_SIM_ROOT": str(root)}), patch.object(
                    surface_launch, "Path", side_effect=isolated_path
                ), patch.object(surface_launch, "Node", wraps=Node) as nodes, patch.object(
                    surface_launch, "ExecuteProcess", wraps=ExecuteProcess
                ) as processes:
                    for action in surface_launch.generate_launch_description().entities:
                        self.assertIsInstance(action, (DeclareLaunchArgument, OpaqueFunction))
                        action.execute(context)

                world = ET.parse(output / "world-waves.sdf").getroot().find("world")
                self.assertEqual(world.get("name"), expected)
                self.assertIsNotNone(world.find("model[@name='waves']"))
                vehicle = ET.parse(output / "wamv.sdf").getroot().find("model")
                sensor_link = vehicle.find("link[@name='imu_link']")
                sensor = sensor_link.find("sensor")
                imu = (
                    f"/world/{world.get('name')}/model/{vehicle.get('name')}"
                    f"/link/{sensor_link.get('name')}/sensor/{sensor.get('name')}/imu"
                )
                node_configs = [call.kwargs for call in nodes.call_args_list]
                create = next(c for c in node_configs if c["package"] == "ros_gz_sim")
                self.assertEqual(create["arguments"][1], world.get("name"))
                bridge = next(c for c in node_configs if c["package"] == "ros_gz_bridge")
                self.assertIn(imu + "@sensor_msgs/msg/Imu[gz.msgs.IMU", bridge["arguments"])
                self.assertEqual(bridge["remappings"], [(imu, "/model/wamv/imu")])

                commands = [call.kwargs["cmd"] for call in processes.call_args_list]
                server = next(cmd for cmd in commands if cmd[0] == "gz")
                self.assertIn(str(output / "world-waves.sdf"), server)
                self.assertIn("--headless-rendering", server)
                wait = next(cmd for cmd in commands if cmd[:2] == ["bash", "-c"])
                tokens = shlex.split(wait[2])
                self.assertEqual(tokens[tokens.index("-t") + 1], imu)


if __name__ == "__main__":
    unittest.main(verbosity=2)
