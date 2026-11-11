#!/usr/bin/env python3
"""Offline safety tests; real pinned sources are additionally checked in image QA."""
import difflib
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest

MODULE = Path(__file__).with_name("apply.py")
loader = importlib.util.spec_from_file_location("backport", MODULE)
bp = importlib.util.module_from_spec(loader)
loader.loader.exec_module(bp)


class BackportTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.bundle = self.root / "bundle"
        self.bundle.mkdir()
        self.source = self.root / "source"
        self.source.mkdir()
        self.output = self.root / "overlay"
        self.a, self.b, self.c = b"original\n", b"existing compatibility fix\n", b"final\n"
        self.spec = {
            "path": "test.txt",
            "upstream_pr": "test",
            "upstream_head": "test",
            "steps": [
                self.step("first.patch", self.a, self.b),
                self.step("second.patch", self.b, self.c),
            ],
        }
        (self.source / "test.txt").write_bytes(self.a)
        (self.source / "unrelated.txt").write_bytes(b"preserve this resource\n")
        self.manifest = {"schema": 1, "gst": self.spec, "terrain": self.spec}
        self.save_manifest()

    def step(self, name, before, after):
        data = "".join(
            difflib.unified_diff(
                before.decode().splitlines(True),
                after.decode().splitlines(True),
                fromfile="a/test.txt",
                tofile="b/test.txt",
            )
        ).encode()
        (self.bundle / name).write_bytes(data)
        return {
            "patch": name,
            "patch_sha256": bp.digest(data),
            "before": bp.digest(before),
            "after": bp.digest(after),
        }

    def save_manifest(self):
        (self.bundle / "manifest.json").write_text(json.dumps(self.manifest))

    def test_bundle_integrity(self):
        self.assertEqual(bp.load_bundle(self.bundle), self.manifest)
        (self.bundle / "first.patch").write_text("tampered")
        with self.assertRaises(ValueError):
            bp.load_bundle(self.bundle)

    def test_apply_original_and_repeat(self):
        self.assertTrue(bp.apply_gst(self.source, self.spec, self.bundle)["changed"])
        self.assertEqual((self.source / "test.txt").read_bytes(), self.c)
        self.assertFalse(bp.apply_gst(self.source, self.spec, self.bundle)["changed"])

    def test_patch_updates_mtime_for_incremental_builds(self):
        target = self.source / "test.txt"
        target.chmod(0o640)
        os.utime(target, (1000000000, 1000000000))
        bp.apply_gst(self.source, self.spec, self.bundle)
        self.assertGreater(target.stat().st_mtime, 1000000000)
        self.assertEqual(target.stat().st_mode & 0o777, 0o640)

    def test_existing_compatibility_fix(self):
        (self.source / "test.txt").write_bytes(self.b)
        bp.apply_gst(self.source, self.spec, self.bundle)
        self.assertEqual((self.source / "test.txt").read_bytes(), self.c)

    def test_unknown_source_untouched(self):
        target = self.source / "test.txt"
        target.write_bytes(b"custom user source\n")
        with self.assertRaises(ValueError):
            bp.apply_gst(self.source, self.spec, self.bundle)
        self.assertEqual(target.read_bytes(), b"custom user source\n")

    def test_wrong_result_hash_untouched(self):
        self.spec["steps"][-1]["after"] = "wrong"
        with self.assertRaises(ValueError):
            bp.apply_gst(self.source, self.spec, self.bundle)
        self.assertEqual((self.source / "test.txt").read_bytes(), self.a)

    def test_broken_chain_untouched(self):
        self.spec["steps"][1]["before"] = "wrong"
        with self.assertRaises(ValueError):
            bp.apply_gst(self.source, self.spec, self.bundle)
        self.assertEqual((self.source / "test.txt").read_bytes(), self.a)

    def test_symlinked_source_rejected(self):
        target = self.source / "test.txt"
        target.unlink()
        target.symlink_to(self.source / "unrelated.txt")
        with self.assertRaises(ValueError):
            bp.apply_gst(self.source, self.spec, self.bundle)

    def test_terrain_preservation_and_repeat(self):
        before = bp.inventory(self.source)
        self.assertTrue(
            bp.apply_terrain(self.source, self.output, self.spec, self.bundle)["changed"]
        )
        self.assertEqual(bp.inventory(self.source), before)
        self.assertEqual((self.output / "test.txt").read_bytes(), self.c)
        self.assertEqual(
            (self.output / "unrelated.txt").read_bytes(),
            (self.source / "unrelated.txt").read_bytes(),
        )
        self.assertFalse(
            bp.apply_terrain(self.source, self.output, self.spec, self.bundle)["changed"]
        )

    def test_already_fixed_terrain(self):
        (self.source / "test.txt").write_bytes(self.c)
        result = bp.apply_terrain(self.source, self.output, self.spec, self.bundle)
        self.assertEqual(result["changed_resources"], [])

    def test_custom_overlay_not_overwritten(self):
        self.output.mkdir()
        (self.output / "private.txt").write_text("keep")
        with self.assertRaises(ValueError):
            bp.apply_terrain(self.source, self.output, self.spec, self.bundle)
        self.assertEqual((self.output / "private.txt").read_text(), "keep")

    def test_modified_overlay_not_overwritten(self):
        bp.apply_terrain(self.source, self.output, self.spec, self.bundle)
        (self.output / "test.txt").write_text("user edit")
        with self.assertRaises(ValueError):
            bp.apply_terrain(self.source, self.output, self.spec, self.bundle)
        self.assertEqual((self.output / "test.txt").read_text(), "user edit")

    def test_unknown_terrain_no_partial_output(self):
        (self.source / "test.txt").write_text("new upstream shader")
        with self.assertRaises(ValueError):
            bp.apply_terrain(self.source, self.output, self.spec, self.bundle)
        self.assertFalse(self.output.exists())

    def test_overlap_rejected(self):
        for output in (self.source, self.source / "child", self.root):
            with self.assertRaises(ValueError):
                bp.apply_terrain(self.source, output, self.spec, self.bundle)

    def test_invalid_manifest_paths(self):
        self.spec["path"] = "../outside"
        self.save_manifest()
        with self.assertRaises(ValueError):
            bp.load_bundle(self.bundle)

    def test_shipped_patch_checksums(self):
        bp.load_bundle(MODULE.parent)


if __name__ == "__main__":
    unittest.main()
