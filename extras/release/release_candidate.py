#!/usr/bin/env python3
"""Publish a merged release candidate or prepare one manual recovery PR."""

import argparse
import base64
from datetime import datetime, timezone
import json
import os
from pathlib import PurePosixPath
import re
import subprocess
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET

from publish_image import IMAGE, promote, promote_latest


MANIFEST = ".github/release-candidate.json"
LABEL = "release-candidate"
REQUIRED = {"docker-amd64.yml", "lint.yml", "documentation.yml", "release-checks.yml"}


class GitHub:
    def __init__(self, repo, token):
        self.repo = repo
        self.token = token

    def api(self, path, data=None, method=None, missing=False):
        request = urllib.request.Request(
            f"https://api.github.com/repos/{self.repo}/{path}",
            data=json.dumps(data).encode() if data is not None else None,
            method=method or ("POST" if data is not None else "GET"),
            headers={
                "Authorization": f"Bearer {self.token}",
                "Accept": "application/vnd.github+json",
                "Content-Type": "application/json",
                "X-GitHub-Api-Version": "2022-11-28",
            },
        )
        try:
            with urllib.request.urlopen(request, timeout=60) as response:
                return json.load(response)
        except urllib.error.HTTPError as error:
            if missing and error.code == 404:
                return None
            raise RuntimeError(f"GitHub {method or 'GET'} {path}: HTTP {error.code}") from error

    def pages(self, path, key=None):
        separator = "&" if "?" in path else "?"
        page = 1
        while True:
            result = self.api(f"{path}{separator}per_page=100&page={page}")
            items = result[key] if key else result
            yield from items
            if len(items) < 100:
                return
            page += 1

    def file(self, path, ref):
        result = self.api(f"contents/{path}?ref={urllib.parse.quote(ref, safe='')}")
        return base64.b64decode(result["content"]).decode(), result["sha"]


def candidate(api, sha):
    matches = []
    for pr in api.pages(f"commits/{sha}/pulls"):
        if (
            pr.get("merged_at")
            and pr.get("merge_commit_sha") == sha
            and pr["base"]["ref"] == "main"
            and pr["base"]["repo"]["full_name"] == api.repo
            and (pr["head"].get("repo") or {}).get("full_name") == api.repo
            and LABEL in {label["name"] for label in pr["labels"]}
        ):
            match = re.fullmatch(
                r"release/([0-9]+\.[0-9]+\.[0-9]+)-rc(?:-[a-z0-9-]+)?", pr["head"]["ref"]
            )
            if match:
                matches.append((pr, match[1]))
    if not matches:
        return None
    if len(matches) != 1:
        raise ValueError("More than one release candidate is associated with this commit")
    pr, version = matches[0]
    manifest = json.loads(api.file(MANIFEST, sha)[0])
    if (
        manifest["version"] != version
        or type(manifest["attempt"]) is not int
        or manifest["attempt"] < 1
    ):
        raise ValueError("Invalid release candidate manifest")
    changed = {item["filename"] for item in api.pages(f"pulls/{pr['number']}/files")}
    if MANIFEST not in changed:
        raise ValueError("A release candidate must update its manifest")
    return pr, manifest


def ci_state(runs):
    latest = {}
    for run in runs:
        if run["event"] != "push" or run["head_branch"] != "main":
            continue
        path = run["path"].split("@")[0].rsplit("/", 1)[-1]
        if path not in latest or run["id"] > latest[path]["id"]:
            latest[path] = run
    if not REQUIRED.issubset(latest) or any(r["status"] != "completed" for r in latest.values()):
        return "waiting", []
    failed = [r for r in latest.values() if r["conclusion"] != "success"]
    return ("failed" if failed else "success"), failed


def validate_version(api, sha, version):
    if json.loads(api.file("website/package.json", sha)[0])["version"] != version:
        raise ValueError("Website version differs from the candidate")
    tree = api.api(f"git/trees/{sha}?recursive=1")
    if tree.get("truncated"):
        raise ValueError("Cannot validate a truncated source tree")
    packages = [item["path"] for item in tree["tree"] if item["path"].endswith("/package.xml")]
    if not packages:
        raise ValueError("No ROS package versions found")
    for path in packages:
        if ET.fromstring(api.file(path, sha)[0]).findtext("version") != version:
            raise ValueError(f"Version mismatch in {path}")
    rust = "gazebo/posim_gz_multibeam_sonar/wgpu_vendor/sonar_wgpu_rust/"
    for filename in ("Cargo.toml", "Cargo.lock"):
        content = api.file(rust + filename, sha)[0]
        if not re.search(r'name = "sonar_wgpu"\s+version = "' + re.escape(version) + '"', content):
            raise ValueError(f"Version mismatch in {filename}")
    changelog = api.file("CHANGELOG.md", sha)[0]
    if not re.search(r"^## " + re.escape(version) + r"(?: - .*)?$", changelog, re.M):
        raise ValueError("The candidate is absent from the changelog")
    return api.file(f"docs/releases/v{version}.md", sha)[0]


def release_body(notes, repo, version, sha, digest):
    date = datetime.now(timezone.utc).date().isoformat()
    notes = re.sub(
        r"^Status: release candidate.*$", f"Release date: {date} (UTC).", notes, flags=re.M
    )
    # Candidate-only instructions are not copied into the published release.
    notes = re.sub(
        r"<!-- candidate-only:start -->.*?<!-- candidate-only:end -->\n?", "", notes, flags=re.S
    )

    def link(match):
        target = match[1]
        if re.match(r"[a-zA-Z][a-zA-Z0-9+.-]*:", target) or target.startswith("#"):
            return match[0]
        parts = []
        for part in PurePosixPath("docs/releases", target).parts:
            if part == "..":
                if not parts:
                    raise ValueError("Release link escapes the repository")
                parts.pop()
            elif part != ".":
                parts.append(part)
        return f"](https://github.com/{repo}/blob/v{version}/{'/'.join(parts)})"

    notes = re.sub(r"\]\(([^)]+)\)", link, notes)
    return notes.rstrip() + (
        f"\n\n## Published image\n\n"
        f"- Docker: `{IMAGE}:{version}` (AMD64 and ARM64).\n"
        f"- Immutable image: `{IMAGE}@{digest}`.\n"
        f"- Source commit: `{sha}`.\n"
    )


def tag_sha(api, tag):
    ref = api.api(f"git/ref/tags/{tag}", missing=True)
    if ref is None:
        return None
    obj = ref["object"]
    while obj["type"] == "tag":
        obj = api.api(f"git/tags/{obj['sha']}")["object"]
    if obj["type"] != "commit":
        raise ValueError("Release tag does not point to a commit")
    return obj["sha"]


def publish(api, sha, version, notes, publisher=promote, latest_updater=None):
    tag = f"v{version}"
    existing_sha = tag_sha(api, tag)
    if existing_sha is not None and existing_sha != sha:
        raise ValueError(f"{tag} already points to a different commit")
    releases = [r for r in api.pages("releases") if r["tag_name"] == tag]
    release = releases[0] if releases else None
    if release and release["draft"] and release["target_commitish"] != sha:
        raise ValueError("Existing draft targets a different commit")
    if release and not release["draft"] and existing_sha is None:
        raise ValueError("Published release has no verifiable source tag")
    # No release is made public before the two-architecture Docker alias is verified.
    digest = publisher(version, sha)
    if release and not release["draft"]:
        print(f"{tag} is already published at the verified source and image")
    else:
        if existing_sha is None:
            api.api("git/refs", {"ref": f"refs/tags/{tag}", "sha": sha})
        body = release_body(notes, api.repo, version, sha, digest)
        if release is None:
            release = api.api(
                "releases",
                {
                    "tag_name": tag,
                    "target_commitish": sha,
                    "name": f"Release {version}",
                    "body": body,
                    "draft": True,
                    "prerelease": False,
                },
            )
        api.api(
            f"releases/{release['id']}",
            {
                "body": body,
                "draft": False,
                "prerelease": False,
                "make_latest": "true",
            },
            method="PATCH",
        )
    # Older-release retries must not move latest backwards. A failed sync can
    # resume here without changing the published release or rebuilding images.
    if api.api("releases/latest")["tag_name"] == tag:
        (latest_updater or promote_latest)(version, sha)
    print(f"Published {tag} from {sha} at {digest}")


def recover(api, sha, pr, manifest, failed):
    version = manifest["version"]
    branch = f"release/{version}-rc-retry-{sha[:12]}"
    # A closed recovery PR is a maintainer decision; do not reopen or replace it.
    pulls = list(api.pages("pulls?state=all&base=main"))
    for item in pulls:
        same_repo = (item["head"].get("repo") or {}).get("full_name") == api.repo
        if same_repo and item["head"]["ref"] == branch:
            if item["state"] == "open":
                api.api(f"issues/{item['number']}/labels", {"labels": [LABEL]})
            print(f"Recovery already recorded in {item['html_url']}")
            return
        if (
            same_repo
            and item["state"] == "open"
            and item["head"]["ref"].startswith(f"release/{version}-rc")
        ):
            print(f"An existing candidate already covers {version}: {item['html_url']}")
            return
    if tag_sha(api, f"v{version}") is not None:
        raise ValueError("Version tag already exists; rerun publication for its original commit")
    base_sha = api.api("git/ref/heads/main")["object"]["sha"]
    current_text, current_blob = api.file(MANIFEST, base_sha)
    current = json.loads(current_text)
    if current["version"] != version or current["attempt"] > manifest["attempt"]:
        print("A newer release candidate has superseded this failure")
        return
    existing = api.api(f"git/ref/heads/{branch}", missing=True)
    if existing is None:
        api.api("git/refs", {"ref": f"refs/heads/{branch}", "sha": base_sha})
    branch_text, branch_blob = api.file(MANIFEST, branch)
    next_manifest = {"version": version, "attempt": manifest["attempt"] + 1}
    if json.loads(branch_text) != next_manifest:
        if branch_blob != current_blob:
            raise ValueError("Recovery branch has independent manifest edits")
        api.api(
            f"contents/{MANIFEST}",
            {
                "message": f"release: prepare {version} follow-up candidate",
                "branch": branch,
                "sha": branch_blob,
                "content": base64.b64encode(
                    (json.dumps(next_manifest, indent=2) + "\n").encode()
                ).decode(),
            },
            method="PUT",
        )
    failures = "\n".join(f"- [{r['name']}]({r['html_url']}): {r['conclusion']}" for r in failed)
    body = (
        f"Follow-up to #{pr['number']} after CI failed for `{sha}`.\n\n{failures}\n\n"
        "This draft advances the candidate attempt only; it does not fix source code. "
        "Inspect the failed job logs, add the required fixes, and review before merging. "
        "If a transient failure succeeds on rerun, close this draft instead.\n\n"
        "Merging requires a human decision. The new merge commit must pass all release gates. "
        "This automation never approves or merges pull requests."
    )
    created = api.api(
        "pulls",
        {
            "title": f"release: repair {version} candidate after CI failure",
            "head": branch,
            "base": "main",
            "body": body,
            "draft": True,
        },
    )
    api.api(f"issues/{created['number']}/labels", {"labels": [LABEL]})
    print(f"Created draft recovery candidate: {created['html_url']}")


def process(api, sha, publisher=promote):
    if not re.fullmatch(r"[0-9a-f]{40}", sha):
        raise ValueError("A full merge commit SHA is required")
    selected = candidate(api, sha)
    if selected is None:
        print("Not a merged, labeled release candidate; no changes")
        return
    pr, manifest = selected
    current = json.loads(api.file(MANIFEST, "main")[0])
    if current["version"] != manifest["version"] or current["attempt"] > manifest["attempt"]:
        print("A newer merged candidate has superseded this attempt")
        return
    runs = list(api.pages(f"actions/runs?head_sha={sha}&event=push", "workflow_runs"))
    if any(run["head_sha"] != sha for run in runs):
        raise ValueError("CI run source mismatch")
    status, failed = ci_state(runs)
    if status == "waiting":
        print("Waiting for all release gates; no changes")
    elif status == "failed":
        recover(api, sha, pr, manifest, failed)
    else:
        notes = validate_version(api, sha, manifest["version"])
        publish(api, sha, manifest["version"], notes, publisher)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sha", required=True)
    args = parser.parse_args()
    api = GitHub(os.environ["GITHUB_REPOSITORY"], os.environ["GH_TOKEN"])

    def publish_with_login(version, sha):
        subprocess.run(
            [
                "docker",
                "login",
                "--username",
                os.environ["DOCKERHUB_USERNAME"],
                "--password-stdin",
            ],
            input=os.environ["DOCKERHUB_TOKEN"],
            text=True,
            check=True,
        )
        return promote(version, sha)

    process(api, args.sha, publish_with_login)


if __name__ == "__main__":
    main()
