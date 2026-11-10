#!/usr/bin/env python3
"""Build an opt-in Ogre 2.3.3 core overlay without replacing the installed engine."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import signal
import subprocess
import time

PIN = "8d4daeaf46d7d8f85f1833f17daedd7dac05daec"
SOURCE_HASH = "ad2978d33dd289be1888f9ca38143655afbe6397be4a560784363736faf005a7"
HEADER_HASH = "b20e7576609f3743d0109e41db3c905b2864baf41174266d061a8493e6a1a0a8"


def digest(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def run(command, env, log, timeout=900):
    start = time.monotonic()
    with log.open("w") as stream:
        child = subprocess.Popen(
            command, env=env, stdout=stream, stderr=subprocess.STDOUT, start_new_session=True
        )
        try:
            while child.poll() is None:
                if time.monotonic() - start > timeout:
                    raise TimeoutError("Build step exceeded its time budget")
                if shutil.disk_usage(log.parent).free < 1024**3:
                    raise RuntimeError("Build stopped: free disk space fell below 1 GiB")
                time.sleep(0.25)
            if child.returncode:
                raise subprocess.CalledProcessError(child.returncode, command)
        finally:
            if child.poll() is None:
                os.killpg(child.pid, signal.SIGTERM)
                try:
                    child.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid, signal.SIGKILL)
                    child.wait()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-prefix", type=Path, required=True)
    parser.add_argument(
        "--source",
        type=Path,
        required=True,
        help="clean upstream Git checkout at the documented pinned commit",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="new, non-existing directory outside the installed engine",
    )
    parser.add_argument("--jobs", type=int, choices=(1, 2), default=2)
    args = parser.parse_args()
    engine, original, out = [p.resolve() for p in (args.engine_prefix, args.source, args.output)]
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        parser.error("This validated profile is macOS arm64 only; do not use it on Linux/Windows")
    if out.exists() or out.is_relative_to(engine) or out.is_relative_to(original):
        parser.error("Output must be new and outside the installed engine/source")
    installed = engine / "lib/libOgreNextMain.2.3.3.dylib"
    header = engine / "include/OGRE-Next/OgreBuildSettings.h"
    if not installed.is_file() or digest(header) != HEADER_HASH:
        parser.error("Unsupported Ogre version/build settings; refusing a mixed-ABI overlay")
    head = subprocess.check_output(
        ["git", "-C", str(original), "rev-parse", "HEAD"], text=True
    ).strip()
    dirty = subprocess.check_output(
        ["git", "-C", str(original), "status", "--porcelain"], text=True
    )
    if head != PIN or dirty or digest(original / "OgreMain/src/OgreRenderable.cpp") != SOURCE_HASH:
        parser.error("Ogre source must be pristine at the exact pinned revision")
    if shutil.disk_usage(out.parent).free < 3 * 1024**3:
        parser.error("At least 3 GiB of free space is required")
    before = digest(installed)
    out.mkdir()
    source, build = out / "source", out / "build"
    source.mkdir()
    for name in ("OgreMain", "CMake"):
        shutil.copytree(original / name, source / name)
    for name in ("CMakeLists.txt", "COPYING", "AUTHORS"):
        shutil.copy2(original / name, source / name)
    # Scope this overlay to OgreMain; all installed render systems remain unchanged.
    cmake = source / "CMakeLists.txt"
    text = cmake.read_text()
    marker = "# install resource files\n"
    if text.count(marker) != 1:
        raise RuntimeError("Upstream build layout changed")
    text = text.replace(
        "include(ConfigureBuild)",
        "# Match the installed conda build profile (ETC codec disabled).\nset(OGRE_CONFIG_ENABLE_ETC FALSE)\ninclude(ConfigureBuild)",
    )
    cmake.write_text(
        text.split(marker)[0]
        + "# Isolated core overlay; no install target is run.\nadd_subdirectory(OgreMain)\n"
    )
    env = dict(os.environ)
    env["CMAKE_PREFIX_PATH"] = str(engine)
    env["PKG_CONFIG_PATH"] = str(engine / "lib/pkgconfig")
    for key in ("CPATH", "LIBRARY_PATH", "CPLUS_INCLUDE_PATH", "DYLD_INSERT_LIBRARIES"):
        env.pop(key, None)
    patch = Path(__file__).resolve().with_name("xcode-compat.patch")
    subprocess.run(
        ["git", "apply", "--unidiff-zero", "--check", str(patch)], cwd=source, check=True
    )
    subprocess.run(["git", "apply", "--unidiff-zero", str(patch)], cwd=source, check=True)
    target = source / "OgreMain/src/OgreRenderable.cpp"
    text = target.read_text()
    needle = "        material->load();\n        mMaterial = material;"
    if text.count(needle) != 1:
        raise RuntimeError("Lifetime fix already present or source no longer matches")
    target.write_text(
        text.replace(
            needle,
            "        material->load();\n"
            "        // Keep the old low-level datablock alive until it is unlinked.\n"
            "        const MaterialPtr previousMaterial = mMaterial;\n"
            "        mMaterial = material;",
        )
    )
    flags = [
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        "-DCMAKE_CXX_STANDARD=11",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
        "-DCMAKE_CXX_FLAGS_RELEASE=-O1 -DNDEBUG -D__STDC_FORMAT_MACROS -Wno-elaborated-enum-base",
        "-DCMAKE_INSTALL_PREFIX=" + str(out / "unused-install"),
        "-DCMAKE_PREFIX_PATH=" + str(engine),
        "-DCMAKE_IGNORE_PREFIX_PATH=/opt/homebrew",
        "-DOGRE_USE_NEW_PROJECT_NAME=ON",
        "-DOGRE_BUILD_LIBS_AS_FRAMEWORKS=OFF",
        "-DOGRE_BUILD_RENDERSYSTEM_METAL=ON",
        "-DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON",
        "-DOGRE_BUILD_RENDERSYSTEM_GLES2=OFF",
        "-DOGRE_BUILD_SAMPLES2=OFF",
        "-DOGRE_BUILD_TESTS=OFF",
        "-DOGRE_BUILD_TOOLS=OFF",
        "-DOGRE_USE_BOOST=OFF",
        "-DOGRE_CONFIG_THREADS=0",
        "-DOGRE_CONFIG_THREAD_PROVIDER=std",
        "-DOGRE_SIMD_SSE2=OFF",
        "-DOGRE_SIMD_NEON=ON",
        "-DOGRE_CONFIG_ENABLE_ETC=OFF",
        "-DOGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT=OFF",
        "-DOGRE_BUILD_COMPONENT_HLMS_PBS=ON",
        "-DOGRE_BUILD_COMPONENT_HLMS_UNLIT=ON",
        "-DOGRE_BUILD_COMPONENT_OVERLAY=ON",
        "-DOGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS=ON",
        "-DOGRE_BUILD_COMPONENT_SCENE_FORMAT=ON",
        "-DOGRE_BUILD_COMPONENT_MESHLODGENERATOR=ON",
        "-DOGRE_CONFIG_ENABLE_FREEIMAGE=ON",
        "-DOGRE_CONFIG_ENABLE_TBB_SCHEDULER=ON",
        "-DOGRE_CONFIG_ENABLE_ZIP=ON",
        "-DOGRE_CONFIG_ENABLE_JSON=ON",
        "-DOGRE_IDSTRING_ALWAYS_READABLE=OFF",
        "-DOGRE_GLSUPPORT_USE_COCOA=ON",
        "-DFreeImage_INCLUDE_DIR=" + str(engine / "include"),
        "-DFreeImage_LIBRARY_REL=" + str(engine / "lib/libfreeimage.3.dylib"),
        "-DFREETYPE_INCLUDE_DIR=" + str(engine / "include/freetype2"),
        "-DFREETYPE_FT2BUILD_INCLUDE_DIR=" + str(engine / "include/freetype2"),
        "-DFREETYPE_LIBRARY_REL=" + str(engine / "lib/libfreetype.dylib"),
        "-DZLIB_INCLUDE_DIR=" + str(engine / "include"),
        "-DZLIB_LIBRARY_REL=" + str(engine / "lib/libz.1.dylib"),
        "-DZZip_INCLUDE_DIR=" + str(engine / "include"),
        "-DZZip_LIBRARY_REL=" + str(engine / "lib/libzzip-0.13.dylib"),
    ]
    start = time.monotonic()
    run(["cmake", "-S", str(source), "-B", str(build), *flags], env, out / "configure.log")
    generated = build / "include/OgreBuildSettings.h"
    if generated.read_bytes() != header.read_bytes():
        raise RuntimeError("Generated OgreBuildSettings differs; build stopped before replacement")
    run(
        ["cmake", "--build", str(build), "--target", "OgreNextMain", "-j" + str(args.jobs)],
        env,
        out / "build.log",
    )
    library = build / "lib/macosx/libOgreNextMain.2.3.3.dylib"
    if not library.is_file():
        raise RuntimeError("Expected core library not produced")
    lib = out / "lib"
    lib.mkdir()
    shutil.copy2(library, lib / library.name)
    # Use the same install name as the installed ABI; the wrapper selects this directory.
    subprocess.run(
        ["install_name_tool", "-id", "@rpath/" + library.name, str(lib / library.name)], check=True
    )
    subprocess.run(["codesign", "--force", "--sign", "-", str(lib / library.name)], check=True)
    if digest(installed) != before:
        raise RuntimeError("Installed core changed during the build")
    manifest = {
        "profile": "conda-osx-arm64-ogre-next-2.3.3",
        "source_commit": PIN,
        "upstream_pr": "https://github.com/OGRECave/ogre-next/pull/598",
        "engine_prefix": str(engine),
        "installed_sha256": before,
        "build_settings_sha256": digest(header),
        "library": str(lib / library.name),
        "library_sha256": digest(lib / library.name),
        "xcode_patch_sha256": digest(patch),
        "seconds": time.monotonic() - start,
        "no_global_install": True,
        "runtime_verified": False,
    }
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(manifest), flush=True)


if __name__ == "__main__":
    main()
