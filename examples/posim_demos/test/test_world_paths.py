"""Exercise world-path quoting at both launch entry points."""

from contextlib import ExitStack
from importlib.util import module_from_spec, spec_from_file_location
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
from types import SimpleNamespace
from unittest.mock import patch

from launch import LaunchContext
from launch.actions import DeclareLaunchArgument
from launch.utilities import normalize_to_list_of_substitutions, perform_substitutions
import pytest


def arguments(entry, share, platform_name="posix", empty=False, headless=True, paused=False):
    path = Path(__file__).resolve().parents[1] / "launch" / f"posim_{entry}.launch.py"
    spec = spec_from_file_location("launch_under_test", path)
    module = module_from_spec(spec)
    spec.loader.exec_module(module)
    module.os = SimpleNamespace(**{**os.__dict__, "name": platform_name})
    context = LaunchContext()
    for action in module.generate_launch_description().entities:
        if isinstance(action, DeclareLaunchArgument) and action.default_value is not None:
            context.launch_configurations[action.name] = perform_substitutions(
                context, action.default_value
            )
    context.launch_configurations.update(
        world_name="empty.sdf" if empty else "scene",
        headless=str(headless).lower(),
        gui=str(not headless).lower(),
        paused=str(paused).lower(),
        verbose="4" if entry == "robot" else "true",
    )
    with ExitStack() as stack:
        if entry == "robot":
            stack.enter_context(
                patch.object(module.FindPackageShare, "find", lambda self, package: str(share))
            )
        else:
            stack.enter_context(
                patch.object(module, "get_package_share_directory", return_value=str(share))
            )
        actions = module.launch_setup(context)
        include = next(a for a in actions if "gz_args" in dict(a.launch_arguments))
        value = dict(include.launch_arguments)["gz_args"]
        return perform_substitutions(context, normalize_to_list_of_substitutions(value))


@pytest.fixture
def share(tmp_path):
    path = tmp_path / "plain"
    (path / "worlds").mkdir(parents=True)
    (path / "worlds/scene.world").write_text('<sdf><world name="scene"/></sdf>')
    return path


@pytest.mark.skipif(os.name != "posix", reason="Requires a POSIX shell")
@pytest.mark.parametrize("entry", ["world", "robot"])
@pytest.mark.parametrize(
    "directory",
    [
        "plain",
        "space path",
        "author's files",
        "semi;colon",
        "literal$HOME",
        'double"quote',
        "해양 연구",
    ],
)
def test_world_remains_one_literal_shell_argument(tmp_path, entry, directory):
    share = tmp_path / directory
    (share / "worlds").mkdir(parents=True)
    world = share / "worlds/scene.world"
    world.write_text('<sdf><world name="scene"/></sdf>')
    prefix = (
        shlex.quote(sys.executable)
        + " -c "
        + shlex.quote("import json,sys; print(json.dumps(sys.argv[1:]))")
    )
    result = subprocess.run(
        prefix + " " + arguments(entry, share),
        shell=True,
        executable="/bin/sh",
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert result.returncode == 0, result.stderr
    argv = json.loads(result.stdout)
    assert argv.count(str(world)) == 1
    assert "-r" in argv
    assert "-s" in argv


@pytest.mark.parametrize("entry", ["world", "robot"])
def test_windows_keeps_unquoted_command_construction(share, entry):
    # Deliberately preserve Windows behavior; not a Windows execution test.
    spaced = share.with_name("Program Files")
    share.rename(spaced)
    assert str(spaced / "worlds/scene.world") in arguments(entry, spaced, "nt")
    assert "'" not in arguments(entry, spaced, "nt")


@pytest.mark.parametrize("headless", [False, True])
@pytest.mark.parametrize("paused", [False, True])
def test_robot_empty_world_preserves_flags(share, headless, paused):
    argv = shlex.split(arguments("robot", share, empty=True, headless=headless, paused=paused))
    assert argv.count("empty.sdf") == 1
    assert ("-r" in argv) == (not paused)
    assert ("-s" in argv) == headless
