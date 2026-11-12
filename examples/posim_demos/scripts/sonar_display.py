#!/usr/bin/env python3
"""Convert the sonar's BGR preview for Gazebo's RGB-only image display."""

import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image


class SonarDisplay(Node):
    def __init__(self):
        super().__init__("sonar_display")
        self.publisher = self.create_publisher(Image, "sonar_rgb", 1)
        self.subscription = self.create_subscription(
            Image, "sonar_image", self.convert, qos_profile_sensor_data
        )

    def convert(self, message):
        if message.encoding != "bgr8":
            self.get_logger().error(f"Expected bgr8 sonar preview, received {message.encoding}")
            return
        pixels = bytearray(message.data)
        # Swap red and blue within each row, retaining any row padding.
        for row in range(message.height):
            start = row * message.step
            end = start + message.width * 3
            red = pixels[start + 2 : end : 3]
            pixels[start + 2 : end : 3] = pixels[start:end:3]
            pixels[start:end:3] = red
        message.encoding = "rgb8"
        message.data = pixels
        self.publisher.publish(message)


def main():
    rclpy.init()
    node = SonarDisplay()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, rclpy.executors.ExternalShutdownException):
        pass
    finally:
        node.destroy_node()
        rclpy.try_shutdown()


if __name__ == "__main__":
    main()
