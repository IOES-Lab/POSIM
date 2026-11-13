#!/usr/bin/env python3
"""Offline tests for the ARM64 xrdp environment restoration hook."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

DOCKER = Path(__file__).resolve().parents[1]
PATHS = {
    "GZ_SIM_RESOURCE_PATH": [
        "/opt/asv_wave_sim/gz-waves-models/models",
        "/opt/asv_wave_sim/gz-waves-models/world_models",
    ],
    "GZ_SIM_SYSTEM_PLUGIN_PATH": ["/opt/waves/lib"],
    "LD_LIBRARY_PATH": ["/opt/posim/rendering-compat/lib", "/opt/waves/lib"],
    "GZ_RENDERING_PLUGIN_PATH": ["/opt/posim/rendering-compat/lib"],
    "GZ_RENDERING_RESOURCE_PATH": ["/opt/posim/terra"],
}


class RuntimeEnvironmentTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        # Redirect only absolute installation paths; never create host /opt files.
        text = (DOCKER / "rdp-runtime-env.sh").read_text()
        self.script = self.root / "env.sh"
        self.script.write_text(text.replace("/opt/", str(self.root / "opt") + "/"))
        for paths in PATHS.values():
            for path in paths:
                self.fixture(path).mkdir(parents=True, exist_ok=True)

    def fixture(self, path):
        return self.root / path.lstrip("/")

    def run_env(self, initial=None, repetitions=1):
        env = {"PATH": os.environ["PATH"]}
        env.update(initial or {})
        command = "; ".join(['source "$1"'] * repetitions)
        command += "; env; if declare -F _posim_prepend_existing; then exit 9; fi"
        run = subprocess.run(
            ["bash", "-eu", "-c", command, "test", str(self.script)],
            env=env,
            text=True,
            capture_output=True,
            check=True,
        )
        return dict(line.split("=", 1) for line in run.stdout.splitlines() if "=" in line)

    def test_clean_environment_exports_all_existing_paths(self):
        env = self.run_env()
        for name, paths in PATHS.items():
            self.assertEqual(env[name].split(":"), [str(self.fixture(p)) for p in paths])

    def test_custom_paths_are_preserved_in_order(self):
        custom = {
            name: "/custom/one:/custom/two"
            for name in PATHS
            if name != "GZ_RENDERING_RESOURCE_PATH"
        }
        env = self.run_env(custom)
        for name in custom:
            self.assertTrue(env[name].endswith(":/custom/one:/custom/two"))

    def test_explicit_rendering_overlay_is_not_a_search_path(self):
        env = self.run_env({"GZ_RENDERING_RESOURCE_PATH": "/custom/render"})
        self.assertEqual(env["GZ_RENDERING_RESOURCE_PATH"], "/custom/render")

    def test_repeated_source_is_idempotent(self):
        first = self.run_env()
        second = self.run_env({name: first[name] for name in PATHS}, repetitions=2)
        self.assertEqual(
            {name: first[name] for name in PATHS}, {name: second[name] for name in PATHS}
        )

    def test_existing_entry_is_not_duplicated(self):
        path = str(self.fixture("/opt/waves/lib"))
        env = self.run_env({"LD_LIBRARY_PATH": "/custom:" + path})
        self.assertEqual(env["LD_LIBRARY_PATH"].split(":").count(path), 1)
        self.assertTrue(env["LD_LIBRARY_PATH"].endswith("/custom:" + path))

    def test_missing_installation_is_not_exported(self):
        self.fixture("/opt/posim/terra").rmdir()
        env = self.run_env()
        self.assertNotIn("GZ_RENDERING_RESOURCE_PATH", env)

    def test_missing_installation_keeps_custom_value(self):
        self.fixture("/opt/posim/terra").rmdir()
        env = self.run_env({"GZ_RENDERING_RESOURCE_PATH": "/custom/render"})
        self.assertEqual(env["GZ_RENDERING_RESOURCE_PATH"], "/custom/render")

    def test_empty_values_do_not_add_empty_path_elements(self):
        env = self.run_env({name: "" for name in PATHS})
        for name in PATHS:
            self.assertNotIn("", env[name].split(":"))

    def test_arm64_hook_runs_before_desktop(self):
        text = (DOCKER / "lyrical.arm64v8.dockerfile").read_text()
        self.assertIn("COPY .docker/rdp-runtime-env.sh /usr/local/share/posim/", text)
        self.assertLess(
            text.index("'source /usr/local/share/posim/rdp-runtime-env.sh'"),
            text.index("'exec /usr/bin/dbus-run-session -- /usr/bin/startxfce4'"),
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
