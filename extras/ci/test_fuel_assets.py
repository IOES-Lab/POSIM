"""A cache is complete only when locked files AND nested resources are present."""

import hashlib
import importlib.util
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / "prepare-image-assets.py"
SPEC = importlib.util.spec_from_file_location("fuel_assets", SCRIPT)
assets = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(assets)


class FuelAssetTests(unittest.TestCase):
    def setUp(self):
        self.tmp = TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.cache = Path(self.tmp.name)
        self.url = "https://fuel.gazebosim.org/1.0/Owner/models/Test/2"
        self.relative = str(assets.model_path(self.url))
        self.root = self.cache / self.relative
        self.root.mkdir(parents=True)
        (self.root / "model.sdf").write_text('<sdf version="1.9"><model name="test"/></sdf>')
        (self.root / "model.config").write_text("<model><name>test</name></model>")
        self.entry = {
            "url": self.url,
            "cache_path": self.relative,
            "license": {"url": "https://creativecommons.org/licenses/by/4.0/"},
            "files": assets.hashes(self.root),
        }

    def test_complete_cache(self):
        assets.verify_asset(self.cache, self.entry)
        assets.verify_dependencies(self.cache, [self.entry])

    def test_corrupt_file_rejected(self):
        (self.root / "model.sdf").write_text("changed")
        with self.assertRaisesRegex(RuntimeError, "integrity mismatch"):
            assets.verify_asset(self.cache, self.entry)

    def test_missing_and_extra_files_rejected(self):
        (self.root / "extra.txt").write_text("unexpected")
        with self.assertRaises(RuntimeError):
            assets.verify_asset(self.cache, self.entry)
        (self.root / "extra.txt").unlink()
        (self.root / "model.config").unlink()
        with self.assertRaises(RuntimeError):
            assets.verify_asset(self.cache, self.entry)

    def test_symlink_rejected(self):
        (self.root / "escape").symlink_to(self.cache)
        with self.assertRaisesRegex(RuntimeError, "symlink"):
            assets.verify_asset(self.cache, self.entry)

    def test_tip_version_and_unsafe_urls_rejected(self):
        for url in [
            self.url[:-1] + "tip",
            self.url.rsplit("/", 1)[0],
            self.url + "?x=1",
            self.url.replace("/Owner/", "/../"),
            self.url.replace("https:", "http:"),
        ]:
            with self.subTest(url=url), self.assertRaises(ValueError):
                assets.model_path(url)

    def test_wrong_cached_version_rejected(self):
        self.entry["cache_path"] = self.relative[:-1] + "3"
        with self.assertRaisesRegex(ValueError, "mismatch"):
            assets.verify_asset(self.cache, self.entry)

    def test_nested_dependency_requires_separate_pinned_model(self):
        (self.root / "model.sdf").write_text(
            "<sdf><model><link><visual><material><pbr><metal><albedo_map>"
            "https://fuel.gazebosim.org/1.0/Other/models/Texture/3/files/map.png"
            "</albedo_map></metal></pbr></material></visual></link></model></sdf>"
        )
        with self.assertRaisesRegex(RuntimeError, "Unpinned nested"):
            assets.verify_dependencies(self.cache, [self.entry])

    def test_nested_file_is_required_even_with_model_present(self):
        (self.root / "model.sdf").write_text(f"<sdf><uri>{self.url}/files/missing.png</uri></sdf>")
        with self.assertRaisesRegex(RuntimeError, "Missing nested"):
            assets.verify_dependencies(self.cache, [self.entry])

    def test_existing_cache_never_downloads(self):
        with patch.object(assets.subprocess, "run") as run:
            assets.prepare_asset(self.cache, self.entry)
            run.assert_not_called()

    def test_bad_existing_cache_is_not_silently_repaired(self):
        (self.root / "model.config").unlink()
        with patch.object(assets.subprocess, "run") as run, self.assertRaises(RuntimeError):
            assets.prepare_asset(self.cache, self.entry)
        run.assert_not_called()

    def test_lock_has_critical_version_three_dependency_and_valid_hashes(self):
        lock = json.loads((SCRIPT.parent / "fuel/quickstart-assets.lock.json").read_text())
        self.assertEqual(len(lock["assets"]), 12)
        paths = {a["cache_path"] for a in lock["assets"]}
        self.assertEqual(len(paths), 12)
        self.assertIn("fuel.ignitionrobotics.org/cole/models/sunken vase/3", paths)
        self.assertIn("fuel.ignitionrobotics.org/cole/models/sunken vase/4", paths)
        for asset in lock["assets"]:
            self.assertEqual(str(assets.model_path(asset["url"])), asset["cache_path"])
            self.assertTrue(asset["license"]["url"])
            for digest in asset["files"].values():
                self.assertEqual(len(bytes.fromhex(digest)), hashlib.sha256().digest_size)


if __name__ == "__main__":
    unittest.main()
