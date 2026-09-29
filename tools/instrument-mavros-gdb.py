"""Instrument only a disposable diagnostic container, never the production image."""

from pathlib import Path
import shlex

from ament_index_python.packages import get_package_share_directory

root = Path(get_package_share_directory("dave_robot_models"))
args = ["gdb", "-q", "-batch", "--return-child-result"]
for command in (
    "set pagination off",
    "set confirm off",
    "handle SIGINT nostop noprint pass",
    "handle SIGPIPE nostop noprint pass",
    "run",
    "thread apply all bt",
):
    args.extend(["-ex", command])
args.append("--args")
prefix = shlex.join(args)
for vehicle in ("bluerov2", "bluerov2_heavy"):
    path = root / "config" / vehicle / "robot_config.py"
    text = path.read_text()
    needle = 'executable="mavros_node",'
    if text.count(needle) != 1:
        raise RuntimeError(f"Expected one MAVROS node in {path}")
    path.write_text(text.replace(needle, needle + "\n        prefix=" + repr(prefix) + ","))
