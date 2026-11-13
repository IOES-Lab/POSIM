#!/usr/bin/env python3
"""Release tests use fake GitHub and Docker responses; they never publish."""

import base64
import copy
import json
from pathlib import Path
import re
import subprocess
import unittest
from unittest.mock import Mock, patch

import publish_image as image
import release_candidate as rc


SHA = "a" * 40
OTHER = "b" * 40
DIGEST = "sha256:" + "c" * 64
VERSION = "1.0.1"
MANIFEST = {"version": VERSION, "attempt": 1}


def pull():
    return {
        "number": 80,
        "merged_at": "2026-11-14T00:00:00Z",
        "merge_commit_sha": SHA,
        "base": {"ref": "main", "repo": {"full_name": "IOES-Lab/POSIM"}},
        "head": {"ref": "release/1.0.1-rc-automation", "repo": {"full_name": "IOES-Lab/POSIM"}},
        "labels": [{"name": rc.LABEL}],
    }


def runs():
    return [
        {
            "id": index,
            "path": f".github/workflows/{name}",
            "name": name,
            "event": "push",
            "head_branch": "main",
            "head_sha": SHA,
            "status": "completed",
            "conclusion": "success",
            "html_url": f"https://ci/{index}",
        }
        for index, name in enumerate(sorted(rc.REQUIRED), 1)
    ]


class FakeGitHub:
    """Small mutable remote for exercising recovery and publication reruns."""

    repo = "IOES-Lab/POSIM"

    def __init__(self):
        self.refs = {"heads/main": OTHER}
        self.files = {rc.MANIFEST: (json.dumps(MANIFEST), "blob1")}
        self.pulls = []
        self.releases = []
        self.writes = []
        self.fail_pull = False
        self.fail_publish = False
        self.latest_tag = None

    def file(self, path, ref):
        return self.files[path]

    def pages(self, path, key=None):
        if path == "pulls?state=all&base=main":
            return iter(self.pulls)
        if path == "releases":
            return iter(self.releases)
        raise AssertionError(path)

    def api(self, path, data=None, method=None, missing=False):
        if data is None and path == "releases/latest":
            if self.latest_tag:
                return {"tag_name": self.latest_tag}
            return next(release for release in self.releases if not release["draft"])
        if data is None and path.startswith("git/ref/"):
            sha = self.refs.get(path.removeprefix("git/ref/"))
            if sha is None:
                assert missing
                return None
            return {"object": {"type": "commit", "sha": sha}}
        self.writes.append((path, copy.deepcopy(data), method))
        if path == "git/refs":
            self.refs[data["ref"].removeprefix("refs/")] = data["sha"]
            return {}
        if path.startswith("contents/"):
            self.files[path.removeprefix("contents/")] = (
                base64.b64decode(data["content"]).decode(),
                "blob2",
            )
            return {}
        if path == "pulls":
            if self.fail_pull:
                raise RuntimeError("Organization does not allow PR creation")
            pr = {
                "number": 81,
                "state": "open",
                "html_url": "https://pr/81",
                "head": {"ref": data["head"], "repo": {"full_name": self.repo}},
                **data,
            }
            pr["head"] = {"ref": data["head"], "repo": {"full_name": self.repo}}
            self.pulls.append(pr)
            return pr
        if path.startswith("issues/"):
            return data
        if path == "releases":
            release = {"id": 1, **data}
            self.releases.append(release)
            return release
        if path == "releases/1":
            if self.fail_publish:
                raise RuntimeError("Publication unavailable")
            self.releases[0].update(data)
            return self.releases[0]
        raise AssertionError((path, data))


class CandidateTests(unittest.TestCase):
    def api(self, pr=None):
        api = Mock(repo="IOES-Lab/POSIM")
        api.pages.side_effect = [[pr or pull()], [{"filename": rc.MANIFEST}]]
        api.file.return_value = (json.dumps(MANIFEST), "blob")
        return api

    def test_merged_labeled_same_repository_candidate(self):
        self.assertEqual(rc.candidate(self.api(), SHA)[1], MANIFEST)

    def test_untrusted_or_unmerged_candidates_do_not_mutate(self):
        for kind in ("fork", "unmerged", "wrong_sha", "no_label", "ordinary_branch", "wrong_base"):
            pr = pull()
            if kind == "fork":
                pr["head"]["repo"]["full_name"] = "fork/POSIM"
            elif kind == "unmerged":
                pr["merged_at"] = None
            elif kind == "wrong_sha":
                pr["merge_commit_sha"] = OTHER
            elif kind == "no_label":
                pr["labels"] = []
            elif kind == "ordinary_branch":
                pr["head"]["ref"] = "fix/runtime"
            else:
                pr["base"]["ref"] = "develop"
            api = self.api(pr)
            with self.subTest(kind=kind):
                self.assertIsNone(rc.candidate(api, SHA))
                api.file.assert_not_called()

    def test_candidate_must_update_manifest(self):
        api = self.api()
        api.pages.side_effect = [[pull()], [{"filename": "README.md"}]]
        with self.assertRaisesRegex(ValueError, "must update"):
            rc.candidate(api, SHA)

    def test_candidate_manifest_must_match_branch(self):
        api = self.api()
        api.file.return_value = ('{"version":"2.0.0","attempt":1}', "blob")
        with self.assertRaises(ValueError):
            rc.candidate(api, SHA)

    def test_ci_requires_all_gates_and_waits_for_completion(self):
        self.assertEqual(rc.ci_state(runs())[0], "success")
        self.assertEqual(rc.ci_state(runs()[:-1])[0], "waiting")
        pending = runs()
        pending[-1]["status"] = "in_progress"
        self.assertEqual(rc.ci_state(pending)[0], "waiting")

    def test_failed_cancelled_and_skipped_gates_block_publication(self):
        for conclusion in (
            "failure",
            "cancelled",
            "skipped",
            "timed_out",
            "neutral",
            "action_required",
        ):
            data = runs()
            data[0]["conclusion"] = conclusion
            with self.subTest(conclusion=conclusion):
                self.assertEqual(rc.ci_state(data), ("failed", [data[0]]))

    def test_latest_rerun_and_additional_main_workflows_are_checked(self):
        data = runs()
        retry = {**data[0], "id": 100, "status": "in_progress"}
        self.assertEqual(rc.ci_state(data + [retry])[0], "waiting")
        extra = {
            **data[0],
            "id": 101,
            "path": ".github/workflows/rviz-close.yml",
            "conclusion": "failure",
        }
        self.assertEqual(rc.ci_state(data + [extra])[0], "failed")
        retry["event"] = "pull_request"
        self.assertEqual(rc.ci_state(data + [retry])[0], "success")

    @patch.object(rc, "candidate", return_value=(pull(), MANIFEST))
    def test_wrong_sha_ci_is_rejected(self, selected):
        api = Mock()
        api.file.return_value = (json.dumps(MANIFEST), "blob")
        api.pages.return_value = [{**runs()[0], "head_sha": OTHER}]
        with self.assertRaisesRegex(ValueError, "source mismatch"):
            rc.process(api, SHA)

    @patch.object(rc, "candidate", return_value=(pull(), MANIFEST))
    def test_newer_merged_candidate_suppresses_old_completion(self, selected):
        api = Mock()
        api.file.return_value = (json.dumps({**MANIFEST, "attempt": 2}), "blob")
        rc.process(api, SHA)
        api.pages.assert_not_called()

    @patch.object(rc, "validate_version", return_value="notes")
    @patch.object(rc, "publish")
    @patch.object(rc, "recover")
    @patch.object(rc, "candidate", return_value=(pull(), MANIFEST))
    def test_controller_selects_only_the_appropriate_action(
        self, selected, recover, publish, validate
    ):
        api = Mock()
        api.file.return_value = (json.dumps(MANIFEST), "blob")
        for status in ("waiting", "failed", "success"):
            data = runs()
            if status == "waiting":
                data.pop()
            elif status == "failed":
                data[0]["conclusion"] = "failure"
            api.pages.return_value = data
            publish.reset_mock()
            recover.reset_mock()
            rc.process(api, SHA)
            self.assertEqual(publish.called, status == "success")
            self.assertEqual(recover.called, status == "failed")


class RecoveryTests(unittest.TestCase):
    def test_failed_candidate_creates_one_draft_without_merging(self):
        api = FakeGitHub()
        failed = [{**runs()[0], "conclusion": "failure"}]
        rc.recover(api, SHA, pull(), MANIFEST, failed)
        rc.recover(api, SHA, pull(), MANIFEST, failed)
        self.assertEqual(len(api.pulls), 1)
        self.assertTrue(api.pulls[0]["draft"])
        self.assertIn(SHA, api.pulls[0]["body"])
        self.assertIn(failed[0]["html_url"], api.pulls[0]["body"])
        self.assertEqual(json.loads(api.files[rc.MANIFEST][0])["attempt"], 2)
        self.assertFalse(any("/merge" in path or "/reviews" in path for path, _, _ in api.writes))

    def test_closed_followup_is_not_reopened_or_replaced(self):
        api = FakeGitHub()
        rc.recover(api, SHA, pull(), MANIFEST, [])
        api.pulls[0]["state"] = "closed"
        writes = len(api.writes)
        rc.recover(api, SHA, pull(), MANIFEST, [])
        self.assertEqual(len(api.writes), writes)

    def test_permission_failure_can_resume_after_branch_creation(self):
        api = FakeGitHub()
        api.fail_pull = True
        with self.assertRaises(RuntimeError):
            rc.recover(api, SHA, pull(), MANIFEST, [])
        api.fail_pull = False
        # The main file remains attempt 1; the existing recovery branch is attempt 2.
        original_file = api.file
        api.file = lambda path, ref: (
            (json.dumps(MANIFEST), "blob1") if ref == OTHER else original_file(path, ref)
        )
        rc.recover(api, SHA, pull(), MANIFEST, [])
        self.assertEqual(len(api.pulls), 1)
        self.assertEqual(sum(path == f"contents/{rc.MANIFEST}" for path, _, _ in api.writes), 1)

    def test_existing_version_tag_blocks_source_replacement(self):
        api = FakeGitHub()
        api.refs["tags/v1.0.1"] = SHA
        with self.assertRaisesRegex(ValueError, "original commit"):
            rc.recover(api, SHA, pull(), MANIFEST, [])
        self.assertEqual(api.writes, [])

    def test_newer_version_does_not_get_a_stale_recovery_branch(self):
        api = FakeGitHub()
        api.files[rc.MANIFEST] = ('{"version":"1.0.2","attempt":1}', "blob3")
        rc.recover(api, SHA, pull(), MANIFEST, [])
        self.assertEqual(api.writes, [])


class PublicationTests(unittest.TestCase):
    def setUp(self):
        patcher = patch.object(rc, "promote_latest", return_value=DIGEST)
        self.latest = patcher.start()
        self.addCleanup(patcher.stop)

    def test_image_is_verified_before_tag_and_release_creation(self):
        api = FakeGitHub()

        def publisher(version, sha):
            self.assertEqual((version, sha), (VERSION, SHA))
            self.assertEqual(api.writes, [])
            return DIGEST

        rc.publish(api, SHA, VERSION, "# Notes", publisher)
        self.assertEqual(api.refs["tags/v1.0.1"], SHA)
        self.assertFalse(api.releases[0]["draft"])
        self.assertIn(DIGEST, api.releases[0]["body"])
        self.latest.assert_called_once_with(VERSION, SHA)
        writes = len(api.writes)
        rc.publish(api, SHA, VERSION, "# Notes", lambda *_: DIGEST)
        self.assertEqual(len(api.writes), writes)

    def test_image_failure_does_not_create_a_tag_or_release(self):
        api = FakeGitHub()
        with self.assertRaises(RuntimeError):
            rc.publish(
                api, SHA, VERSION, "notes", Mock(side_effect=RuntimeError("Docker unavailable"))
            )
        self.assertEqual(api.writes, [])
        self.latest.assert_not_called()

    def test_publication_failure_resumes_the_existing_draft(self):
        api = FakeGitHub()
        api.fail_publish = True
        with self.assertRaises(RuntimeError):
            rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST)
        self.assertTrue(api.releases[0]["draft"])
        self.latest.assert_not_called()
        api.fail_publish = False
        rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST)
        self.assertEqual(len(api.releases), 1)
        self.assertFalse(api.releases[0]["draft"])

    def test_latest_is_only_updated_after_publication(self):
        api = FakeGitHub()

        def update(version, sha):
            self.assertFalse(api.releases[0]["draft"])
            self.assertEqual(api.refs["tags/v1.0.1"], SHA)

        rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST, update)

    def test_old_release_retry_cannot_roll_latest_back(self):
        api = FakeGitHub()
        api.refs["tags/v1.0.1"] = SHA
        api.releases = [{"id": 1, "tag_name": "v1.0.1", "draft": False}]
        api.latest_tag = "v1.0.2"
        rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST)
        self.latest.assert_not_called()
        self.assertEqual(api.writes, [])

    def test_latest_failure_resumes_without_republishing_release(self):
        api = FakeGitHub()
        self.latest.side_effect = RuntimeError("Registry unavailable")
        with self.assertRaises(RuntimeError):
            rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST)
        self.assertFalse(api.releases[0]["draft"])
        writes = len(api.writes)
        self.latest.side_effect = None
        rc.publish(api, SHA, VERSION, "notes", lambda *_: DIGEST)
        self.assertEqual(len(api.writes), writes)
        self.assertEqual(len(api.releases), 1)

    def test_conflicting_tag_or_draft_is_never_overwritten(self):
        for kind in ("tag", "draft"):
            api = FakeGitHub()
            if kind == "tag":
                api.refs["tags/v1.0.1"] = OTHER
            else:
                api.releases = [
                    {"id": 1, "tag_name": "v1.0.1", "draft": True, "target_commitish": OTHER}
                ]
            publisher = Mock()
            with self.assertRaises(ValueError):
                rc.publish(api, SHA, VERSION, "notes", publisher)
            publisher.assert_not_called()
            self.assertEqual(api.writes, [])

    def test_published_notes_have_tag_links_and_no_candidate_instructions(self):
        notes = (
            "Status: release candidate. Not published.\n"
            "[Install](../../website/content/en/install.md)\n"
            "<!-- candidate-only:start -->Not yet available<!-- candidate-only:end -->\n"
        )
        body = rc.release_body(notes, "IOES-Lab/POSIM", VERSION, SHA, DIGEST)
        self.assertNotIn("Not yet", body)
        self.assertNotIn("release candidate", body)
        self.assertIn("/blob/v1.0.1/website/content/en/install.md", body)
        self.assertIn("Release date:", body)

    def test_metadata_mismatch_stops_before_publication(self):
        api = Mock()
        api.file.return_value = ('{"version":"1.0.2"}', "blob")
        with self.assertRaisesRegex(ValueError, "Website version"):
            rc.validate_version(api, SHA, VERSION)


class ImageTests(unittest.TestCase):
    @patch.object(image, "source_digest", return_value=DIGEST)
    @patch.object(image, "docker")
    def test_latest_moves_to_the_verified_multiarchitecture_release(self, docker, source):
        docker.side_effect = [DIGEST, "sha256:" + "d" * 64, "", DIGEST]
        self.assertEqual(image.promote_latest(VERSION, SHA), DIGEST)
        docker.assert_any_call(
            "create", "--tag", "ioeslab/posim:latest", f"ioeslab/posim@{DIGEST}"
        )

    @patch.object(image, "source_digest", return_value=DIGEST)
    @patch.object(image, "docker")
    def test_latest_rejects_a_version_alias_with_wrong_digest(self, docker, source):
        docker.return_value = "sha256:" + "d" * 64
        with self.assertRaises(ValueError):
            image.promote_latest(VERSION, SHA)
        self.assertEqual(docker.call_count, 1)

    @patch.object(image, "docker")
    def test_missing_architecture_or_wrong_revision_is_rejected(self, docker):
        for images in (
            {},
            {"linux/amd64": {"config": {"Labels": {"org.opencontainers.image.revision": SHA}}}},
        ):
            docker.side_effect = [DIGEST, json.dumps(images)]
            with self.assertRaises(ValueError):
                image.source_digest(SHA)

    @patch.object(image, "source_digest", return_value=DIGEST)
    @patch.object(image, "docker")
    def test_promote_reuses_digest_without_building(self, docker, source):
        docker.side_effect = [
            subprocess.CalledProcessError(1, "docker", "image: not found"),
            "",
            DIGEST,
        ]
        self.assertEqual(image.promote(VERSION, SHA), DIGEST)
        docker.assert_any_call("create", "--tag", "ioeslab/posim:1.0.1", f"ioeslab/posim@{DIGEST}")

    @patch.object(image, "source_digest", return_value=DIGEST)
    @patch.object(image, "docker")
    def test_existing_matching_alias_is_read_only(self, docker, source):
        docker.side_effect = [DIGEST, DIGEST]
        self.assertEqual(image.promote(VERSION, SHA), DIGEST)
        self.assertFalse(any(call.args[0] == "create" for call in docker.call_args_list))

    @patch.object(image, "source_digest", return_value=DIGEST)
    @patch.object(image, "docker")
    def test_conflicting_alias_and_registry_errors_are_not_treated_as_missing(
        self, docker, source
    ):
        for result in (
            "sha256:" + "d" * 64,
            subprocess.CalledProcessError(1, "docker", "unauthorized"),
        ):
            docker.reset_mock()
            docker.side_effect = [result]
            with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                image.promote(VERSION, SHA)
            self.assertEqual(docker.call_count, 1)


class RepositoryTests(unittest.TestCase):
    def test_controller_watches_every_main_push_workflow(self):
        root = Path(__file__).resolve().parents[2]
        controller = (root / ".github/workflows/release-candidate.yml").read_text()
        for path in (root / ".github/workflows").glob("*.yml"):
            text = path.read_text()
            if "  push:\n" in text:
                name = re.search(r"^name: (.+)$", text, re.M)[1]
                self.assertIn(f"      - {name}\n", controller, path.name)

    def test_main_image_build_cannot_publish_latest(self):
        root = Path(__file__).resolve().parents[2]
        workflow = (root / ".github/workflows/docker-amd64.yml").read_text()
        self.assertNotIn("$IMAGE_NAME:latest", workflow)
        self.assertNotIn("UPDATE_LATEST", workflow)

    def test_real_candidate_versions_and_published_note_conversion(self):
        root = Path(__file__).resolve().parents[2]
        api = Mock()
        api.file.side_effect = lambda path, ref: ((root / path).read_text(), "blob")
        api.api.return_value = {
            "tree": [
                {"path": str(p.relative_to(root))}
                for p in root.rglob("package.xml")
                if "node_modules" not in p.parts
            ]
        }
        manifest = json.loads((root / rc.MANIFEST).read_text())
        notes = rc.validate_version(api, SHA, manifest["version"])
        body = rc.release_body(notes, "IOES-Lab/POSIM", manifest["version"], SHA, DIGEST)
        self.assertNotIn("<!-- candidate-only", body)
        self.assertNotIn("this candidate does not announce", body)
        self.assertIn(DIGEST, body)


if __name__ == "__main__":
    unittest.main()
