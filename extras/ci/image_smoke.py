#!/usr/bin/env python3
"""Check installed POSIM resources and headless Quickstarts inside an image."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shlex
import signal
import subprocess
import time


PACKAGES = (
    "dave_interfaces dave_demos dave_gz_model_plugins dave_gz_sensor_plugins "
    "dave_gz_world_plugins dave_ros_gz_plugins dave_object_models dave_robot_models "
    "dave_sensor_models dave_worlds multibeam_sonar multibeam_sonar_system "
    "dave_multibeam_sonar_demo"
).split()


def command(args, timeout=15):
    try:
        result = subprocess.run(args, capture_output=True, text=True, timeout=timeout)
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return 124, "", f"Timed out after {timeout}s: {shlex.join(args)}"


def capture(out, name, args, timeout=15):
    code, stdout, stderr = command(args, timeout)
    # Camera payloads can be large. Retain a bounded excerpt and its full digest.
    (out / f"{name}.txt").write_text(stdout[:65536])
    (out / f"{name}.stderr").write_text(stderr[:65536])
    (out / f"{name}.json").write_text(
        json.dumps(
            {
                "command": args,
                "returncode": code,
                "stdout_bytes": len(stdout.encode()),
                "stdout_sha256": hashlib.sha256(stdout.encode()).hexdigest(),
            },
            indent=2,
        )
    )
    return code, stdout


def inventory(out):
    from ament_index_python.packages import get_package_share_directory

    shares = {name: Path(get_package_share_directory(name)) for name in PACKAGES}
    for share in shares.values():
        if not (share / "package.xml").is_file():
            raise RuntimeError(f"Installed package.xml missing: {share}")
    launches = [
        shares["dave_demos"] / "launch" / f"dave_{kind}.launch.py"
        for kind in ("world", "robot", "sensor", "object")
    ]
    for launch in launches:
        if not launch.is_file():
            raise RuntimeError(f"Installed launch missing: {launch}")
        code, _ = capture(
            out, launch.stem, ["ros2", "launch", "dave_demos", launch.name, "--show-args"], 30
        )
        if code:
            raise RuntimeError(f"Cannot resolve launch arguments: {launch.name}")
    worlds = sorted((shares["dave_worlds"] / "worlds").glob("*.world"))
    robots = sorted((shares["dave_robot_models"] / "description").glob("*/model.sdf"))
    sensors = sorted((shares["dave_sensor_models"] / "description").glob("*/model.sdf"))
    counts = {"worlds": len(worlds), "robots": len(robots), "sensors": len(sensors)}
    if counts != {"worlds": 18, "robots": 5, "sensors": 11}:
        raise RuntimeError(f"Unexpected installed resource inventory: {counts}")
    capture(out, "deb_versions", ["dpkg-query", "-W", "ros-lyrical-*"], 30)
    capture(out, "gazebo_version", ["gz", "sim", "--versions"])
    ws = Path(os.environ["POSIM_WORKSPACE"])
    for companion in ("dockwater", "rocker"):
        code, _ = capture(
            out,
            f"revision_{companion}",
            [
                "git",
                "-c",
                f"safe.directory={ws / 'src' / companion}",
                "-C",
                str(ws / "src" / companion),
                "rev-parse",
                "HEAD",
            ],
        )
        if code:
            raise RuntimeError(f"Cannot record companion revision: {companion}")
    return {"shares": {k: str(v) for k, v in shares.items()}, "counts": counts}


def exercise(out, case):
    _, launch_command, entity, topic_pattern = case
    checks = {}
    proc = None
    forced = False
    returncode = None
    with (out / "launch.log").open("w") as log:
        try:
            proc = subprocess.Popen(
                shlex.split(launch_command),
                stdout=log,
                stderr=subprocess.STDOUT,
                start_new_session=True,
            )
            deadline = time.monotonic() + 90
            world = None
            while time.monotonic() < deadline and proc.poll() is None:
                code, services, _ = command(["gz", "service", "-l"], 8)
                match = re.search(r"^/world/([^/]+)/control$", services, re.M)
                if code == 0 and match:
                    world = match[1]
                    break
                time.sleep(1)
            checks["world_ready"] = world is not None
            if world:
                time.sleep(8)
                code, stats = capture(
                    out,
                    "world_stats",
                    ["gz", "topic", "-e", "-t", f"/world/{world}/stats", "-n", "2"],
                    20,
                )
                iterations = [int(n) for n in re.findall(r"iterations:\s*(\d+)", stats)]
                checks["simulation_advances"] = (
                    code == 0 and len(iterations) >= 2 and iterations[-1] > iterations[0]
                )
                code, poses = capture(
                    out,
                    "poses",
                    ["gz", "topic", "-e", "-t", f"/world/{world}/pose/info", "-n", "1"],
                    20,
                )
                checks["entity_present"] = entity == "__NONE__" or (
                    code == 0
                    and re.search(r'name:\s*"' + re.escape(entity) + r'"', poses) is not None
                )
                # A topic listing is not sufficient: obtain a real message.
                if topic_pattern.startswith("/world/") and topic_pattern.endswith("/"):
                    checks["expected_world"] = (
                        re.search(topic_pattern, f"/world/{world}/stats") is not None
                    )
                else:
                    payload = ""
                    payload_code = 1
                    deadline = time.monotonic() + 60
                    while time.monotonic() < deadline and proc.poll() is None:
                        _, topics, _ = command(["gz", "topic", "-l"], 8)
                        choices = [t for t in topics.splitlines() if re.search(topic_pattern, t)]
                        if choices:
                            payload_code, payload = capture(
                                out,
                                "gz_payload",
                                ["gz", "topic", "-e", "-t", choices[0], "-n", "1"],
                                15,
                            )
                            if payload_code == 0 and payload.strip():
                                break
                        time.sleep(1)
                    checks["gazebo_payload"] = payload_code == 0 and bool(payload.strip())
                if case[0] in ("rexrov_empty", "rexrov_waves", "dvl"):
                    ros_topic = "/dvl/velocity" if case[0] == "dvl" else "/model/rexrov/odometry"
                    code, payload = capture(
                        out,
                        "ros_payload",
                        [
                            "ros2",
                            "topic",
                            "echo",
                            ros_topic,
                            "--once",
                            "--qos-reliability",
                            "best_effort",
                        ],
                        40,
                    )
                    checks["ros_payload"] = code == 0 and "header:" in payload
                capture(out, "ros_topics", ["ros2", "topic", "list", "-t"], 20)
                checks["launch_alive"] = proc.poll() is None
        finally:
            if proc is not None:
                # Limit shutdown signals to this test's new process group.
                try:
                    os.killpg(proc.pid, signal.SIGINT)
                except ProcessLookupError:
                    pass
                try:
                    returncode = proc.wait(timeout=25)
                except subprocess.TimeoutExpired:
                    forced = True
                    os.killpg(proc.pid, signal.SIGKILL)
                    returncode = proc.wait(timeout=10)
    checks["clean_shutdown"] = not forced and returncode in (0, 130, -signal.SIGINT)
    log_text = (out / "launch.log").read_text(errors="replace")
    faults = re.findall(
        r".*(?:Segmentation fault|exit code -11|exit code -6|Traceback \(most recent call last\)|"
        r"Failed to load system plugin|error while loading shared libraries).*",
        log_text,
    )
    checks["no_fatal_log"] = not faults
    return {
        "checks": checks,
        "shutdown_returncode": returncode,
        "faults": faults,
        "command": shlex.split(launch_command),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("case")
    args = parser.parse_args()
    out = Path("/results") / args.case
    out.mkdir(parents=True, exist_ok=True)
    result = {"case": args.case, "architecture": platform.machine(), "status": "FAIL"}
    try:
        if args.case == "inventory":
            result.update(inventory(out))
        else:
            cases = [
                line.split("\t")
                for line in Path(__file__).with_name("quickstarts.tsv").read_text().splitlines()
                if line and not line.startswith("#")
            ]
            case = next(c for c in cases if c[0] == args.case)
            result.update(exercise(out, case))
            if not all(result["checks"].values()):
                raise RuntimeError("One or more runtime checks failed")
        result["status"] = "PASS"
    except Exception as error:
        result["error"] = str(error)
    (out / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result), flush=True)
    return 0 if result["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
