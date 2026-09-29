"""Wrap only the disposable diagnostic container's native Gazebo executable."""

from pathlib import Path
import shlex


def instrument(binary):
    real = binary.with_name(binary.name + ".real")
    if real.exists():
        raise RuntimeError("Refusing to wrap an already instrumented executable")
    binary.rename(real)
    # Disable automatic network symbol downloads BEFORE loading the executable.
    # Fetch the one suspect library's symbols separately with a bounded timeout.
    args = [
        "gdb",
        "-q",
        "-batch",
        "--return-child-result",
        "-iex",
        "set debuginfod enabled off",
    ]
    for command in (
        "set pagination off",
        "set confirm off",
        "handle SIGINT nostop noprint pass",
        "handle SIGPIPE nostop noprint pass",
        "run",
        "info sharedlibrary",
        "thread apply all bt full",
    ):
        args.extend(["-ex", command])
    args.extend(["--args", str(real)])
    # cmdsim.rb requires stdout from --version to match its version exactly.
    # GDB chatter in that probe prevents the server from ever being launched.
    binary.write_text(
        '#!/bin/bash\nif [[ "$#" == 1 && "$1" == --version ]]; then\n'
        + "  exec "
        + shlex.quote(str(real))
        + ' "$@"\nfi\n'
        + "exec "
        + shlex.join(args)
        + ' "$@"\n'
    )
    binary.chmod(0o755)


if __name__ == "__main__":
    matches = list(Path("/opt/ros/lyrical/opt/gz_sim_vendor/libexec/gz").glob("sim*/gz-sim-main"))
    if len(matches) != 1:
        raise RuntimeError(f"Expected one native Gazebo executable, got {matches}")
    instrument(matches[0])
