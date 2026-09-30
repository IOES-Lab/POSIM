"""Use the recorded store identity; never overwrite another candidate image."""

import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch
import urllib.error

SPEC = importlib.util.spec_from_file_location(
    "candidate_publication", Path(__file__).with_name("candidate-publication.py")
)
publication = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(publication)


class CandidatePublicationTests(unittest.TestCase):
    def resolve(self, responses, arch="amd64", **kwargs):
        with patch.object(
            publication, "request_json", return_value=({"token": "test-pull-token"}, {})
        ), patch.object(publication, "registry_manifest", side_effect=responses):
            return publication.registry_image(arch, **kwargs)

    def index_responses(self):
        top = publication.LOCK["images"]["arm64"]["image_id"]
        platform = "sha256:" + "2" * 64
        config = "sha256:" + "4" * 64
        index = {
            "manifests": [
                {"digest": platform, "platform": {"os": "linux", "architecture": "arm64"}},
                {
                    "digest": "sha256:" + "3" * 64,
                    "platform": {"os": "unknown", "architecture": "unknown"},
                },
            ]
        }
        return [(index, top), ({"config": {"digest": config}}, platform)]

    def test_tags_are_validation_only(self):
        for arch, entry in publication.LOCK["images"].items():
            self.assertTrue(entry["tag"].startswith("validation-pr5-abad9d70-"))
            self.assertNotIn("latest", entry["tag"])
            self.assertNotIn("main", entry["tag"])
            self.assertEqual(len(entry["image_id"]), 71)
            self.assertIn(arch, ("arm64", "amd64"))

    def test_missing_tag_allowed_before_publication(self):
        error = urllib.error.HTTPError("https://registry.example", 404, "missing", {}, None)
        self.assertEqual(self.resolve([error], allow_missing=True)["exists"], False)

    def test_missing_tag_rejected_after_publication(self):
        error = urllib.error.HTTPError("https://registry.example", 404, "missing", {}, None)
        with self.assertRaises(urllib.error.HTTPError):
            self.resolve([error])

    def test_auth_and_network_failures_are_not_absence(self):
        for status in (401, 403, 429, 500):
            error = urllib.error.HTTPError("https://registry.example", status, "failure", {}, None)
            with self.subTest(status=status), self.assertRaises(urllib.error.HTTPError):
                self.resolve([error], allow_missing=True)

    def test_matching_legacy_config_permits_idempotent_reuse(self):
        expected = publication.LOCK["images"]["amd64"]["image_id"]
        r = self.resolve([({"config": {"digest": expected}}, "sha256:" + "1" * 64)])
        self.assertTrue(r["exists"])
        self.assertEqual(r["image_id"], expected)
        self.assertEqual(r["config_digest"], expected)
        self.assertEqual(r["image_identity"], "config_digest")

    def test_different_legacy_config_refuses_overwrite(self):
        with self.assertRaisesRegex(RuntimeError, "another image"):
            self.resolve(
                [({"config": {"digest": "sha256:" + "0" * 64}}, "sha256:" + "1" * 64)],
                allow_missing=True,
            )

    def test_containerd_index_is_not_the_config_digest(self):
        r = self.resolve(self.index_responses(), arch="arm64")
        self.assertEqual(r["tag_digest"], r["image_id"])
        self.assertNotEqual(r["config_digest"], r["image_id"])
        self.assertEqual(r["image_identity"], "index_digest")

    def test_platform_selection_ignores_attestation(self):
        r = self.resolve(self.index_responses(), arch="arm64")
        self.assertEqual(r["platform_digest"], "sha256:" + "2" * 64)

    def test_wrong_index_refuses_even_when_config_matches_old_assumption(self):
        expected = publication.LOCK["images"]["arm64"]["image_id"]
        with self.assertRaisesRegex(RuntimeError, "another image"):
            self.resolve(
                [({"config": {"digest": expected}}, "sha256:" + "1" * 64)],
                arch="arm64",
                allow_missing=True,
            )

    def test_ambiguous_platform_is_rejected(self):
        descriptor = {
            "digest": "sha256:" + "2" * 64,
            "platform": {"os": "linux", "architecture": "arm64"},
        }
        with self.assertRaisesRegex(RuntimeError, "ambiguous"):
            self.resolve(
                [({"manifests": [descriptor, descriptor]}, "sha256:" + "1" * 64)],
                arch="arm64",
            )

    def test_wrong_platform_digest_is_rejected(self):
        responses = self.index_responses()
        responses[1] = (responses[1][0], "sha256:" + "3" * 64)
        with self.assertRaisesRegex(RuntimeError, "digest mismatch"):
            self.resolve(responses, arch="arm64")

    def test_missing_config_digest_is_rejected(self):
        responses = self.index_responses()
        responses[1] = ({}, responses[1][1])
        with self.assertRaisesRegex(RuntimeError, "Invalid config"):
            self.resolve(responses, arch="arm64")


if __name__ == "__main__":
    unittest.main()
