#!/usr/bin/env python3
"""Select only tested, tree-identical candidate images and record their digests."""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import zipfile

from github_api import from_environment
from image_provenance import (
    ATTEMPT_LABEL,
    RUN_LABEL,
    TREE_LABEL,
    commit_tree,
    merged_prs,
    successful_candidate_run,
    verify_source,
)
from publish_image import IMAGE, docker, inspect_digest


ARTIFACT = "posim-image-receipt"


def find_candidate(api, target_sha):
    target_tree = commit_tree(api, target_sha)
    for pr in merged_prs(api, target_sha):
        head = pr["head"]["sha"]
        if commit_tree(api, head) != target_tree:
            continue
        runs = api.pages(
            f"actions/workflows/docker-amd64.yml/runs?head_sha={head}&event=pull_request&status=success",
            "workflow_runs",
        )
        for run in runs:
            if not successful_candidate_run(api, run, head):
                continue
            artifacts = api.pages(f"actions/runs/{run['id']}/artifacts", "artifacts")
            for artifact in artifacts:
                if (
                    artifact["name"] != f"{ARTIFACT}-{run['run_attempt']}"
                    or artifact["expired"]
                    or artifact["size_in_bytes"] > 65536
                ):
                    continue
                try:
                    receipt = api.artifact_json(artifact["id"])
                except (ValueError, KeyError, zipfile.BadZipFile):
                    continue
                if not isinstance(receipt, dict):
                    continue
                expected = {
                    "schema": 1,
                    "kind": "candidate",
                    "source_sha": head,
                    "build_sha": head,
                    "source_tree": target_tree,
                    "run_id": run["id"],
                    "run_attempt": run["run_attempt"],
                    "pr_number": pr["number"],
                }
                if any(receipt.get(key) != value for key, value in expected.items()):
                    continue
                digest = receipt.get("digest", "")
                if not re.fullmatch(r"sha256:[0-9a-f]{64}", digest):
                    continue
                try:
                    images = json.loads(
                        docker("inspect", f"{IMAGE}@{digest}", "--format", "{{json .Image}}")
                    )
                except subprocess.CalledProcessError as error:
                    if ": not found" in error.output:
                        continue
                    raise
                expected_labels = {
                    TREE_LABEL: target_tree,
                    RUN_LABEL: str(run["id"]),
                    ATTEMPT_LABEL: str(run["run_attempt"]),
                }
                if any(
                    images.get(arch, {}).get("config", {}).get("Labels", {}).get(key) != value
                    for arch in ("linux/amd64", "linux/arm64")
                    for key, value in expected_labels.items()
                ):
                    continue
                try:
                    provenance = verify_source(images, target_sha, api)
                except ValueError:
                    continue
                if provenance["build_sha"] == head:
                    return {"digest": digest, **provenance}
    return None


def record(api, sha, tree, tag, kind):
    digest = inspect_digest(f"{IMAGE}:{tag}")
    images = json.loads(docker("inspect", f"{IMAGE}@{digest}", "--format", "{{json .Image}}"))
    provenance = verify_source(images, sha, api)
    if commit_tree(api, sha) != tree:
        raise ValueError("Checkout tree differs from the source commit")
    return {
        "schema": 1,
        "kind": kind,
        "source_sha": sha,
        "source_tree": tree,
        "digest": digest,
        **provenance,
        "run_id": int(os.environ["GITHUB_RUN_ID"]),
        "run_attempt": int(os.environ["GITHUB_RUN_ATTEMPT"]),
        "pr_number": int(os.environ.get("PR_NUMBER") or "0"),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    select = sub.add_parser("select")
    select.add_argument("--sha", required=True)
    receipt = sub.add_parser("record")
    receipt.add_argument("--sha", required=True)
    receipt.add_argument("--tree", required=True)
    receipt.add_argument("--tag", required=True)
    receipt.add_argument("--kind", choices=("candidate", "main"), required=True)
    receipt.add_argument("--output", required=True)
    args = parser.parse_args()
    api = from_environment()
    if args.command == "select":
        selected = find_candidate(api, args.sha)
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
            output.write(f"reuse={'true' if selected else 'false'}\n")
            if selected:
                output.write(f"digest={selected['digest']}\n")
        print(
            json.dumps(selected) if selected else "No reusable candidate; build the merged source"
        )
    else:
        result = record(api, args.sha, args.tree, args.tag, args.kind)
        Path(args.output).write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(result))


if __name__ == "__main__":
    main()
