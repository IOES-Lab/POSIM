#!/usr/bin/env python3
import asyncio
import json
import signal

import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from rclpy.signals import SignalHandlerOptions
from sensor_msgs.msg import Joy
import websockets
from websockets.exceptions import ConnectionClosed


class JoyWebSocketServer(Node):
    """Bridge browser websocket joystick payloads to sensor_msgs/Joy."""

    def __init__(self):
        super().__init__("joy_ws_server")

        self.declare_parameter("ws_host", "0.0.0.0")
        self.declare_parameter("ws_port", 8765)
        self.declare_parameter("output_topic", "/joy")

        ws_host_value = self.get_parameter("ws_host").value
        ws_port_value = self.get_parameter("ws_port").value
        output_topic_value = self.get_parameter("output_topic").value

        self.ws_host = str(ws_host_value or "0.0.0.0")
        self.ws_port = int(ws_port_value)
        self.output_topic = str(output_topic_value or "/joy")

        self.pub = self.create_publisher(Joy, self.output_topic, 10)

        self.get_logger().info(
            f"WebSocket listening on ws://{self.ws_host}:{self.ws_port} -> " f"{self.output_topic}"
        )

    def publish_payload(self, payload):
        msg = Joy()
        msg.header.stamp = self.get_clock().now().to_msg()

        payload_id = str(payload.get("id", "")).strip() or "unknown"
        msg.header.frame_id = f"browser_gamepad:{payload_id}"

        msg.axes = [float(value) for value in payload.get("axes", [])]
        msg.buttons = [int(value) for value in payload.get("buttons", [])]

        self.pub.publish(msg)


async def relay_messages(websocket, node, stopping):
    """Relay live input only while both the server and ROS context are active."""
    node.get_logger().info("Web joystick client connected")
    try:
        async for message in websocket:
            # Give signal/close callbacks a turn even when recv() drains a backlog.
            await asyncio.sleep(0)
            if stopping.is_set() or not rclpy.ok():
                break
            try:
                payload = json.loads(message)
            except (ValueError, TypeError) as exc:
                node.get_logger().warning(f"Invalid websocket JSON payload: {exc}")
                continue
            try:
                node.publish_payload(payload)
            except Exception as exc:
                if stopping.is_set() or not rclpy.ok():
                    break
                node.get_logger().error(f"Failed to publish Joy payload: {exc}")
    except ConnectionClosed:
        # A dropped browser connection is expected; the listener stays available.
        pass


async def main_async():
    # Close websocket handlers before invalidating their ROS publisher context.
    rclpy.init(signal_handler_options=SignalHandlerOptions.NO)
    node = None
    stopping = asyncio.Event()
    loop = asyncio.get_running_loop()
    previous_handlers = {}
    try:
        node = JoyWebSocketServer()
        for signum in (signal.SIGINT, signal.SIGTERM):
            previous_handlers[signum] = signal.getsignal(signum)
            loop.add_signal_handler(signum, stopping.set)

        async def handler(websocket):
            await relay_messages(websocket, node, stopping)

        async with websockets.serve(handler, node.ws_host, node.ws_port, close_timeout=2):
            try:
                while not stopping.is_set() and rclpy.ok():
                    rclpy.spin_once(node, timeout_sec=0.01)
                    await asyncio.sleep(0.01)
            except ExternalShutdownException:
                if rclpy.ok():
                    raise
            finally:
                stopping.set()
    finally:
        for signum, previous in previous_handlers.items():
            loop.remove_signal_handler(signum)
            signal.signal(signum, previous)
        if node is not None:
            node.destroy_node()
        rclpy.try_shutdown()


def main():
    try:
        asyncio.run(main_async())
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
