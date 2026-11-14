#!/usr/bin/env python3
"""GitHub metadata access shared by release publication and image reuse."""

import base64
import io
import json
import os
import subprocess
import urllib.error
import urllib.parse
import urllib.request
import zipfile


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

    def artifact_json(self, artifact_id):
        # gh handles the authenticated API redirect without forwarding the token
        # to the signed artifact download host. Never extract or execute files.
        try:
            data = subprocess.check_output(
                [
                    "gh",
                    "api",
                    "--allow-escape-sequences",
                    f"repos/{self.repo}/actions/artifacts/{int(artifact_id)}/zip",
                ],
                env={**os.environ, "GH_TOKEN": self.token},
                stderr=subprocess.STDOUT,
            )
        except subprocess.CalledProcessError as error:
            if b"HTTP 404" in error.output or b"HTTP 410" in error.output:
                return None
            raise
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            entry = archive.getinfo("image-receipt.json")
            if entry.file_size > 16384:
                raise ValueError("Image receipt is unexpectedly large")
            return json.loads(archive.read(entry))


def from_environment():
    return GitHub(os.environ["GITHUB_REPOSITORY"], os.environ["GH_TOKEN"])
