"""A debugger must not corrupt the version probe required by gz's Ruby CLI."""

import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

path = Path(__file__).resolve().parents[2] / "tools/instrument-gazebo-gdb.py"
spec = importlib.util.spec_from_file_location("gazebo_gdb", path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class GazeboDiagnosticWrapperTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.binary = self.root / "gz-sim-main"
        self.binary.write_text('#!/bin/bash\nprintf "10.5.0\\n"\n')
        self.binary.chmod(0o755)
        debugger = self.root / "gdb"
        debugger.write_text('#!/bin/bash\nprintf "debugger:%s\\n" "$@"\n')
        debugger.chmod(0o755)
        self.env = {**os.environ, "PATH": str(self.root) + os.pathsep + os.environ["PATH"]}

    def run_binary(self, *args):
        return subprocess.check_output([str(self.binary), *args], env=self.env, text=True)

    def test_version_stdout_is_byte_identical(self):
        before = self.run_binary("--version")
        module.instrument(self.binary)
        self.assertEqual(self.run_binary("--version"), before)

    def test_simulation_still_runs_under_debugger(self):
        module.instrument(self.binary)
        output = self.run_binary("-s", "world with spaces.sdf")
        self.assertIn("debugger:set debuginfod enabled off\n", output)
        self.assertIn("debugger:" + str(self.binary) + ".real\n", output)
        self.assertIn("debugger:world with spaces.sdf\n", output)

    def test_refuses_double_instrumentation(self):
        module.instrument(self.binary)
        before = self.binary.read_bytes()
        with self.assertRaises(RuntimeError):
            module.instrument(self.binary)
        self.assertEqual(before, self.binary.read_bytes())


if __name__ == "__main__":
    unittest.main()
