#!/usr/bin/env python3
"""Promote an exact, tested commit image without rebuilding or moving existing tags."""

import argparse
import json
import re
import subprocess

from image_provenance import verify_source

IMAGE = "ioeslab/posim"


def docker(*args):
    return subprocess.check_output(
        ["docker", "buildx", "imagetools", *args], text=True, stderr=subprocess.STDOUT
    ).strip()


def inspect_digest(image):
    digest = docker("inspect", image, "--format", "{{.Manifest.Digest}}")
    if not re.fullmatch(r"sha256:[0-9a-f]{64}", digest):
        raise ValueError(f"Invalid image digest for {image}: {digest}")
    return digest


def source_image(sha, api=None):
    if not re.fullmatch(r"[0-9a-f]{40}", sha):
        raise ValueError("A full source commit SHA is required")
    digest = inspect_digest(f"{IMAGE}:sha-{sha[:12]}")
    images = json.loads(docker("inspect", f"{IMAGE}@{digest}", "--format", "{{json .Image}}"))
    return {"digest": digest, **verify_source(images, sha, api)}


def source_digest(sha):
    return source_image(sha)["digest"]


def promote(version, sha):
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+(?:-[0-9A-Za-z][0-9A-Za-z.-]*)?", version):
        raise ValueError("Invalid release version")
    digest = source_digest(sha)
    target = f"{IMAGE}:{version}"
    try:
        existing = inspect_digest(target)
    except subprocess.CalledProcessError as error:
        if ": not found" not in error.output:
            raise
        docker("create", "--tag", target, f"{IMAGE}@{digest}")
    else:
        if existing != digest:
            raise ValueError(f"{target} already points to another digest")
    if inspect_digest(target) != digest:
        raise ValueError(f"{target} did not retain the source image digest")
    return digest


def promote_latest(version, sha):
    digest = source_digest(sha)
    if inspect_digest(f"{IMAGE}:{version}") != digest:
        raise ValueError("The published version alias differs from its source image")
    target = f"{IMAGE}:latest"
    try:
        existing = inspect_digest(target)
    except subprocess.CalledProcessError as error:
        if ": not found" not in error.output:
            raise
        existing = None
    if existing != digest:
        docker("create", "--tag", target, f"{IMAGE}@{digest}")
    if inspect_digest(target) != digest:
        raise ValueError("latest did not retain the published release digest")
    return digest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tag", required=True)
    parser.add_argument(
        "--latest", action="store_true", help="Also sync the confirmed latest release"
    )
    args = parser.parse_args()
    version = json.load(open("website/package.json", encoding="utf-8"))["version"]
    if args.tag != f"v{version}":
        raise ValueError("Release tag does not match website/package.json")
    sha = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    print(f"Published {IMAGE}:{version} at {promote(version, sha)}")
    if args.latest:
        print(f"Published {IMAGE}:latest at {promote_latest(version, sha)}")


if __name__ == "__main__":
    main()
