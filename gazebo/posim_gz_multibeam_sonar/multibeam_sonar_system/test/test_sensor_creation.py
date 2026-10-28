#!/usr/bin/env python3
"""Opt-in Gazebo integration regression; needs sourced ROS and plugin paths.

Run with Python that can import rclpy and marine_acoustic_msgs:
  python3 test_sensor_creation.py --backend cpu --output /tmp/creation-test
The output directory must not already exist. A rendering device is required.
"""

import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import time
import uuid
import xml.etree.ElementTree as ET

import rclpy
from rclpy.qos import DurabilityPolicy, QoSProfile
from marine_acoustic_msgs.msg import ProjectedSonarImage


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--backend', choices=('cpu', 'wgpu', 'cuda'), default='cpu')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    env = dict(os.environ, GZ_PARTITION='creation_' + uuid.uuid4().hex,
               POSIM_SONAR_COMPUTE_BACKEND=args.backend,
               ROS_LOG_DIR=str(args.output.resolve() / 'ros_logs'))
    world = Path(__file__).with_name('sensor_creation.sdf').resolve()
    root = ET.parse(world).getroot()
    model = root.find("./world/model[@name='blueview_p900']")
    spawn = args.output.resolve() / 'model.sdf'
    # Gazebo uses the literal gz: prefix when reading custom-sensor elements.
    ET.register_namespace('gz', 'http://gazebosim.org')
    spawn.write_text('<sdf version="1.10">' + ET.tostring(model, encoding='unicode') + '</sdf>')
    rclpy.init()
    node = rclpy.create_node('sonar_creation_regression')
    frames = []
    node.create_subscription(ProjectedSonarImage,
                             '/sensor/multibeam_sonar/sonar_image_raw', frames.append,
                             QoSProfile(depth=10, durability=DurabilityPolicy.TRANSIENT_LOCAL))
    proc = None
    result = {'backend': args.backend, 'pass': False}
    log_path = args.output / 'gazebo.log'

    def collect():
        deadline = time.monotonic() + 120
        while len(frames) < 5 and time.monotonic() < deadline:
            assert proc.poll() is None, 'Gazebo exited before publishing sonar data'
            rclpy.spin_once(node, timeout_sec=0.1)
        assert len(frames) >= 5, 'No sonar output within 120 seconds'
        for message in frames:
            assert message.image.beam_count > 0 and len(message.ranges) > 0
            assert len(message.image.data) == message.image.beam_count * len(message.ranges) * 4

    def service(endpoint, request_type, request):
        reply = subprocess.run(
            ['gz', 'service', '-s', endpoint, '--reqtype', request_type,
             '--reptype', 'gz.msgs.Boolean', '--timeout', '10000', '--req', request],
            env=env, capture_output=True, text=True, timeout=15, check=True)
        assert 'data: true' in reply.stdout, reply.stdout + reply.stderr

    try:
        with log_path.open('w') as log:
            proc = subprocess.Popen(['gz', 'sim', '-s', '-r', '--headless-rendering',
                                     '-v', '4', str(world)], cwd=args.output, env=env,
                                    stdout=log, stderr=log, start_new_session=True)
            collect()
            text = log_path.read_text(errors='replace')
            assert text.count('Initializing [blueview_p900') == 1, 'Duplicate initialization'
            assert 'already exists' not in text, 'Duplicate rendering resource'
            service('/world/default/remove', 'gz.msgs.Entity',
                    'name: "blueview_p900", type: MODEL')
            # Drain old transient-local samples before re-creating the sensor.
            until = time.monotonic() + 2
            while time.monotonic() < until:
                rclpy.spin_once(node, timeout_sec=0.1)
            frames.clear()
            service('/world/default/create', 'gz.msgs.EntityFactory',
                    f'sdf_filename: "{spawn}", name: "blueview_p900", allow_renaming: false')
            collect()
            text = log_path.read_text(errors='replace')
            assert text.count('Initializing [blueview_p900') == 2, 'Re-creation failed'
            assert 'already exists' not in text, 'Duplicate resource after re-creation'
            os.killpg(proc.pid, signal.SIGINT)
            proc.wait(timeout=20)
            assert proc.returncode == 0, f'Abnormal exit: {proc.returncode}'
            result['pass'] = True
    finally:
        if proc is not None and proc.poll() is None:
            os.killpg(proc.pid, signal.SIGKILL)
            proc.wait()
        if proc is not None:
            result['exit'] = proc.returncode
        node.destroy_node()
        rclpy.try_shutdown()
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result))


if __name__ == '__main__':
    main()
