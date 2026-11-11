"""Check the geographic axes published by each optional world_ned frame."""

import ast
import math
from pathlib import Path

import pytest


@pytest.mark.parametrize("kind", ["robot", "sensor", "object"])
@pytest.mark.parametrize(
    "ned,enu",
    [((1, 0, 0), (0, 1, 0)), ((0, 1, 0), (1, 0, 0)), ((0, 0, 1), (0, 0, -1))],
    ids=["north", "east", "down"],
)
def test_ned_axes_in_world(kind, ned, enu):
    repo = Path(__file__).resolve().parents[3]
    launch = repo / "models" / f"posim_{kind}_models" / "launch" / f"upload_{kind}.launch.py"
    tree = ast.parse(launch.read_text())
    publisher = next(
        node.value
        for node in ast.walk(tree)
        if isinstance(node, ast.Assign)
        and any(
            isinstance(target, ast.Name) and target.id == "tf2_spawner" for target in node.targets
        )
    )
    arguments = ast.literal_eval(next(k.value for k in publisher.keywords if k.arg == "arguments"))
    options = dict(zip(arguments[::2], arguments[1::2]))
    assert options["--frame-id"] == "world"
    assert options["--child-frame-id"] == "world_ned"
    roll = float(options.get("--roll", 0))
    pitch = float(options.get("--pitch", 0))
    yaw = float(options.get("--yaw", 0))

    # Rotate the NED basis into its parent ENU frame, in fixed-axis RPY order.
    x, y, z = ned
    y, z = math.cos(roll) * y - math.sin(roll) * z, math.sin(roll) * y + math.cos(roll) * z
    x, z = math.cos(pitch) * x + math.sin(pitch) * z, -math.sin(pitch) * x + math.cos(pitch) * z
    x, y = math.cos(yaw) * x - math.sin(yaw) * y, math.sin(yaw) * x + math.cos(yaw) * y
    assert (x, y, z) == pytest.approx(enu, abs=1e-12)
