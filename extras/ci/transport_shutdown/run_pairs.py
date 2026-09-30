#!/usr/bin/env python3
"""Collect matched transport churn trials without reclassifying failures as passes."""

import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shutil
import signal
import subprocess
import time
import uuid


COUNTS = re.compile(r"rounds=(\d+) anchor_received=(\d+) churn_received=(\d+)")


def trial_passed(publisher_rc, subscriber_rc, timeouts, publisher_log, subscriber_log):
    pub = COUNTS.search(publisher_log)
    sub = COUNTS.search(subscriber_log)
    return bool(
        publisher_rc == 0
        and subscriber_rc == 0
        and not timeouts
        and pub
        and sub
        and int(pub[1]) > 0
        and all(int(sub[i]) > 0 for i in (1, 2, 3))
    )


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def stop_process(process):
    if process.poll() is None:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", required=True)
    parser.add_argument("--variant", action="append", required=True, help="name:library-directory")
    parser.add_argument("--trials", type=int, default=5)
    parser.add_argument("--seconds", type=int, default=20)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    if not 1 <= args.trials <= 20 or not 3 <= args.seconds <= 300:
        parser.error("trials must be 1..20 and seconds 3..300")
    variants = [item.split(":", 1) for item in args.variant]
    if any(len(v) != 2 or not re.fullmatch(r"[a-zA-Z0-9_-]+", v[0]) for v in variants):
        parser.error("each variant must be name:library-directory")
    if len({v[0] for v in variants}) != len(variants):
        parser.error("variant names must be unique")
    root = Path(args.output).resolve()
    root.mkdir(parents=True, exist_ok=False)
    executable = str(Path(args.executable).resolve(strict=True))
    records = []
    captured = set()
    metadata = {
        "scope": "Diagnostic comparison only; not final-image acceptance",
        "trials_per_variant": args.trials,
        "seconds_per_trial": args.seconds,
        "architecture": os.uname().machine,
        "probe_sha256": sha256(executable),
    }
    (root / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    for number in range(1, args.trials + 1):
        for name, library in variants:
            directory = root / f"{name}-{number}"
            directory.mkdir()
            env = os.environ.copy()
            env["GZ_PARTITION"] = "posim-" + uuid.uuid4().hex
            env["DEBUGINFOD_URLS"] = ""
            if library:
                env["LD_LIBRARY_PATH"] = library + ":" + env.get("LD_LIBRARY_PATH", "")
            linked = subprocess.check_output(["ldd", executable], env=env, text=True)
            (directory / "ldd.txt").write_text(linked)
            line = next(x for x in linked.splitlines() if "libgz-transport.so.15 =>" in x)
            loaded = line.split("=>")[1].split()[0]
            if library and Path(loaded).resolve().parent != Path(library).resolve():
                raise RuntimeError(f"Wrong transport library: {line}")

            def configure_core():
                hard = resource.getrlimit(resource.RLIMIT_CORE)[1]
                size = hard if name not in captured else 0
                resource.setrlimit(resource.RLIMIT_CORE, (size, hard))

            timeouts = []
            with (directory / "publisher.log").open("w") as pub_log, (
                directory / "subscriber.log"
            ).open("w") as sub_log:
                publisher = subprocess.Popen(
                    [executable, "publish", str(args.seconds)],
                    env=env,
                    stdout=pub_log,
                    stderr=subprocess.STDOUT,
                    start_new_session=True,
                )
                subscriber = None
                try:
                    subscriber = subprocess.Popen(
                        [executable, "subscribe", str(args.seconds)],
                        env=env,
                        stdout=sub_log,
                        stderr=subprocess.STDOUT,
                        cwd=directory,
                        start_new_session=True,
                        preexec_fn=configure_core,
                    )
                    deadline = time.monotonic() + args.seconds + 10
                    for label, process in (("subscriber", subscriber), ("publisher", publisher)):
                        try:
                            process.wait(timeout=max(0.1, deadline - time.monotonic()))
                        except subprocess.TimeoutExpired:
                            timeouts.append(label)
                            stop_process(process)
                finally:
                    stop_process(publisher)
                    if subscriber is not None:
                        stop_process(subscriber)
            record = {
                "variant": name,
                "trial": number,
                "publisher_rc": publisher.returncode,
                "subscriber_rc": subscriber.returncode,
                "timeouts": timeouts,
                "library": loaded,
                "library_sha256": sha256(loaded),
                "partition": env["GZ_PARTITION"],
            }
            record["pass"] = trial_passed(
                publisher.returncode,
                subscriber.returncode,
                timeouts,
                (directory / "publisher.log").read_text(errors="replace"),
                (directory / "subscriber.log").read_text(errors="replace"),
            )
            core = directory / "core"
            if core.exists():
                captured.add(name)
                record["core_sha256"] = sha256(core)
                with (directory / "core-stack.log").open("w") as stack:
                    try:
                        result = subprocess.run(
                            [
                                "gdb",
                                "--batch",
                                "-iex",
                                "set debuginfod enabled off",
                                executable,
                                str(core),
                                "-ex",
                                "set pagination off",
                                "-ex",
                                "thread apply all bt",
                                "-ex",
                                "info sharedlibrary",
                            ],
                            env=env,
                            stdout=stack,
                            stderr=subprocess.STDOUT,
                            timeout=30,
                        )
                        record["gdb_returncode"] = result.returncode
                    except (subprocess.TimeoutExpired, FileNotFoundError) as error:
                        record["gdb_error"] = str(error)
                with core.open("rb") as src, gzip.open(directory / "core.gz", "wb") as dst:
                    shutil.copyfileobj(src, dst)
                core.unlink()
            records.append(record)
            (root / "results.json").write_text(json.dumps(records, indent=2) + "\n")
            print(json.dumps(record), flush=True)
    summary = {
        name: {
            "trials": sum(r["variant"] == name for r in records),
            "passed": sum(r["variant"] == name and r["pass"] for r in records),
            "sigsegv": sum(r["variant"] == name and r["subscriber_rc"] == -11 for r in records),
        }
        for name, _ in variants
    }
    (root / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary), flush=True)
    if "patched" in summary and summary["patched"]["passed"] != args.trials:
        raise SystemExit("Patched diagnostic failed; all trial records were retained")


if __name__ == "__main__":
    main()
