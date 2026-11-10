#!/usr/bin/env python3
"""Run one process with a verified Ogre overlay; never edit global shell settings."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--overlay", type=Path, required=True)
    parser.add_argument("--trace-loads", action="store_true")
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    manifest = json.loads((args.overlay / "manifest.json").read_text())
    engine = Path(manifest["engine_prefix"])
    library = Path(manifest["library"])
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        parser.error("This overlay profile supports only macOS arm64")
    if digest(library) != manifest["library_sha256"]:
        parser.error("Overlay library hash changed; refusing to launch")
    if digest(engine / "lib/libOgreNextMain.2.3.3.dylib") != manifest["installed_sha256"]:
        parser.error("Installed Ogre changed; rebuild and revalidate the overlay")
    if (
        digest(engine / "include/OGRE-Next/OgreBuildSettings.h")
        != manifest["build_settings_sha256"]
    ):
        parser.error("Installed Ogre configuration changed")
    command = args.command[1:] if args.command[:1] == ["--"] else args.command
    if not command:
        parser.error("Provide an executable after --")
    env = dict(os.environ)
    # Preserve caller-supplied POSIM/bridge/wave overlays, but never inject code.
    paths = [str(library.parent)]
    paths.extend(path for path in env.get("DYLD_LIBRARY_PATH", "").split(os.pathsep) if path)
    paths.append(str(engine / "lib"))
    env["DYLD_LIBRARY_PATH"] = os.pathsep.join(dict.fromkeys(paths))
    env.pop("DYLD_INSERT_LIBRARIES", None)
    if args.trace_loads:
        env["DYLD_PRINT_LIBRARIES"] = "1"
    os.execvpe(command[0], command, env)


if __name__ == "__main__":
    main()
