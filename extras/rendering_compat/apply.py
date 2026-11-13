#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Check or apply pinned source-only dynamic-mesh compatibility patches.

No installed library, environment, image, or system package is changed.
Use fresh external source checkouts. Checking is the default; --apply is explicit.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent


def sha(data):
    return hashlib.sha256(data).hexdigest()


def prepare(root, spec, patch_dir=HERE):
    """Prepare one file without writing its checkout; reject unfamiliar bytes."""
    root = Path(root).resolve(strict=True)
    relative = Path(spec["path"])
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError("Source path must be relative and remain inside its checkout")
    target = root / relative
    if target.is_symlink() or not target.resolve(strict=True).is_relative_to(root):
        raise ValueError(f"Refusing source link outside the checkout: {target}")
    before = target.read_bytes()
    digest = sha(before)
    if digest == spec["after_sha256"]:
        return target, before, before
    if digest != spec["before_sha256"]:
        raise ValueError(f"Unreviewed source bytes: {target} ({digest})")
    patch = Path(patch_dir) / spec["patch"]
    if sha(patch.read_bytes()) != spec["patch_sha256"]:
        raise ValueError(f"Patch checksum mismatch: {patch}")
    with tempfile.TemporaryDirectory(prefix="posim-rendering-patch-") as temp:
        staged = Path(temp) / relative
        staged.parent.mkdir(parents=True)
        staged.write_bytes(before)
        subprocess.run(
            ["git", "apply", "--whitespace=error-all", str(patch.resolve())],
            cwd=temp,
            check=True,
            capture_output=True,
        )
        after = staged.read_bytes()
    if sha(after) != spec["after_sha256"]:
        raise ValueError(f"Unexpected patched source: {target}")
    return target, before, after


def apply_all(plans):
    """Reject concurrent edits before writing; each file replacement is atomic.

    This is not a cross-file transaction across power loss. Rerunning is idempotent.
    """
    for target, before, _ in plans:
        if target.read_bytes() != before:
            raise ValueError(f"Source changed after preflight: {target}")
    for target, before, after in plans:
        if before == after:
            continue
        name = None
        try:
            with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as f:
                name = Path(f.name)
                f.write(after)
            os.chmod(name, target.stat().st_mode & 0o777)
            os.replace(name, target)
        finally:
            if name and name.exists():
                name.unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rendering-source", type=Path, required=True)
    parser.add_argument("--waves-source", type=Path, required=True)
    parser.add_argument("--apply", action="store_true")
    args = parser.parse_args()
    specs = json.loads((HERE / "manifest.json").read_text())["components"]
    roots = {"rendering": args.rendering_source, "waves": args.waves_source}
    # Preflight both components before modifying either source checkout.
    plans = [prepare(roots[spec["name"]], spec) for spec in specs]
    if args.apply:
        apply_all(plans)
    for target, before, after in plans:
        status = "already patched" if before == after else ("applied" if args.apply else "ready")
        print(f"{status}: {target} -> {sha(after)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        raise SystemExit(f"Compatibility patch rejected: {error}")
