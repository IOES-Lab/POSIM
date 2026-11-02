"""Safety tests; these do not build or load Ogre."""

import contextlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / (name + ".py"))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


RUN = module("run_overlay")
BUILD = module("build_overlay")


class OverlayTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.engine = self.root / "engine"
        self.overlay = self.root / "overlay"
        self.lib = self.overlay / "lib/libOgreNextMain.2.3.3.dylib"
        installed = self.engine / "lib/libOgreNextMain.2.3.3.dylib"
        header = self.engine / "include/OGRE-Next/OgreBuildSettings.h"
        for path, text in [
            (self.lib, b"patched"),
            (installed, b"original"),
            (header, b"settings"),
        ]:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(text)
        self.installed = installed
        self.manifest = {
            "engine_prefix": str(self.engine),
            "library": str(self.lib),
            "installed_sha256": RUN.digest(installed),
            "library_sha256": RUN.digest(self.lib),
            "build_settings_sha256": RUN.digest(header),
        }
        (self.overlay / "manifest.json").write_text(json.dumps(self.manifest))

    def invoke(self):
        with patch("sys.argv", ["run_overlay", "--overlay", str(self.overlay), "--", "probe"]):
            with patch.object(RUN.platform, "system", return_value="Darwin"):
                with patch.object(RUN.platform, "machine", return_value="arm64"):
                    RUN.main()

    def test_component_overlay_precedes_installed_engine(self):
        with patch.dict(
            "os.environ", {"DYLD_LIBRARY_PATH": "/candidate", "DYLD_INSERT_LIBRARIES": "/bad"}
        ):
            with patch.object(RUN.os, "execvpe") as execute:
                self.invoke()
                env = execute.call_args.args[2]
                self.assertEqual(
                    env["DYLD_LIBRARY_PATH"].split(":"),
                    [str(self.lib.parent), "/candidate", str(self.engine / "lib")],
                )
                self.assertNotIn("DYLD_INSERT_LIBRARIES", env)
        self.assertEqual(self.installed.read_bytes(), b"original")

    def test_empty_search_path_does_not_inject_current_directory(self):
        with patch.dict("os.environ", {"DYLD_LIBRARY_PATH": ""}):
            with patch.object(RUN.os, "execvpe") as execute:
                self.invoke()
                self.assertNotIn("", execute.call_args.args[2]["DYLD_LIBRARY_PATH"].split(":"))

    def test_corrupt_overlay_refused(self):
        self.lib.write_bytes(b"different")
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.invoke()

    def test_changed_installed_library_refused(self):
        self.installed.write_bytes(b"new-installed-version")
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.invoke()

    def test_changed_configuration_refused(self):
        (self.engine / "include/OGRE-Next/OgreBuildSettings.h").write_bytes(b"new-settings")
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.invoke()

    def test_build_refuses_existing_destination(self):
        argv = [
            "build_overlay",
            "--engine-prefix",
            str(self.engine),
            "--source",
            str(self.root / "source"),
            "--output",
            str(self.overlay),
        ]
        with patch("sys.argv", argv), patch.object(
            BUILD.platform, "system", return_value="Darwin"
        ), patch.object(BUILD.platform, "machine", return_value="arm64"):
            with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                BUILD.main()
        self.assertEqual(self.installed.read_bytes(), b"original")

    def test_linux_build_is_not_silently_claimed_supported(self):
        argv = [
            "build_overlay",
            "--engine-prefix",
            str(self.engine),
            "--source",
            str(self.root / "source"),
            "--output",
            str(self.root / "new"),
        ]
        with patch("sys.argv", argv), patch.object(BUILD.platform, "system", return_value="Linux"):
            with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                BUILD.main()
        self.assertFalse((self.root / "new").exists())


if __name__ == "__main__":
    unittest.main()
