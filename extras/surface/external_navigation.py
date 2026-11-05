"""Explicit simulator ground truth for the standalone WAM-V demo; not a sensor."""

from contextlib import ExitStack
import signal

import rclpy
from rclpy.context import Context
from rclpy.executors import ExternalShutdownException, SingleThreadedExecutor
from rclpy.signals import SignalHandlerOptions
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from nav_msgs.msg import Odometry
from geometry_msgs.msg import PoseWithCovarianceStamped, TwistWithCovarianceStamped
from geographic_msgs.msg import GeoPointStamped
from scipy.spatial.transform import Rotation
from mavros_msgs.msg import State
from mavros_msgs.srv import CommandLong


class Navigation(Node):
    def __init__(self, *, context=None):
        super().__init__("wamv_simulated_navigation", context=context)
        self.start = None
        self.count = 0
        self.pose = self.create_publisher(
            PoseWithCovarianceStamped, "/mavros/vision_pose/pose_cov", 10
        )
        self.speed = self.create_publisher(
            TwistWithCovarianceStamped, "/mavros/vision_speed/speed_twist_cov", 10
        )
        self.origin = self.create_publisher(
            GeoPointStamped, "/mavros/global_position/set_gp_origin", 10
        )
        self.create_subscription(
            Odometry,
            "/model/wamv/odometry",
            self.receive,
            QoSProfile(depth=1, reliability=ReliabilityPolicy.BEST_EFFORT),
        )
        self.interval = self.create_client(CommandLong, "/mavros/cmd/command")
        self.requested = False
        self.create_subscription(State, "/mavros/state", self.state, 10)
        self.get_logger().warning(
            "SITL navigation uses simulated Gazebo ground truth, not a real positioning sensor."
        )

    def state(self, m):
        if m.connected and not self.requested and self.interval.service_is_ready():
            request = CommandLong.Request()
            request.command = 511
            request.param1 = 32.0
            request.param2 = 50000.0
            future = self.interval.call_async(request)
            self.requested = True

            def complete(f):
                try:
                    self.requested = bool(f.result().success)
                except Exception:
                    self.requested = False

            future.add_done_callback(complete)

    def receive(self, m):
        p, q = m.pose.pose.position, m.pose.pose.orientation
        if self.start is None:
            self.start = (p.x, p.y)
        pose = PoseWithCovarianceStamped()
        pose.header = m.header
        pose.pose.pose.position.x = p.x - self.start[0]
        pose.pose.pose.position.y = p.y - self.start[1]
        pose.pose.pose.position.z = p.z
        pose.pose.pose.orientation = q
        for i in (0, 7, 14):
            pose.pose.covariance[i] = 0.01
        for i in (21, 28, 35):
            pose.pose.covariance[i] = 0.001
        self.pose.publish(pose)
        velocity = m.twist.twist.linear
        v = Rotation.from_quat([q.x, q.y, q.z, q.w]).apply([velocity.x, velocity.y, velocity.z])
        speed = TwistWithCovarianceStamped()
        speed.header = m.header
        speed.twist.twist.linear.x, speed.twist.twist.linear.y, speed.twist.twist.linear.z = map(
            float, v
        )
        for i in (0, 7, 14):
            speed.twist.covariance[i] = 0.01
        self.speed.publish(speed)
        if self.count < 600 and self.count % 20 == 0:
            origin = GeoPointStamped()
            origin.header = m.header
            origin.position.latitude = 35.07446
            origin.position.longitude = 129.08468
            origin.position.altitude = 0.0
            self.origin.publish(origin)
        self.count += 1


def main(args=None, *, restore_signal_handlers=True):
    # Record a stop request rather than raising inside ROS constructors. In
    # particular, an interrupted Executor constructor can leave its destructor
    # with uninitialized attributes. Keep the context alive until cleanup.
    stop_requested = False

    def request_stop(_signum, _frame):
        nonlocal stop_requested
        stop_requested = True

    with ExitStack() as cleanup:
        for signum in (signal.SIGINT, signal.SIGTERM):
            previous = signal.signal(signum, request_stop)
            if restore_signal_handlers:
                cleanup.callback(signal.signal, signum, previous)

        context = Context()
        cleanup.callback(context.try_shutdown)
        rclpy.init(args=args, context=context, signal_handler_options=SignalHandlerOptions.NO)
        if stop_requested:
            return

        node = Navigation(context=context)
        cleanup.callback(node.destroy_node)
        if stop_requested:
            return

        executor = SingleThreadedExecutor(context=context)
        cleanup.callback(executor.shutdown)
        executor.add_node(node)
        try:
            while not stop_requested and context.ok():
                # A bounded wait also observes stop requests when no data arrives.
                executor.spin_once(timeout_sec=0.1)
        except ExternalShutdownException:
            pass
        # ExitStack releases executor -> node -> context, then restores signal
        # handlers for embedded callers. Repeated signals during cleanup only
        # record another request.


if __name__ == "__main__":
    try:
        main(restore_signal_handlers=False)
    finally:
        # ROS cleanup has finished. Ignore late duplicate signals during Python
        # finalization, which otherwise resets callable handlers to SIG_DFL.
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
