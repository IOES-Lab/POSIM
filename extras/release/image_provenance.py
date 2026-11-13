#!/usr/bin/env python3
"""Validate image source identity, including tree-identical merged candidates."""

import re

from github_api import from_environment


TREE_LABEL = "ioeslab.posim.source-tree"
RUN_LABEL = "ioeslab.posim.build-run-id"
ATTEMPT_LABEL = "ioeslab.posim.build-run-attempt"


def commit_tree(api, sha):
    if not re.fullmatch(r"[0-9a-f]{40}", sha):
        raise ValueError("Expected a full commit SHA")
    return api.api(f"git/commits/{sha}")["tree"]["sha"]


def merged_prs(api, sha):
    return [
        pr
        for pr in api.pages(f"commits/{sha}/pulls")
        if (
            pr.get("merged_at")
            and pr.get("merge_commit_sha") == sha
            and pr["base"]["ref"] == "main"
            and pr["base"]["repo"]["full_name"] == api.repo
            and (pr["head"].get("repo") or {}).get("full_name") == api.repo
        )
    ]


def successful_candidate_run(api, run, build_sha):
    workflow = api.api("actions/workflows/docker-amd64.yml")
    return (
        run["workflow_id"] == workflow["id"]
        and run["event"] == "pull_request"
        and run["status"] == "completed"
        and run["conclusion"] == "success"
        and run["head_sha"] == build_sha
        and run["head_repository"]["full_name"] == api.repo
        and run["repository"]["full_name"] == api.repo
    )


def verify_source(images, expected_sha, api=None):
    labels = []
    for arch in ("linux/amd64", "linux/arm64"):
        value = images.get(arch, {}).get("config", {}).get("Labels", {})
        if not re.fullmatch(r"[0-9a-f]{40}", value.get("org.opencontainers.image.revision", "")):
            raise ValueError(f"{arch} has no valid source revision")
        labels.append(value)
    build_sha = labels[0]["org.opencontainers.image.revision"]
    if labels[1]["org.opencontainers.image.revision"] != build_sha:
        raise ValueError("Architecture images have different source revisions")
    if build_sha == expected_sha:
        return {"build_sha": build_sha}

    api = api or from_environment()
    tree = commit_tree(api, expected_sha)
    if commit_tree(api, build_sha) != tree:
        raise ValueError("Built source differs from the merged source tree")
    run_id, attempt = labels[0].get(RUN_LABEL, ""), labels[0].get(ATTEMPT_LABEL, "")
    if not re.fullmatch(r"[1-9][0-9]*", run_id) or not re.fullmatch(r"[1-9][0-9]*", attempt):
        raise ValueError("Reused image has no CI provenance")
    for value in labels:
        if (
            value.get(TREE_LABEL) != tree
            or value.get(RUN_LABEL) != run_id
            or value.get(ATTEMPT_LABEL) != attempt
        ):
            raise ValueError("Architecture images disagree on build provenance")
    run = api.api(f"actions/runs/{run_id}/attempts/{attempt}")
    if (
        run["id"] != int(run_id)
        or run["run_attempt"] != int(attempt)
        or not successful_candidate_run(api, run, build_sha)
    ):
        raise ValueError("Reused image did not come from a successful candidate CI run")
    if not any(pr["head"]["sha"] == build_sha for pr in merged_prs(api, expected_sha)):
        raise ValueError("The tested PR head is not the merged source")
    return {
        "build_sha": build_sha,
        "source_tree": tree,
        "build_run": f"https://github.com/{api.repo}/actions/runs/{run_id}/attempts/{attempt}",
    }
