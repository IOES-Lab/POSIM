#!/usr/bin/env python3
"""Opt-in POSIX integration test: interrupt a dynamically spawned sonar during startup.

Needs sourced Gazebo/plugin paths and a rendering device. Example:
  python3 test_early_shutdown.py --backend wgpu --output /tmp/sonar-shutdown
The test only signals process groups it starts. No system-wide process cleanup.
"""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time
import uuid
import xml.etree.ElementTree as ET


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend", choices=("cpu", "wgpu", "cuda"), default="cpu")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=2)
    parser.add_argument("--sensor-system-plugin", type=Path)
    parser.add_argument(
        "--render-backend",
        choices=("metal", "headless"),
        default="metal" if sys.platform == "darwin" else "headless",
    )
    args = parser.parse_args()
    rendering = (
        ["--render-engine-server-api-backend", "metal"]
        if args.render_backend == "metal"
        else ["--headless-rendering"]
    )
    if args.repeats < 1:
        parser.error("--repeats must be positive")
    args.output.mkdir(parents=True, exist_ok=False)
    fixtures = Path(__file__).parent
    world = fixtures / "shutdown_world.sdf"
    if args.sensor_system_plugin:
        if not args.sensor_system_plugin.is_file():
            parser.error("--sensor-system-plugin must name an existing library")
        root = ET.parse(world).getroot()
        root.find("./world/plugin[@name='gz::sim::systems::Sensors']").set(
            "filename", str(args.sensor_system_plugin.resolve())
        )
        world = args.output.resolve() / "world.sdf"
        world.write_text(ET.tostring(root, encoding="unicode"))
    results = []
    for repetition in range(args.repeats):
        for sig in (signal.SIGINT, signal.SIGTERM):
            for delay in (0.0, 0.005, 0.05):
                out = args.output / f"{repetition}_{int(sig)}_{delay}"
                out.mkdir()
                env = dict(
                    os.environ,
                    GZ_PARTITION="shutdown_" + uuid.uuid4().hex,
                    POSIM_SONAR_COMPUTE_BACKEND=args.backend,
                    ROS_LOG_DIR=str(out.resolve() / "ros_logs"),
                )
                row = dict(
                    backend=args.backend,
                    repetition=repetition,
                    signal=int(sig),
                    delay_s=delay,
                    passed=False,
                )
                proc = None
                try:
                    with (out / "gazebo.log").open("w") as log:
                        proc = subprocess.Popen(
                            [
                                "gz",
                                "sim",
                                "-s",
                                "-r",
                                *rendering,
                                "-v",
                                "4",
                                str(world),
                            ],
                            env=env,
                            cwd=out,
                            stdout=log,
                            stderr=log,
                            start_new_session=True,
                        )
                        deadline = time.monotonic() + 60
                        while time.monotonic() < deadline:
                            assert proc.poll() is None, "Gazebo exited before spawning"
                            text = (out / "gazebo.log").read_text(errors="replace")
                            if "Serving world controls" in text:
                                break
                            time.sleep(0.02)
                        else:
                            raise AssertionError("World readiness marker not seen")
                        reply = subprocess.run(
                            [
                                "gz",
                                "service",
                                "-s",
                                "/world/default/create",
                                "--reqtype",
                                "gz.msgs.EntityFactory",
                                "--reptype",
                                "gz.msgs.Boolean",
                                "--timeout",
                                "10000",
                                "--req",
                                f'sdf_filename: "{fixtures / "shutdown_sonar.sdf"}", '
                                'name: "blueview_p900", allow_renaming: false',
                            ],
                            env=env,
                            capture_output=True,
                            text=True,
                            timeout=15,
                            check=True,
                        )
                        assert "data: true" in reply.stdout, reply.stdout + reply.stderr
                        deadline = time.monotonic() + 120
                        while time.monotonic() < deadline:
                            assert proc.poll() is None, "Gazebo exited before signal"
                            if "Using sonar compute backend:" in (out / "gazebo.log").read_text(
                                errors="replace"
                            ):
                                break
                            time.sleep(0.002)
                        else:
                            raise AssertionError("Sonar startup marker not seen")
                        time.sleep(delay)
                        start = time.monotonic()
                        os.killpg(proc.pid, sig)
                        proc.wait(timeout=20)
                        row["shutdown_s"] = time.monotonic() - start
                        assert proc.returncode == 0, f"Exit {proc.returncode}"
                        text = (out / "gazebo.log").read_text(errors="replace")
                        assert not any(
                            message in text
                            for message in (
                                "context is not valid",
                                "context is invalid",
                                "context argument is null",
                                "mutex lock failed",
                            )
                        )
                        row["passed"] = True
                finally:
                    if proc is not None and proc.poll() is None:
                        os.killpg(proc.pid, signal.SIGKILL)
                        proc.wait()
                        row["forced_cleanup"] = True
                    if proc is not None:
                        row["exit"] = proc.returncode
                    results.append(row)
                    (args.output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
                    print(json.dumps(row), flush=True)


if __name__ == "__main__":
    main()
