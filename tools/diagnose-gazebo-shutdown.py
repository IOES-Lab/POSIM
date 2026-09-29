"""Capture a slow Gazebo shutdown in a disposable diagnostic container.

Pausing launch temporarily prevents its SIGTERM escalation from destroying the
stack while GDB attaches. These instrumented runs are NOT acceptance trials.
"""

import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import threading


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
        except (OSError, StopIteration):
            continue
    found = {parent}
    while True:
        added = {pid for pid, rec in records.items() if rec["ppid"] in found} - found
        if not added:
            break
        found.update(added)
    return {pid: records[pid] for pid in found if pid != parent and pid in records}


original_wait = subprocess.Popen.wait
original_killpg = os.killpg
captured = []
current_output = None


def delayed_stack(parent, done, output):
    if done.wait(3):
        return
    processes = descendants(parent)
    targets = [
        pid
        for pid, rec in processes.items()
        if "gz sim" in rec["command"] or "gz-sim" in rec["command"]
    ]
    if not targets:
        return
    try:
        os.kill(parent, signal.SIGSTOP)
    except ProcessLookupError:
        return
    try:
        output.mkdir(parents=True, exist_ok=True)
        (output / "processes-at-3s.json").write_text(json.dumps(processes, indent=2))
        for pid in targets:
            with (output / f"gdb-{pid}.log").open("w") as log:
                result = subprocess.run(
                    [
                        "gdb",
                        "-q",
                        "-batch",
                        "-p",
                        str(pid),
                        "-ex",
                        "set pagination off",
                        "-ex",
                        "thread apply all bt",
                        "-ex",
                        "detach",
                    ],
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    timeout=15,
                )
                captured.append(
                    {"pid": pid, "gdb_status": result.returncode, "record": str(output)}
                )
    except subprocess.TimeoutExpired:
        (output / "gdb-timeout.txt").write_text("Debugger attachment timed out.\n")
    finally:
        try:
            os.kill(parent, signal.SIGCONT)
        except ProcessLookupError:
            pass


active = {}


def traced_killpg(pid, sig):
    original_killpg(pid, sig)
    if sig == signal.SIGINT and current_output is not None:
        done = threading.Event()
        worker = threading.Thread(target=delayed_stack, args=(pid, done, current_output))
        active[pid] = (done, worker)
        worker.start()


def traced_wait(proc, timeout=None):
    try:
        return original_wait(proc, timeout)
    finally:
        if proc.pid in active:
            done, worker = active.pop(proc.pid)
            done.set()
            worker.join(timeout=20)


spec = importlib.util.spec_from_file_location("image_smoke", "/checks/image_smoke.py")
smoke = importlib.util.module_from_spec(spec)
spec.loader.exec_module(smoke)
os.killpg = traced_killpg
subprocess.Popen.wait = traced_wait
for n in range(1, 21):
    record = f"gazebo-shutdown-{n}"
    current_output = Path("/results") / record
    sys.argv = ["image_smoke.py", "spherical_world", "--record", record]
    smoke.main()
    if captured:
        break
Path("/results/gazebo-stack-captures.json").write_text(json.dumps(captured, indent=2))
Path("/results/diagnostic-outcome.txt").write_text(
    f"Captured {len(captured)} slow-shutdown process stacks; diagnostic only, not acceptance.\n"
)
