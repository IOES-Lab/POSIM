"""Wrap only the disposable diagnostic container's native Gazebo executable."""

from pathlib import Path
import shlex

matches = list(Path("/opt/ros/lyrical/opt/gz_sim_vendor/libexec/gz").glob("sim*/gz-sim-main"))
if len(matches) != 1:
    raise RuntimeError(f"Expected one native Gazebo executable, got {matches}")
binary = matches[0]
real = binary.with_name(binary.name + ".real")
if real.exists():
    raise RuntimeError("Refusing to wrap an already instrumented executable")
binary.rename(real)
args = ["gdb", "-q", "-batch", "--return-child-result"]
for command in (
    "set pagination off",
    "set confirm off",
    "set debuginfod enabled on",
    "handle SIGINT nostop noprint pass",
    "handle SIGPIPE nostop noprint pass",
    "run",
    "info sharedlibrary",
    "thread apply all bt full",
):
    args.extend(["-ex", command])
args.extend(["--args", str(real)])
binary.write_text("#!/bin/bash\nexec " + shlex.join(args) + ' "$@"\n')
binary.chmod(0o755)
