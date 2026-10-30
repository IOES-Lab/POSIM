#!/usr/bin/env python3
"""Opt-in POSIX sonar regression (ROS, Gazebo and a rendering device required).

Source ROS and the plugin install, then run:
  python3 test_empty_return_display.py --backend cpu --output /tmp/empty-display
Repeat with --backend wgpu when available. The output directory must be new.
Check 25 timestamp-matched raw/image pairs in each phase, including recovery.
The small 33-beam fixture isolates display behavior from large-message transport.
This checks display behavior, not acoustic accuracy or full numerical equivalence.
"""

import argparse
import array
import gzip
import hashlib
import json
import math
import os
from pathlib import Path
import shlex
import signal
import struct
import subprocess
import sys
import time
import uuid
import zlib
import xml.etree.ElementTree as ET

import rclpy
from rclpy.qos import DurabilityPolicy, QoSProfile, ReliabilityPolicy
from rclpy.serialization import serialize_message
from marine_acoustic_msgs.msg import ProjectedSonarImage
from sensor_msgs.msg import Image


def stamp(message):
    return message.header.stamp.sec * 10**9 + message.header.stamp.nanosec


def save_png(message, path):
    def chunk(tag, data):
        return (
            struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data))
        )

    data = bytes(message.data)
    rows = []
    for y in range(message.height):
        row = data[y * message.step : y * message.step + message.width * 3]
        rgb = bytearray(len(row))
        rgb[0::3], rgb[1::3], rgb[2::3] = row[2::3], row[1::3], row[0::3]
        rows.append(b"\0" + rgb)
    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", message.width, message.height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(b"".join(rows)))
        + chunk(b"IEND", b"")
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend", choices=("cpu", "wgpu", "cuda"), default="cpu")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--service-command", default="gz service")
    parser.add_argument("--sensor-system-plugin", type=Path)
    parser.add_argument(
        "--render-backend",
        choices=("metal", "headless"),
        default="metal" if sys.platform == "darwin" else "headless",
    )
    parser.add_argument(
        "--preview-layout",
        choices=("metric", "beam-raster"),
        default="metric",
        help="Metric fan is separate from the raw range-by-beam matrix; legacy control is explicit.",
    )
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    env = dict(
        os.environ,
        GZ_PARTITION="display_" + uuid.uuid4().hex,
        POSIM_SONAR_COMPUTE_BACKEND=args.backend,
        ROS_LOG_DIR=str(args.output / "ros_logs"),
    )
    here = Path(__file__).resolve().parent
    env_world = os.environ.get("SONAR_DISPLAY_TEST_WORLD")
    world = Path(env_world) if env_world else here / "empty_return_world.sdf"
    if args.sensor_system_plugin:
        if not args.sensor_system_plugin.is_file():
            parser.error("--sensor-system-plugin must name an existing library")
        tree = ET.parse(world)
        tree.getroot().find("./world/plugin[@name='gz::sim::systems::Sensors']").set(
            "filename", str(args.sensor_system_plugin.resolve())
        )
        world = args.output / "world.sdf"
        tree.write(world, encoding="utf-8", xml_declaration=True)
    rendering = (
        ["--render-engine-server-api-backend", "metal"]
        if args.render_backend == "metal"
        else ["--headless-rendering"]
    )
    model = here / "empty_return_sonar.sdf"
    rclpy.init()
    node = rclpy.create_node("sonar_display_regression")
    result = {
        "backend": args.backend,
        "pass": False,
        "phases": [],
        "render_backend": args.render_backend,
        "preview_layout": args.preview_layout,
        "fence_clock": "published_message_header",
    }
    phase = {"active": False, "fence": 0, "pairs": [], "raw": {}, "image": {}, "latest_stamp": -1}
    proc = None

    def callback(message, kind):
        result[kind + "_received"] = result.get(kind + "_received", 0) + 1
        t = stamp(message)
        phase["latest_stamp"] = max(phase["latest_stamp"], t)
        if not phase["active"] or t <= phase["fence"] or len(phase["pairs"]) >= 25:
            return
        phase[kind][t] = message
        if len(phase[kind]) > 64:
            del phase[kind][min(phase[kind])]
        if any(row["stamp_ns"] == t for row in phase["pairs"]):
            return
        if t not in phase["raw"] or t not in phase["image"]:
            return
        raw, image = phase["raw"].pop(t), phase["image"].pop(t)
        n, b = len(raw.ranges), raw.image.beam_count
        assert n > 1 and b > 1 and len(raw.beam_directions) == b
        assert all(math.isfinite(x) for x in raw.ranges)
        assert all(x < y for x, y in zip(raw.ranges, raw.ranges[1:]))
        assert raw.image.dtype == raw.image.DTYPE_FLOAT32
        assert len(raw.image.data) == n * b * 4
        shape = (n + 1, 2 * n + 1) if args.preview_layout == "metric" else (n, b)
        assert image.encoding == "bgr8" and (image.height, image.width) == shape
        assert image.step >= image.width * 3 and len(image.data) == image.height * image.step
        values = array.array("f", bytes(raw.image.data))
        if bool(raw.image.is_bigendian) != (sys.byteorder == "big"):
            values.byteswap()
        assert not any(math.isnan(v) or v == math.inf for v in values)
        pixels = b"".join(
            bytes(image.data[y * image.step : y * image.step + image.width * 3])
            for y in range(image.height)
        )
        empty = all(v == -math.inf for v in values)
        if not phase["pairs"]:
            save_png(image, phase["dest"] / "image.png")
            for suffix, msg in [("raw", raw), ("image", image)]:
                with gzip.open(phase["dest"] / (suffix + ".cdr.gz"), "wb") as out:
                    out.write(serialize_message(msg))
        if phase["expect_empty"]:
            assert empty, "Expected no raw returns after moving the panel outside the clip range"
            assert not any(pixels), "No-return raw frame rendered nonblack (false echo)"
        else:
            assert any(math.isfinite(v) for v in values), "Valid echo did not recover"
            assert any(pixels), "Valid echo image is black"
        phase["pairs"].append(
            {
                "stamp_ns": t,
                "ranges": n,
                "beams": b,
                "empty": empty,
                "raw_sha256": hashlib.sha256(bytes(raw.image.data)).hexdigest(),
                "image_sha256": hashlib.sha256(pixels).hexdigest(),
            }
        )

    node.create_subscription(
        ProjectedSonarImage,
        "/sensor/multibeam_sonar/sonar_image_raw",
        lambda m: callback(m, "raw"),
        QoSProfile(
            depth=20,
            durability=DurabilityPolicy.VOLATILE,
            reliability=ReliabilityPolicy.BEST_EFFORT,
        ),
    )
    node.create_subscription(
        Image,
        "/sensor/multibeam_sonar/sonar_image",
        lambda m: callback(m, "image"),
        QoSProfile(
            depth=20,
            durability=DurabilityPolicy.VOLATILE,
            reliability=ReliabilityPolicy.BEST_EFFORT,
        ),
    )

    def spin(seconds):
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            assert proc.poll() is None, "Gazebo exited unexpectedly"
            rclpy.spin_once(node, timeout_sec=0.05)

    def service(path, request_type, request):
        reply = subprocess.run(
            shlex.split(args.service_command)
            + [
                "-s",
                path,
                "--reqtype",
                request_type,
                "--reptype",
                "gz.msgs.Boolean",
                "--timeout",
                "10000",
                "--req",
                request,
            ],
            env=env,
            text=True,
            capture_output=True,
            timeout=15,
        )
        assert reply.returncode == 0 and "data: true" in reply.stdout, reply.stdout + reply.stderr

    try:
        with (args.output / "gazebo.log").open("w") as log:
            proc = subprocess.Popen(
                ["gz", "sim", "-s", "-r", *rendering, "-v", "4", str(world)],
                cwd=args.output,
                env=env,
                stdout=log,
                stderr=log,
                start_new_session=True,
            )
            spin(3)
            service(
                "/world/default/create",
                "gz.msgs.EntityFactory",
                f'sdf_filename: "{model}", name: "blueview_p900", allow_renaming: false',
            )
            spin(5)
            for distance in (4.0, 10.001, 10.02, 12.0, 4.001):
                phase["active"] = False
                service(
                    "/world/default/set_pose/blocking",
                    "gz.msgs.Pose",
                    f'name: "target_gray", position: {{x:{distance + 0.01},y:0,z:2}}',
                )
                spin(2)
                dest = args.output / str(distance)
                dest.mkdir()
                # Fence in the published headers' own clock domain. Captured
                # simulation stamps must never be compared with Unix wall time.
                assert phase["latest_stamp"] >= 0, "No timestamp observed before phase"
                phase.update(
                    active=True,
                    fence=phase["latest_stamp"],
                    pairs=[],
                    raw={},
                    image={},
                    dest=dest,
                    expect_empty=distance > 10,
                )
                deadline = time.monotonic() + 30
                while len(phase["pairs"]) < 25 and time.monotonic() < deadline:
                    spin(0.05)
                phase["active"] = False
                assert (
                    len(phase["pairs"]) == 25
                ), "Missing 25 matched raw/image frames in 30 seconds"
                row = {"distance_m": distance, "pairs": phase["pairs"]}
                (dest / "measurements.json").write_text(json.dumps(row, indent=2) + "\n")
                result["phases"].append(row)
                print(f"{args.backend}: {distance}m, 25 matched pairs passed", flush=True)
            os.killpg(proc.pid, signal.SIGINT)
            proc.wait(timeout=20)
            assert proc.returncode == 0, f"Abnormal Gazebo exit: {proc.returncode}"
            result["pass"] = True
    except Exception as error:
        result["error"] = repr(error)
        result["failed_phase"] = {
            "fence": phase["fence"],
            "pairs": phase["pairs"],
            "raw_pending": sorted(phase["raw"]),
            "image_pending": sorted(phase["image"]),
        }
        raise
    finally:
        if proc is not None and proc.poll() is None:
            os.killpg(proc.pid, signal.SIGINT)
            try:
                proc.wait(timeout=20)
            except subprocess.TimeoutExpired:
                result["forced_cleanup"] = True
                os.killpg(proc.pid, signal.SIGKILL)
                proc.wait()
        if proc is not None:
            result["exit"] = proc.returncode
        node.destroy_node()
        rclpy.try_shutdown()
        (args.output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
        print(json.dumps({k: v for k, v in result.items() if k != "phases"}))


if __name__ == "__main__":
    main()
