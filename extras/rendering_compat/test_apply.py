#!/usr/bin/env python3
"""Offline safety tests; no installed simulator or network is required."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
loader = importlib.util.spec_from_file_location("dynamic_mesh_apply", HERE / "apply.py")
apply = importlib.util.module_from_spec(loader)
loader.loader.exec_module(apply)


class PatchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.target = self.root / "src/mesh.cc"
        self.target.parent.mkdir()
        self.before, self.after = b"old\n", b"new\n"
        self.target.write_bytes(self.before)
        self.target.chmod(0o640)
        self.patch = self.root / "change.patch"
        self.patch.write_text("--- a/src/mesh.cc\n+++ b/src/mesh.cc\n@@ -1 +1 @@\n-old\n+new\n")
        self.spec = dict(
            path="src/mesh.cc",
            patch="change.patch",
            before_sha256=apply.sha(self.before),
            after_sha256=apply.sha(self.after),
            patch_sha256=apply.sha(self.patch.read_bytes()),
        )

    def prepare(self, spec=None):
        return apply.prepare(self.root, spec or self.spec, self.root)

    def test_check_does_not_write(self):
        self.assertEqual(self.prepare()[2], self.after)
        self.assertEqual(self.target.read_bytes(), self.before)

    def test_apply_preserves_mode(self):
        apply.apply_all([self.prepare()])
        self.assertEqual(self.target.read_bytes(), self.after)
        self.assertEqual(self.target.stat().st_mode & 0o777, 0o640)

    def test_repeated_application_is_noop(self):
        apply.apply_all([self.prepare()])
        stamp = self.target.stat().st_mtime_ns
        apply.apply_all([self.prepare()])
        self.assertEqual(self.target.stat().st_mtime_ns, stamp)

    def test_unknown_source_rejected(self):
        self.target.write_bytes(b"unrelated\n")
        with self.assertRaises(ValueError):
            self.prepare()
        self.assertEqual(self.target.read_bytes(), b"unrelated\n")

    def test_tampered_patch_rejected(self):
        self.patch.write_text("invalid patch")
        with self.assertRaises(ValueError):
            self.prepare()
        self.assertEqual(self.target.read_bytes(), self.before)

    def test_wrong_expected_output_rejected(self):
        with self.assertRaises(ValueError):
            self.prepare(dict(self.spec, after_sha256="0" * 64))
        self.assertEqual(self.target.read_bytes(), self.before)

    def test_concurrent_edit_rejected(self):
        plan = self.prepare()
        self.target.write_bytes(b"edited\n")
        with self.assertRaises(ValueError):
            apply.apply_all([plan])
        self.assertEqual(self.target.read_bytes(), b"edited\n")

    def test_path_escape_rejected(self):
        with self.assertRaises(ValueError):
            self.prepare(dict(self.spec, path="../outside"))

    def test_symlink_rejected(self):
        other = self.root / "other.cc"
        other.write_bytes(self.before)
        self.target.unlink()
        self.target.symlink_to(other)
        with self.assertRaises(ValueError):
            self.prepare()
        self.assertEqual(other.read_bytes(), self.before)

    def test_second_component_failure_leaves_first_unchanged(self):
        other = self.root / "second"
        (other / "src").mkdir(parents=True)
        (other / "src/mesh.cc").write_bytes(b"unreviewed\n")
        with self.assertRaises(ValueError):
            plans = [self.prepare(), apply.prepare(other, self.spec, self.root)]
            apply.apply_all(plans)
        self.assertEqual(self.target.read_bytes(), self.before)
        self.assertEqual((other / "src/mesh.cc").read_bytes(), b"unreviewed\n")


if __name__ == "__main__":
    unittest.main(verbosity=2)
