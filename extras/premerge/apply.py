#!/usr/bin/env python3
"""Apply only reviewed, hash-guarded POSIM dependency backports."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


BUNDLE = Path(__file__).resolve().parent
MARKER = "POSIM_BACKPORT.json"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def load_bundle(bundle=BUNDLE):
    manifest = json.loads((bundle / "manifest.json").read_text())
    if manifest["schema"] != 1:
        raise ValueError("Unknown backport manifest schema")
    for name in ("gst", "terrain"):
        spec = manifest[name]
        relative = Path(spec["path"])
        if relative.is_absolute() or ".." in relative.parts:
            raise ValueError("Invalid source path")
        for step in spec["steps"]:
            if Path(step["patch"]).name != step["patch"]:
                raise ValueError("Invalid patch path")
            if digest((bundle / step["patch"]).read_bytes()) != step["patch_sha256"]:
                raise ValueError("Patch checksum mismatch: " + step["patch"])
    return manifest


def transform(data, spec, bundle=BUNDLE):
    """Check and patch in staging; unknown sources never reach installed files."""
    current = digest(data)
    steps = spec["steps"]
    if current == steps[-1]["after"]:
        return data
    starts = [i for i, step in enumerate(steps) if current == step["before"]]
    if not starts:
        raise ValueError("Unsupported source SHA256 " + current + "; review upstream changes")
    with tempfile.TemporaryDirectory(prefix="posim-patch-") as temp:
        stage = Path(temp)
        target = stage / spec["path"]
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        for step in steps[starts[0] :]:
            if digest(target.read_bytes()) != step["before"]:
                raise ValueError("Discontinuous patch chain")
            command = ["git", "apply", "--no-index", "--whitespace=error"]
            patch = str((bundle / step["patch"]).resolve())
            subprocess.run(command + ["--check", patch], cwd=stage, check=True)
            subprocess.run(command + [patch], cwd=stage, check=True)
            if digest(target.read_bytes()) != step["after"]:
                raise ValueError("Patched source checksum mismatch")
        return target.read_bytes()


def apply_gst(source, spec, bundle=BUNDLE):
    target = source / spec["path"]
    if target.is_symlink():
        raise ValueError("Refusing a symlinked camera source")
    before = target.read_bytes()
    after = transform(before, spec, bundle)
    if before != after:
        # Same-directory replacement: no partly written C++ source on failure.
        with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as f:
            temporary = Path(f.name)
            f.write(after)
        try:
            shutil.copymode(target, temporary)
            os.replace(temporary, target)
        finally:
            temporary.unlink(missing_ok=True)
    return {
        "path": str(target),
        "before": digest(before),
        "after": digest(after),
        "changed": before != after,
    }


def inventory(root):
    return {
        str(p.relative_to(root)): digest(p.read_bytes())
        for p in sorted(root.rglob("*"))
        if p.is_file() and p.relative_to(root) != Path(MARKER)
    }


def apply_terrain(source, output, spec, bundle=BUNDLE):
    source = source.resolve(strict=True)
    if not output.is_absolute() or output.is_symlink():
        raise ValueError("Use an absolute, non-symlink overlay directory")
    output = output.resolve()
    if source == output or source in output.parents or output in source.parents:
        raise ValueError("Source and overlay must not overlap")
    original = (source / spec["path"]).read_bytes()
    patched = transform(original, spec, bundle)
    before = inventory(source)
    expected = dict(before)
    expected[spec["path"]] = digest(patched)
    receipt = {
        "upstream_pr": spec["upstream_pr"],
        "upstream_head": spec["upstream_head"],
        "source": str(source),
        "source_files": before,
        "overlay_files": expected,
    }
    if output.exists():
        if not (output / MARKER).is_file():
            raise ValueError("Existing output is not a managed POSIM overlay")
        if json.loads((output / MARKER).read_text()) != receipt or inventory(output) != expected:
            raise ValueError("Existing overlay changed; use a new output directory")
        return {"output": str(output), "changed": False}
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".posim-terra-", dir=output.parent) as tmp:
        stage = Path(tmp) / "resources"
        # Copy the complete resource tree, leaving all ROS-managed files intact.
        shutil.copytree(source, stage)
        (stage / spec["path"]).write_bytes(patched)
        if inventory(source) != before or inventory(stage) != expected:
            raise ValueError("Resource inventory changed during staging")
        (stage / MARKER).write_text(json.dumps(receipt, indent=2) + "\n")
        stage.rename(output)
    return {
        "output": str(output),
        "changed": True,
        "changed_resources": [p for p in before if before[p] != expected[p]],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    gst = sub.add_parser("gst")
    gst.add_argument("--source", type=Path, required=True)
    terrain = sub.add_parser("terrain")
    terrain.add_argument(
        "--source",
        type=Path,
        required=True,
        help="Installed gz-rendering resource root (contains ogre2/media)",
    )
    terrain.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    manifest = load_bundle()
    if args.mode == "gst":
        result = apply_gst(args.source, manifest["gst"])
    else:
        result = apply_terrain(args.source, args.output, manifest["terrain"])
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
