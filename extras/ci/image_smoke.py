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
import shutil
import subprocess
import time


PACKAGES = (
    "dave_interfaces dave_demos dave_gz_model_plugins dave_gz_sensor_plugins "
    "dave_gz_world_plugins dave_ros_gz_plugins dave_object_models dave_robot_models "
    "dave_sensor_models dave_worlds multibeam_sonar multibeam_sonar_system "
    "dave_multibeam_sonar_demo"
).split()


def fatal_log_lines(log_text):
    """Find child failures even when the launch/gz wrapper exits on SIGINT."""
    faults = re.findall(
        r".*(?:Segmentation fault|exit code (?:-11|-6|134|139)\b|"
        r"\bAborted\b|terminate called|rclcpp::exceptions::RCLError|"
        r"Traceback \(most recent call last\)|"
        r"Failed to load system plugin|error while loading shared libraries).*",
        log_text,
    )
    for line in log_text.splitlines():
        match = re.search(r"process has died .*exit code (-?\d+)\b", line)
        if match and int(match[1]) not in (0, 130, -signal.SIGINT) and line not in faults:
            faults.append(line)
    return faults


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


def wait_for_entity(out, world, entity, proc, deadline):
    """Observe the spawned model within the original 90-second startup budget.

    The world control service can appear before the create service/model. A
    fixed sleep followed by one pose sample tests that race, not model loading.
    Every observation is retained; this does not restart a failed scene.
    """
    attempts = []
    present = False
    while proc.poll() is None:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        code, poses = capture(
            out,
            "poses",
            ["gz", "topic", "-e", "-t", f"/world/{world}/pose/info", "-n", "1"],
            min(8, remaining),
        )
        present = (
            code == 0 and re.search(r'name:\s*"' + re.escape(entity) + r'"', poses) is not None
        )
        attempts.append({"returncode": code, "entity_present": present})
        for suffix in ("txt", "stderr", "json"):
            source = out / f"poses.{suffix}"
            if source.exists():
                shutil.copyfile(source, out / f"poses-attempt-{len(attempts):03d}.{suffix}")
        if present:
            break
        time.sleep(min(1, max(0, deadline - time.monotonic())))
    (out / "entity_readiness.json").write_text(
        json.dumps({"entity": entity, "present": present, "attempts": attempts}, indent=2)
    )
    return present


def linked_library(ldd_output, name):
    """Require an absolute resolved path, not an unresolved or partial match."""
    entries = re.findall(r"^\s*" + re.escape(name) + r"\s+=>\s+(/\S+)\s", ldd_output, re.M)
    if len(entries) != 1:
        raise RuntimeError(f"Missing or ambiguous linkage for {name}")
    return Path(entries[0]).resolve()


def transport_inventory(out, get_package_prefix):
    ws = Path(os.environ["POSIM_TRANSPORT_UNDERLAY"])
    if Path(get_package_prefix("gz_transport_vendor")) != ws / "install":
        raise RuntimeError("The pinned Gazebo Transport overlay is not active")
    for filename, expected in (
        ("upstream-revision.txt", "82b10bdff114f77655c7f0cc856835179a674d6d"),
        ("vendor-revision.txt", "bc048aec33d25d73e651b86e2858339885dfab87"),
    ):
        actual = (ws / filename).read_text().strip()
        if actual != expected:
            raise RuntimeError(f"Unexpected transport provenance: {filename}={actual}")
        (out / f"transport_{filename}").write_text(actual + "\n")
    patches = Path(__file__).resolve().parent.parent / "patches"
    for patch_name, recorded_name in (
        ("gz-transport-poll-serialization.patch", "poll-patch.sha256"),
        ("gz-transport-vendor-poll.patch", "vendor-patch.sha256"),
    ):
        expected = hashlib.sha256((patches / patch_name).read_bytes()).hexdigest()
        if (ws / recorded_name).read_text().strip() != expected:
            raise RuntimeError(f"Transport patch differs from validation source: {patch_name}")
        (out / f"transport_{recorded_name}").write_text(expected + "\n")
    prefix = ws / "install/opt/gz_transport_vendor"
    library = (prefix / "lib/libgz-transport.so.15").resolve()
    expected_hash = (ws / "library.sha256").read_text().strip()
    if hashlib.sha256(library.read_bytes()).hexdigest() != expected_hash:
        raise RuntimeError("Installed Transport library differs from the build record")
    (out / "transport_library.sha256").write_text(expected_hash + "\n")
    config_path = "include/gz/transport15/gz/transport/config.hh"
    vendor_prefix = Path("/opt/ros/lyrical/opt/gz_transport_vendor")
    if (prefix / config_path).read_bytes() != (vendor_prefix / config_path).read_bytes():
        raise RuntimeError("Transport public build configuration differs from the ROS vendor")
    # Check both the regression binary and the installed Gazebo simulator DSO.
    probe = ws / "probe/bin/posim_transport_churn"
    sim_prefix = Path(get_package_prefix("gz_sim_vendor")) / "opt/gz_sim_vendor"
    for name, binary in (
        ("transport_probe_linkage", probe),
        ("sim_transport_linkage", sim_prefix / "lib/libgz-sim.so.10"),
    ):
        code, dependencies = capture(out, name, ["ldd", str(binary)])
        if code or linked_library(dependencies, "libgz-transport.so.15") != library:
            raise RuntimeError(f"{name} is not using the patched Transport overlay")
    # Do not force LD_LIBRARY_PATH here: the installed setup chain must work.
    code, _ = capture(
        out,
        "transport_churn",
        [
            "python3",
            str(Path(__file__).parent / "transport_shutdown/run_pairs.py"),
            "--executable",
            str(probe),
            "--variant",
            "patched:",
            "--trials",
            "5",
            "--seconds",
            "20",
            "--output",
            str(out / "transport_churn"),
        ],
        240,
    )
    if code:
        raise RuntimeError("Installed Transport churn regression failed")


def inventory(out):
    from ament_index_python.packages import get_package_prefix, get_package_share_directory

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
        code, launch_args = capture(
            out, launch.stem, ["ros2", "launch", "dave_demos", launch.name, "--show-args"], 30
        )
        if code:
            raise RuntimeError(f"Cannot resolve launch arguments: {launch.name}")
        if "wait_for_assets" not in launch_args:
            raise RuntimeError(f"Asset readiness argument missing: {launch.name}")
    worlds = sorted((shares["dave_worlds"] / "worlds").glob("*.world"))
    robots = sorted((shares["dave_robot_models"] / "description").glob("*/model.sdf"))
    sensors = sorted((shares["dave_sensor_models"] / "description").glob("*/model.sdf"))
    counts = {"worlds": len(worlds), "robots": len(robots), "sensors": len(sensors)}
    if counts != {"worlds": 18, "robots": 5, "sensors": 11}:
        raise RuntimeError(f"Unexpected installed resource inventory: {counts}")
    capture(out, "deb_versions", ["dpkg-query", "-W", "ros-lyrical-*"], 30)
    capture(out, "gazebo_version", ["gz", "sim", "--versions"])
    transport_inventory(out, get_package_prefix)
    ws = Path(os.environ["POSIM_WORKSPACE"])
    bridge_ws = Path(os.environ["POSIM_BRIDGE_UNDERLAY"])
    if Path(get_package_prefix("ros_gz_bridge")) != bridge_ws / "install":
        raise RuntimeError("The pinned bridge overlay is not active")
    revision = (bridge_ws / "upstream-revision.txt").read_text().strip()
    if revision != "54a2e78a41c623173608cdd8eef2e049ee3ee3b0":
        raise RuntimeError(f"Unexpected bridge source revision: {revision}")
    (out / "bridge_source_revision.txt").write_text(revision + "\n")
    patch = (
        Path(__file__).resolve().parent.parent / "patches/ros-gz-bridge-callback-lifetime.patch"
    )
    expected_patch = hashlib.sha256(patch.read_bytes()).hexdigest()
    if (bridge_ws / "callback-patch.sha256").read_text().strip() != expected_patch:
        raise RuntimeError("Bridge callback patch does not match the validation source")
    (out / "bridge_callback_patch.sha256").write_text(expected_patch + "\n")
    code, _ = capture(
        out, "bridge_ownership", [str(bridge_ws / "probe/bin/posim_bridge_ownership_check")], 60
    )
    if code:
        raise RuntimeError("Bridge ownership regression check failed")
    mavros_ws = Path(os.environ["POSIM_MAVROS_UNDERLAY"])
    for package in ("mavros", "libmavconn"):
        if Path(get_package_prefix(package)) != mavros_ws / "install":
            raise RuntimeError(f"The pinned {package} overlay is not active")
    for filename, expected in (
        ("upstream-revision.txt", "22ae5b7cc7cdb4cb9c2070a8213c72dae445a23e"),
        ("upstream-fix.txt", "3a1f39f1a033d39d9e7c34d9ba7cb28cd3dbcd5f"),
    ):
        actual = (mavros_ws / filename).read_text().strip()
        if actual != expected:
            raise RuntimeError(f"Unexpected MAVROS provenance: {filename}={actual}")
        (out / f"mavros_{filename}").write_text(actual + "\n")
    mavconn_patch = patch.with_name("mavconn-self-close-lifetime.patch")
    expected_mavconn_patch = hashlib.sha256(mavconn_patch.read_bytes()).hexdigest()
    if (mavros_ws / "self-close-patch.sha256").read_text().strip() != expected_mavconn_patch:
        raise RuntimeError("MAVConn patch does not match the validation source")
    (out / "mavconn_patch.sha256").write_text(expected_mavconn_patch + "\n")
    router_patch = patch.with_name("mavros-router-parent-lifetime.patch")
    expected_router_patch = hashlib.sha256(router_patch.read_bytes()).hexdigest()
    if (mavros_ws / "router-parent-patch.sha256").read_text().strip() != expected_router_patch:
        raise RuntimeError("MAVROS Router patch does not match the validation source")
    (out / "mavros_router_patch.sha256").write_text(expected_router_patch + "\n")
    code, _ = capture(
        out,
        "mavros_ownership",
        [str(mavros_ws / "probe/bin/posim_mavros_ownership_check")],
        60,
    )
    if code:
        raise RuntimeError("MAVROS Router ownership regression failed")
    code, dependencies = capture(
        out, "mavros_linkage", ["ldd", str(mavros_ws / "install/lib/mavros/mavros_node")]
    )
    if code or str(mavros_ws / "install/lib/libmavconn.so") not in dependencies:
        raise RuntimeError("MAVROS is not linked to the patched MAVConn overlay")
    code, _ = capture(
        out,
        "mavconn_self_close",
        [str(mavros_ws / "probe/bin/posim_mavconn_self_close_check")],
        60,
    )
    if code:
        raise RuntimeError("MAVConn self-close sanitizer regression failed")
    if not shutil.which("ardusub"):
        raise RuntimeError("ArduSub is not available in the noninteractive image PATH")
    plugin_paths = os.environ.get("GZ_SIM_SYSTEM_PLUGIN_PATH", "").split(":")
    if not any((Path(p) / "libArduPilotPlugin.so").is_file() for p in plugin_paths if p):
        raise RuntimeError("ArduPilotPlugin is not on GZ_SIM_SYSTEM_PLUGIN_PATH")
    code, _ = capture(
        out,
        "camera_unit_tests",
        [str(ws / "build/dave_gz_sensor_plugins/test_underwater_camera")],
        60,
    )
    if code:
        raise RuntimeError("Camera C++ regression tests failed")
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


def camera_payload(out):
    import rclpy
    from rclpy.qos import qos_profile_sensor_data
    from sensor_msgs.msg import Image

    rclpy.init()
    node = rclpy.create_node("posim_camera_image_check")
    messages = []
    node.create_subscription(
        Image, "/underwater_camera/simulated_image", messages.append, qos_profile_sensor_data
    )
    try:
        deadline = time.monotonic() + 45
        while not messages and time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=1)
        if not messages:
            return False
        msg = messages[0]
        payload = bytes(msg.data)
        (out / "ros_image.json").write_text(
            json.dumps(
                {
                    "topic": "/underwater_camera/simulated_image",
                    "width": msg.width,
                    "height": msg.height,
                    "step": msg.step,
                    "encoding": msg.encoding,
                    "data_bytes": len(payload),
                    "data_sha256": hashlib.sha256(payload).hexdigest(),
                },
                indent=2,
            )
        )
        return (
            msg.width > 0
            and msg.height > 0
            and msg.step > 0
            and len(payload) == msg.step * msg.height
        )
    finally:
        node.destroy_node()
        rclpy.shutdown()


def mavros_connected(out):
    import rclpy
    from mavros_msgs.msg import State
    from rclpy.qos import qos_profile_sensor_data

    rclpy.init()
    node = rclpy.create_node("posim_mavros_connection_check")
    states = []
    node.create_subscription(State, "/mavros/state", states.append, qos_profile_sensor_data)
    try:
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=1)
            if any(state.connected for state in states):
                break
        connected = any(state.connected for state in states)
        (out / "mavros_state.json").write_text(
            json.dumps({"topic": "/mavros/state", "connected": connected, "messages": len(states)})
        )
        return connected
    finally:
        node.destroy_node()
        rclpy.shutdown()


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
                checks["entity_present"] = entity == "__NONE__" or wait_for_entity(
                    out, world, entity, proc, deadline
                )
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
                if case[0] == "spherical_world":
                    code, origin = capture(
                        out,
                        "spherical_origin",
                        [
                            "ros2",
                            "service",
                            "call",
                            "/gz/get_origin_spherical_coordinates",
                            "dave_interfaces/srv/GetOriginSphericalCoord",
                            "{}",
                        ],
                        20,
                    )
                    coords = re.search(
                        r"latitude_deg=([0-9.eE+-]+), longitude_deg=([0-9.eE+-]+)", origin
                    )
                    checks["spherical_service"] = (
                        code == 0
                        and coords is not None
                        and abs(float(coords[1]) - 35.074823) < 1e-6
                        and abs(float(coords[2]) - 129.084798) < 1e-6
                    )
                # A topic listing is not sufficient: obtain a real message.
                if case[0] == "camera":
                    # This plugin publishes its transformed image to ROS, not Gazebo.
                    checks["ros_image_payload"] = camera_payload(out)
                elif topic_pattern.startswith("/world/") and topic_pattern.endswith("/"):
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
                if case[0] in ("bluerov2", "bluerov2_heavy"):
                    checks["mavros_connected"] = mavros_connected(out)
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
    faults = fatal_log_lines(log_text)
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
    parser.add_argument("--record", help="Unique evidence directory for a repeat trial")
    args = parser.parse_args()
    out = Path("/results") / (args.record or args.case)
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
