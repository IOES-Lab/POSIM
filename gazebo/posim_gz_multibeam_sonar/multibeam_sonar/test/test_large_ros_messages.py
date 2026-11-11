#!/usr/bin/env python3
"""Opt-in two-process regression for large best-effort sonar messages.

Run with a sourced ROS environment and an unused ROS domain:
  python3 test_large_ros_messages.py --profile ../config/fastdds_macos_local.xml --domain-id 185
"""

import argparse
import array
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import uuid

import rclpy
from rclpy.qos import QoSProfile, ReliabilityPolicy
from marine_acoustic_msgs.msg import ProjectedSonarImage


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--domain-id", type=int, required=True)
    parser.add_argument("--topic", default="/sonar_transport_" + uuid.uuid4().hex)
    parser.add_argument("--publish", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if not args.profile.is_file():
        parser.error("--profile must name an existing Fast DDS XML profile")
    if not 0 <= args.domain_id <= 232:
        parser.error("--domain-id must be between 0 and 232")
    os.environ.update(
        FASTDDS_DEFAULT_PROFILES_FILE=str(args.profile.resolve()),
        RMW_IMPLEMENTATION="rmw_fastrtps_cpp",
        ROS_DOMAIN_ID=str(args.domain_id),
        ROS_AUTOMATIC_DISCOVERY_RANGE="LOCALHOST",
    )
    payload = array.array("B", (i % 251 for i in range(512 * 400 * 4)))
    received = set()
    child = None
    rclpy.init()
    node = rclpy.create_node(
        "sonar_transport_publisher" if args.publish else "sonar_transport_reader"
    )
    qos = QoSProfile(depth=10, reliability=ReliabilityPolicy.BEST_EFFORT)

    def receive(message):
        assert message.image.beam_count == 512 and len(message.ranges) == 400
        assert message.image.data == payload, "Incomplete or corrupted sonar payload"
        received.add(message.header.stamp.sec)

    try:
        if args.publish:
            publisher = node.create_publisher(ProjectedSonarImage, args.topic, qos)
            message = ProjectedSonarImage()
            message.image.beam_count = 512
            message.ranges = array.array("f", range(400))
            message.image.data = payload
        else:
            node.create_subscription(ProjectedSonarImage, args.topic, receive, qos)
            child = subprocess.Popen(
                [
                    sys.executable,
                    str(Path(__file__).resolve()),
                    "--profile",
                    str(args.profile.resolve()),
                    "--domain-id",
                    str(args.domain_id),
                    "--topic",
                    args.topic,
                    "--publish",
                ]
            )
        deadline = time.monotonic() + (10 if args.publish else 13)
        while time.monotonic() < deadline:
            if args.publish:
                message.header.stamp.sec += 1
                publisher.publish(message)
            rclpy.spin_once(node, timeout_sec=0.2)
        if child is not None:
            assert child.wait(timeout=5) == 0, "Publisher failed"
            assert len(received) >= 5, "Fewer than five complete large sonar messages received"
            print(
                json.dumps(
                    {"passed": True, "messages": len(received), "payload_bytes": len(payload)}
                )
            )
    finally:
        if child is not None and child.poll() is None:
            child.terminate()
            try:
                child.wait(timeout=5)
            except subprocess.TimeoutExpired:
                child.kill()
                child.wait()
        node.destroy_node()
        rclpy.try_shutdown()


if __name__ == "__main__":
    main()
