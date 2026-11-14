#!/usr/bin/env python3
"""Candidate reuse must never substitute an untested or different source tree."""

import copy
import io
import json
import subprocess
import unittest
from unittest.mock import Mock, patch
import zipfile

from github_api import GitHub
import image_provenance as proof
import reuse_image as reuse


MERGE = "a" * 40
BUILD = "b" * 40
TREE = "c" * 40
DIGEST = "sha256:" + "d" * 64
REPO = "IOES-Lab/POSIM"


class ReuseTests(unittest.TestCase):
    def setUp(self):
        self.run = {
            "id": 123,
            "run_attempt": 2,
            "workflow_id": 9,
            "event": "pull_request",
            "status": "completed",
            "conclusion": "success",
            "head_sha": BUILD,
            "head_repository": {"full_name": REPO},
            "repository": {"full_name": REPO},
        }
        self.pr = {
            "number": 80,
            "merged_at": "2026-11-14T00:00:00Z",
            "merge_commit_sha": MERGE,
            "head": {"sha": BUILD, "repo": {"full_name": REPO}},
            "base": {"ref": "main", "repo": {"full_name": REPO}},
        }
        self.receipt = {
            "schema": 1,
            "kind": "candidate",
            "source_sha": BUILD,
            "build_sha": BUILD,
            "source_tree": TREE,
            "run_id": 123,
            "run_attempt": 2,
            "pr_number": 80,
            "digest": DIGEST,
        }
        self.artifact = {
            "id": 456,
            "name": f"{reuse.ARTIFACT}-2",
            "expired": False,
            "size_in_bytes": 1000,
        }
        self.trees = {MERGE: TREE, BUILD: TREE}
        self.labels = {
            "org.opencontainers.image.revision": BUILD,
            proof.TREE_LABEL: TREE,
            proof.RUN_LABEL: "123",
            proof.ATTEMPT_LABEL: "2",
        }
        self.images = {
            arch: {"config": {"Labels": copy.deepcopy(self.labels)}}
            for arch in ("linux/amd64", "linux/arm64")
        }
        self.api = Mock(repo=REPO)
        self.api.api.side_effect = self.request
        self.api.pages.side_effect = self.pages
        self.api.artifact_json.side_effect = lambda _: self.receipt

    def request(self, path):
        if path.startswith("git/commits/"):
            return {"tree": {"sha": self.trees[path.rsplit("/", 1)[1]]}}
        if path == "actions/workflows/docker-amd64.yml":
            return {"id": 9}
        if path == "actions/runs/123/attempts/2":
            return self.run
        raise AssertionError(path)

    def pages(self, path, key=None):
        if path == f"commits/{MERGE}/pulls":
            return [self.pr]
        if path.startswith("actions/workflows/docker-amd64.yml/runs?"):
            self.assertIn(f"head_sha={BUILD}", path)
            return [self.run]
        if path == "actions/runs/123/artifacts":
            return [self.artifact]
        raise AssertionError(path)

    def select(self):
        with patch.object(reuse, "docker", return_value=json.dumps(self.images)):
            return reuse.find_candidate(self.api, MERGE)

    def test_tree_identical_successful_candidate_reuses_exact_digest(self):
        selected = self.select()
        self.assertEqual(selected["digest"], DIGEST)
        self.assertEqual(selected["build_sha"], BUILD)
        self.assertEqual(selected["source_tree"], TREE)
        self.assertEqual(
            self.images["linux/arm64"]["config"]["Labels"]["org.opencontainers.image.revision"],
            BUILD,
        )

    def test_new_main_content_requires_rebuild(self):
        self.trees[MERGE] = "e" * 40
        self.assertIsNone(self.select())
        self.api.artifact_json.assert_not_called()

    def test_fork_or_different_merge_never_supplies_candidate(self):
        for kind in ("fork", "unmerged", "wrong_merge", "wrong_base"):
            original = copy.deepcopy(self.pr)
            if kind == "fork":
                self.pr["head"]["repo"]["full_name"] = "untrusted/POSIM"
            elif kind == "unmerged":
                self.pr["merged_at"] = None
            elif kind == "wrong_merge":
                self.pr["merge_commit_sha"] = "e" * 40
            else:
                self.pr["base"]["ref"] = "develop"
            with self.subTest(kind=kind):
                self.assertIsNone(self.select())
            self.pr = original

    def test_failed_incomplete_other_workflow_and_other_source_are_rejected(self):
        changes = [
            {"conclusion": "failure"},
            {"status": "in_progress"},
            {"workflow_id": 10},
            {"head_sha": MERGE},
            {"event": "push"},
            {"head_repository": {"full_name": "untrusted/POSIM"}},
        ]
        for change in changes:
            original = copy.deepcopy(self.run)
            self.run.update(change)
            with self.subTest(change=change):
                self.assertIsNone(self.select())
            self.run = original

    def test_receipt_must_match_the_successful_run_and_pr(self):
        for key, value in {
            "run_id": 124,
            "run_attempt": 1,
            "pr_number": 81,
            "schema": 2,
            "kind": "main",
            "source_sha": MERGE,
            "build_sha": MERGE,
            "source_tree": "e" * 40,
            "digest": "latest",
        }.items():
            original = self.receipt[key]
            self.receipt[key] = value
            with self.subTest(key=key):
                self.assertIsNone(self.select())
            self.receipt[key] = original

    def test_expired_oversized_and_missing_receipts_require_rebuild(self):
        for change in ({"expired": True}, {"size_in_bytes": 65537}, {"name": "unrelated"}):
            original = copy.deepcopy(self.artifact)
            self.artifact.update(change)
            self.assertIsNone(self.select())
            self.artifact = original
        self.receipt = None
        self.assertIsNone(self.select())

    def test_missing_registry_image_requires_rebuild(self):
        with patch.object(
            reuse,
            "docker",
            side_effect=subprocess.CalledProcessError(1, "docker", "image: not found"),
        ):
            self.assertIsNone(reuse.find_candidate(self.api, MERGE))

    def test_registry_auth_failure_is_not_misreported_as_a_missing_image(self):
        with patch.object(
            reuse, "docker", side_effect=subprocess.CalledProcessError(1, "docker", "unauthorized")
        ):
            with self.assertRaises(subprocess.CalledProcessError):
                reuse.find_candidate(self.api, MERGE)

    def test_architecture_and_run_provenance_cannot_be_mixed(self):
        for key, value in {
            "org.opencontainers.image.revision": MERGE,
            proof.TREE_LABEL: "e" * 40,
            proof.RUN_LABEL: "124",
            proof.ATTEMPT_LABEL: "1",
        }.items():
            labels = self.images["linux/arm64"]["config"]["Labels"]
            old = labels[key]
            labels[key] = value
            with self.subTest(key=key):
                self.assertIsNone(self.select())
            labels[key] = old

    def test_promotion_rechecks_the_original_successful_attempt(self):
        self.run["run_attempt"] = 3
        with self.assertRaises(ValueError):
            proof.verify_source(self.images, MERGE, self.api)

    def test_direct_builds_keep_the_original_exact_commit_rule(self):
        self.assertEqual(proof.verify_source(self.images, BUILD), {"build_sha": BUILD})
        self.images.pop("linux/arm64")
        with self.assertRaises(ValueError):
            proof.verify_source(self.images, BUILD)


class ArtifactTests(unittest.TestCase):
    def archive(self, content):
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            archive.writestr("image-receipt.json", content)
            archive.writestr("../../ignored.py", "must never execute")
        return buffer.getvalue()

    @patch("github_api.subprocess.check_output")
    def test_only_small_json_is_read_without_extracting_files(self, command):
        command.return_value = self.archive('{"schema":1}')
        api = GitHub(REPO, "test-token")
        self.assertEqual(api.artifact_json(456), {"schema": 1})
        command.return_value = self.archive(" " * 16385)
        with self.assertRaises(ValueError):
            api.artifact_json(456)

    @patch("github_api.subprocess.check_output")
    def test_expired_download_can_fall_back_to_build(self, command):
        command.side_effect = subprocess.CalledProcessError(1, "gh", b"HTTP 410")
        self.assertIsNone(GitHub(REPO, "test-token").artifact_json(456))


if __name__ == "__main__":
    unittest.main()
