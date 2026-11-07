#!/usr/bin/env python3
"""Test the ARM64 launcher contract without Qt, GStreamer or a running GUI.

Run from the checkout with:
    python3 .docker/tests/test_qgroundcontrol_arm64.py

Set POSIM_QGC_WRAPPER to check an installed candidate launcher instead.
Real bundled-plugin and video checks are separate integration tests.
"""

import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import unittest


WRAPPER = Path(
    os.environ.get(
        "POSIM_QGC_WRAPPER",
        str(Path(__file__).resolve().parent.parent / "qgroundcontrol-arm64.sh"),
    )
)
GST_KEYS = (
    "GST_PLUGIN_PATH",
    "GST_PLUGIN_PATH_1_0",
    "GST_PLUGIN_SYSTEM_PATH",
    "GST_PLUGIN_SYSTEM_PATH_1_0",
    "GST_PLUGIN_SCANNER",
    "GST_PLUGIN_SCANNER_1_0",
    "GST_REGISTRY",
    "GST_REGISTRY_1_0",
)


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "requires POSIX and bash")
class QgcArm64LauncherTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="posim-qgc-test-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.home = self.root / "home with spaces"
        self.app = self.home / "QGC"
        self.plugins = self.app / "usr/lib/gstreamer-1.0"
        self.scanner = self.app / "usr/lib/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner"
        self.plugins.mkdir(parents=True)
        self.scanner.parent.mkdir(parents=True)
        self.scanner.write_text("#!/bin/sh\nexit 0\n")
        self.scanner.chmod(0o755)
        self.app_run = self.app / "AppRun"
        self.app_run.write_text(
            "#!/usr/bin/env python3\n"
            "import json, os, sys\n"
            "print(json.dumps({'argv': sys.argv[1:], 'env': dict(os.environ)}))\n"
            "sys.exit(int(os.environ.get('TEST_EXIT', '0')))\n"
        )
        self.app_run.chmod(0o755)
        self.cache = self.root / "cache with spaces"
        self.env = dict(os.environ, HOME=str(self.home), XDG_CACHE_HOME=str(self.cache))
        self.env.update({key: "/wrong-system-value" for key in GST_KEYS})
        self.env.update(APPDIR="/wrong-bundle", TEST_EXIT="0")

    def launch(self, *args, **overrides):
        return subprocess.run(
            ["bash", str(WRAPPER), *args],
            env=dict(self.env, **overrides),
            capture_output=True,
            text=True,
            timeout=10,
            check=False,
        )

    def test_arguments_and_bundle_location(self):
        result = self.launch("a b", "--option=quoted value")
        self.assertEqual(result.returncode, 0, result.stderr)
        output = json.loads(result.stdout)
        self.assertEqual(output["argv"], ["a b", "--option=quoted value"])
        self.assertEqual(output["env"]["APPDIR"], str(self.app))

    def test_both_gstreamer_variable_variants_are_isolated(self):
        result = self.launch()
        self.assertEqual(result.returncode, 0, result.stderr)
        env = json.loads(result.stdout)["env"]
        for suffix in ("", "_1_0"):
            self.assertEqual(env["GST_PLUGIN_PATH" + suffix], str(self.plugins))
            self.assertEqual(env["GST_PLUGIN_SYSTEM_PATH" + suffix], "")
            self.assertEqual(env["GST_PLUGIN_SCANNER" + suffix], str(self.scanner))
        self.assertEqual(env["GST_REGISTRY"], env["GST_REGISTRY_1_0"])

    def test_private_cache_preserves_shared_cache_and_parent_environment(self):
        shared = self.cache / "gstreamer-1.0/registry.fixture.bin"
        shared.parent.mkdir(parents=True)
        shared.write_bytes(b"preserve this shared registry")
        before = dict(os.environ)
        result = self.launch()
        self.assertEqual(result.returncode, 0, result.stderr)
        env = json.loads(result.stdout)["env"]
        private = self.cache / "posim-qgc/gstreamer-1.0"
        self.assertEqual(Path(env["GST_REGISTRY"]).parent, private)
        self.assertEqual(stat.S_IMODE(private.stat().st_mode), 0o700)
        self.assertEqual(shared.read_bytes(), b"preserve this shared registry")
        self.assertEqual(dict(os.environ), before)

    def test_application_exit_code_is_preserved(self):
        self.assertEqual(self.launch(TEST_EXIT="37").returncode, 37)

    def test_relative_xdg_cache_falls_back_to_home(self):
        result = self.launch(XDG_CACHE_HOME="relative-invalid")
        self.assertEqual(result.returncode, 0, result.stderr)
        registry = Path(json.loads(result.stdout)["env"]["GST_REGISTRY"])
        self.assertEqual(registry.parent, self.home / ".cache/posim-qgc/gstreamer-1.0")

    def test_incomplete_bundle_fails_without_launch(self):
        self.scanner.unlink()
        result = self.launch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("bundle is incomplete", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_unusable_cache_fails_without_launch(self):
        blocked = self.root / "cache-is-a-file"
        blocked.write_text("not a directory")
        result = self.launch(XDG_CACHE_HOME=str(blocked))
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "")

    def test_existing_unwritable_cache_fails_without_launch(self):
        if os.geteuid() == 0:
            self.skipTest("root can write to a read-only directory")
        private = self.cache / "posim-qgc/gstreamer-1.0"
        private.mkdir(parents=True)
        private.chmod(0o500)
        self.addCleanup(private.chmod, 0o700)
        result = self.launch()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("cache directory is not writable", result.stderr)
        self.assertEqual(result.stdout, "")


if __name__ == "__main__":
    unittest.main(verbosity=2)
