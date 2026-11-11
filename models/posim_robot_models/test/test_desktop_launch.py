"""Keep desktop account and display handling in the installed app wrappers."""

import ast
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import unittest


CONFIG = Path(__file__).resolve().parents[1] / "config"
VARIANTS = ("bluerov2", "bluerov2_heavy", "bluerov2_heavy_multibeam_sonar")


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "requires POSIX and bash")
class DesktopLaunchTest(unittest.TestCase):
    def test_qgc_preserves_the_callers_desktop_context(self):
        self.check_launch("qgc_cmd", "qgroundcontrol", [])

    def test_firefox_preserves_the_callers_desktop_context_and_url(self):
        self.check_launch(
            "joystick_cmd", "firefox", ["--new-window", "http://localhost:8080/?x=1&y=2"]
        )

    def check_launch(self, command_name, executable, arguments):
        for variant in VARIANTS:
            tree = ast.parse((CONFIG / variant / "robot_config.py").read_text())
            command = next(
                node.value
                for node in ast.walk(tree)
                if isinstance(node, ast.Assign)
                and any(
                    isinstance(target, ast.Name) and target.id == command_name
                    for target in node.targets
                )
            )
            command = eval(
                compile(ast.Expression(command), "<desktop launch command>", "eval"),
                {"ui_launch_delay": "0", "virtual_joystick_url": "http://localhost:8080/?x=1&y=2"},
            )
            for caller in ("0", "1001"):
                with self.subTest(
                    variant=variant, caller=caller
                ), tempfile.TemporaryDirectory() as temp:
                    work = Path(temp)
                    output = work / "wrapper.json"
                    scripts = {
                        "id": '#!/bin/sh\nprintf "%s\\n" "$TEST_CALLER"\n',
                        "sudo": '#!/bin/sh\necho "Unexpected account switch" >&2\nexit 97\n',
                        executable: (
                            f"#!{sys.executable}\nimport json,os,sys\n"
                            "with open(os.environ['TEST_OUTPUT'] + '.tmp', 'w') as out:\n"
                            "    data = {k: os.environ.get(k) for k in "
                            "('HOME', 'DISPLAY', 'XAUTHORITY', 'TEST_CALLER')}\n"
                            "    data['argv'] = sys.argv[1:]\n"
                            "    json.dump(data, out)\n"
                            "os.replace(os.environ['TEST_OUTPUT'] + '.tmp', os.environ['TEST_OUTPUT'])\n"
                        ),
                    }
                    for name, contents in scripts.items():
                        path = work / name
                        path.write_text(contents)
                        path.chmod(0o755)
                    context = {
                        "HOME": str(work / "caller home"),
                        "DISPLAY": ":42",
                        "XAUTHORITY": str(work / "private display cookie"),
                        "TEST_CALLER": caller,
                    }
                    env = dict(
                        os.environ,
                        **context,
                        TEST_OUTPUT=str(output),
                        PATH=str(work) + os.pathsep + os.environ["PATH"],
                    )
                    process = subprocess.Popen(
                        ["bash", "-c", command],
                        env=env,
                        start_new_session=True,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        text=True,
                    )
                    try:
                        deadline = time.monotonic() + 2
                        while not output.exists() and time.monotonic() < deadline:
                            time.sleep(0.02)
                    finally:
                        try:
                            _, error = process.communicate(timeout=0.2)
                        except subprocess.TimeoutExpired:
                            os.killpg(process.pid, signal.SIGTERM)
                            _, error = process.communicate(timeout=3)
                    self.assertTrue(output.exists(), error)
                    self.assertEqual(json.loads(output.read_text()), dict(context, argv=arguments))


if __name__ == "__main__":
    unittest.main()
