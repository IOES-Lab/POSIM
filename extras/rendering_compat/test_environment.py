#!/usr/bin/env python3
"""Verify renderer selection after ROS underlays reorder inherited paths."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

HOOK = Path(__file__).resolve().parents[2] / "gazebo/posim_gz_rendering/rendering_compat.sh"


class RenderingEnvironmentTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.prefix = str(self.root / "renderer lib")
        Path(self.prefix).mkdir()
        self.hook = self.root / "env.sh"
        self.hook.write_text(
            HOOK.read_text().replace("/opt/posim/rendering-compat/lib", '"' + self.prefix + '"')
        )

    def run_hook(self, value, repetitions=1):
        env = {
            "PATH": os.environ["PATH"],
            "LD_LIBRARY_PATH": value,
            "GZ_RENDERING_PLUGIN_PATH": value,
        }
        command = "; ".join(['. "$1"'] * repetitions)
        command += '; printf "%s\\n%s\\n" "$LD_LIBRARY_PATH" "$GZ_RENDERING_PLUGIN_PATH"'
        result = subprocess.run(
            ["sh", "-eu", "-c", command, "test", str(self.hook)],
            env=env,
            text=True,
            capture_output=True,
            check=True,
        )
        return result.stdout.splitlines()

    def test_inherited_overlay_moves_ahead_of_ros_vendor(self):
        value = "/ros/vendor:" + self.prefix + ":/custom"
        self.assertEqual(self.run_hook(value), [self.prefix + ":/ros/vendor:/custom"] * 2)

    def test_repeated_source_deduplicates_and_preserves_custom_order(self):
        value = "/custom one:" + self.prefix + ":/custom/two:" + self.prefix
        self.assertEqual(self.run_hook(value, 3), [self.prefix + ":/custom one:/custom/two"] * 2)

    def test_empty_paths_select_installed_renderer(self):
        self.assertEqual(self.run_hook(""), [self.prefix] * 2)

    def test_missing_overlay_keeps_existing_paths(self):
        Path(self.prefix).rmdir()
        self.assertEqual(self.run_hook("/custom"), ["/custom"] * 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
