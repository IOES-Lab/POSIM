"""The standalone robot examples must provide ROS simulation time."""

import ast
from pathlib import Path

import pytest


@pytest.mark.parametrize("model", ["rexrov", "glider_slocum"])
def test_one_way_sim_clock(model):
    config = Path(__file__).parents[1] / "config" / model / "robot_config.py"
    strings = [
        node.value
        for node in ast.walk(ast.parse(config.read_text()))
        if isinstance(node, ast.Constant) and isinstance(node.value, str)
    ]
    clocks = [s for s in strings if s.startswith("/clock@")]
    assert clocks == ["/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock"]
