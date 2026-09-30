"""Capture a camera startup stall after the original readiness deadline expires.

Runs only in a disposable diagnostic container. Additional verbosity and GDB
attachment make these diagnostic observations, never acceptance replacements.
"""

import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import sys


def descendants(parent):
    records = {}
    for folder in Path("/proc").glob("[0-9]*"):
        try:
            status = (folder / "status").read_text()
            ppid = int(
                next(line.split()[1] for line in status.splitlines() if line.startswith("PPid:"))
            )
            command = (
                (folder / "cmdline").read_bytes().replace(b"\0", b" ").decode(errors="replace")
            )
            records[int(folder.name)] = {"ppid": ppid, "command": command}
        except (OSError, StopIteration, ProcessLookupError):
            continue
    found = {parent}
    while True:
        added = {pid for pid, rec in records.items() if rec["ppid"] in found} - found
        if not added:
            break
        found.update(added)
    return {pid: records[pid] for pid in found if pid != parent and pid in records}


def capture_stall(parent, output):
    processes = descendants(parent)
    (output / "processes-at-deadline.json").write_text(json.dumps(processes, indent=2))
    targets = [pid for pid, rec in processes.items() if "gz-sim-main" in rec["command"]]
    captures = []
    # Prevent launch from escalating signals while the failed server is inspected.
    try:
        os.kill(parent, signal.SIGSTOP)
    except ProcessLookupError:
        return captures
    try:
        for pid in targets:
            for item in ("maps", "status", "wchan"):
                try:
                    (output / f"proc-{pid}-{item}.txt").write_text(
                        Path(f"/proc/{pid}/{item}").read_text()
                    )
                except OSError:
                    pass
            with (output / f"gdb-{pid}.log").open("w") as log:
                try:
                    result = subprocess.run(
                        [
                            "gdb",
                            "-q",
                            "-batch",
                            "-iex",
                            "set debuginfod enabled off",
                            "-p",
                            str(pid),
                            "-ex",
                            "set pagination off",
                            "-ex",
                            "thread apply all bt",
                            "-ex",
                            "info sharedlibrary",
                            "-ex",
                            "detach",
                        ],
                        stdout=log,
                        stderr=subprocess.STDOUT,
                        timeout=30,
                    )
                    captures.append({"pid": pid, "returncode": result.returncode})
                except subprocess.TimeoutExpired:
                    captures.append({"pid": pid, "error": "GDB timed out"})
    finally:
        try:
            os.kill(parent, signal.SIGCONT)
        except ProcessLookupError:
            pass
    return captures


def main():
    spec = importlib.util.spec_from_file_location("image_smoke", "/checks/image_smoke.py")
    smoke = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(smoke)
    original_wait = smoke.wait_for_entity
    original_exercise = smoke.exercise
    captured = []

    def traced_wait(out, world, entity, proc, deadline):
        ready = original_wait(out, world, entity, proc, deadline)
        if not ready and proc.poll() is None:
            captured.extend(capture_stall(proc.pid, out))
        return ready

    def verbose_exercise(out, case):
        case = list(case)
        case[1] += " debug:=true verbosity_level:=4"
        return original_exercise(out, case)

    smoke.wait_for_entity = traced_wait
    smoke.exercise = verbose_exercise
    sys.argv = ["image_smoke.py", "camera", "--record", "camera-readiness"]
    status = smoke.main()
    cache = Path.home() / ".gz/fuel"
    files = []
    if cache.exists():
        for path in cache.rglob("*"):
            if path.is_file():
                files.append({"path": str(path.relative_to(cache)), "bytes": path.stat().st_size})
    Path("/results/fuel-cache-files.json").write_text(json.dumps(files, indent=2))
    Path("/results/camera-readiness-summary.json").write_text(
        json.dumps(
            {"diagnostic_status": status, "captures": captured, "scope": "Not acceptance"},
            indent=2,
        )
    )
    return status


if __name__ == "__main__":
    raise SystemExit(main())
