#!/usr/bin/env python3
"""Prepare a versioned Fuel cache at image build time; verify it offline at runtime."""

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import tempfile
import time
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def model_path(url):
    uri = urlsplit(url)
    parts = unquote(uri.path).strip("/").split("/")
    if (
        uri.scheme != "https"
        or uri.netloc not in ("fuel.gazebosim.org", "fuel.ignitionrobotics.org")
        or uri.query
        or uri.fragment
        or len(parts) != 5
        or parts[0] != "1.0"
        or parts[2] != "models"
        or not parts[4].isdigit()
        or int(parts[4]) < 1
        or any(p in ("", ".", "..") for p in parts)
    ):
        raise ValueError(f"Expected a pinned public Fuel model URL: {url}")
    return PurePosixPath(uri.netloc, parts[1].lower(), "models", parts[3].lower(), parts[4])


def hashes(root):
    if not root.is_dir() or root.is_symlink():
        raise RuntimeError(f"Missing asset directory: {root}")
    result = {}
    for path in sorted(root.rglob("*")):
        if path.is_symlink():
            raise RuntimeError(f"Unexpected asset symlink: {path}")
        if path.is_file():
            result[path.relative_to(root).as_posix()] = digest(path)
    return result


def verify_asset(cache, asset):
    relative = model_path(asset["url"])
    if str(relative) != asset["cache_path"]:
        raise ValueError("Fuel URL/cache path mismatch")
    if not asset.get("license", {}).get("url"):
        raise ValueError("Missing asset license attribution")
    if "model.sdf" not in asset["files"] or "model.config" not in asset["files"]:
        raise ValueError("Incomplete asset lock")
    actual = hashes(cache / relative)
    if actual != asset["files"]:
        changed = sorted(
            k
            for k in actual.keys() | asset["files"].keys()
            if actual.get(k) != asset["files"].get(k)
        )
        raise RuntimeError(f"Asset integrity mismatch: {relative}: {changed[:8]}")


def verify_dependencies(cache, assets):
    """Reject missing nested HTTP resources, including versioned material files."""
    locked = {a["cache_path"] for a in assets}
    for asset in assets:
        for sdf in (cache / asset["cache_path"]).rglob("*.sdf"):
            for element in ET.parse(sdf).iter():
                text = (element.text or "").strip()
                if not text.startswith(("https://fuel.", "http://fuel.")):
                    continue
                uri = urlsplit(text)
                parts = unquote(uri.path).strip("/").split("/")
                model_url = f"{uri.scheme}://{uri.netloc}/{'/'.join(parts[:5])}"
                relative = model_path(model_url)
                if str(relative) not in locked:
                    raise RuntimeError(f"Unpinned nested dependency: {text}")
                if len(parts) > 5:
                    if parts[5] != "files" or any(p in (".", "..") for p in parts[6:]):
                        raise RuntimeError(f"Invalid nested resource: {text}")
                    target = cache / relative / "/".join(parts[6:])
                    if not target.exists():
                        raise RuntimeError(f"Missing nested resource: {text}")


def prepare_asset(cache, asset):
    relative = model_path(asset["url"])
    target = cache / relative
    if target.exists():
        verify_asset(cache, asset)
        return
    # Retrying a build-time transfer is not retrying a failed simulation trial.
    # Isolated staging prevents a partial download from appearing ready.
    for attempt in range(1, 4):
        with tempfile.TemporaryDirectory(prefix="fuel-stage-", dir=cache.parent) as tmp:
            env = dict(os.environ, GZ_FUEL_CACHE_PATH=tmp)
            try:
                run = subprocess.run(
                    ["gz", "fuel", "download", "-v", "1", "-u", asset["url"]],
                    env=env,
                    capture_output=True,
                    text=True,
                    timeout=300,
                )
                if run.returncode:
                    raise RuntimeError(f"Fuel downloader exit {run.returncode}")
                verify_asset(Path(tmp), asset)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copytree(Path(tmp) / relative, target)
                print(f"Prepared {relative}", flush=True)
                return
            except (subprocess.TimeoutExpired, RuntimeError) as error:
                # Do not emit temporary signed referral URLs from Fuel logs.
                message = re.sub(r"https?://\S+\?\S+", "[URL query redacted]", str(error))
                print(f"Asset transfer {attempt}/3 failed: {relative}: {message}", flush=True)
                if attempt == 3:
                    raise
                time.sleep(5)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cache", required=True, type=Path)
    parser.add_argument("--lock", required=True, type=Path)
    parser.add_argument("--verify", action="store_true", help="No downloads or writes to cache")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    lock = json.loads(args.lock.read_text())
    if lock["schema"] != 1 or not lock["assets"]:
        raise ValueError("Unknown or empty Fuel asset lock")
    if not args.verify:
        args.cache.mkdir(parents=True, exist_ok=True)
    for asset in lock["assets"]:
        if not args.verify:
            prepare_asset(args.cache, asset)
        verify_asset(args.cache, asset)
    verify_dependencies(args.cache, lock["assets"])
    receipt = {
        "status": "PASS",
        "lock_sha256": digest(args.lock),
        "assets": len(lock["assets"]),
        "files": sum(len(a["files"]) for a in lock["assets"]),
        "nested_dependencies_present": True,
        "verification_downloads": 0,
    }
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt), flush=True)


if __name__ == "__main__":
    main()
