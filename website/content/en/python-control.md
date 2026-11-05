# Control with Python

Start REXROV with [First simulation](quickstart.md) and keep it running in Terminal A. Run these programs in [prepared Terminal B](terminals.md). Read the position first, then send one short thruster pulse.

## Subscribe from Python

This minimal example prints REXROV's simulated position. Save it as `observe.py` and run `python3 observe.py` in the sourced ROS environment.

```python
import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node

class Observer(Node):
    def __init__(self):
        super().__init__('posim_observer')
        self.subscription = self.create_subscription(
            Odometry, '/model/rexrov/odometry', self.on_pose, 10)

    def on_pose(self, message):
        p = message.pose.pose.position
        self.get_logger().info(
            f'x={p.x:.2f}, y={p.y:.2f}, z={p.z:.2f}', throttle_duration_sec=1.0)

rclpy.init()
node = Observer()
try:
    rclpy.spin(node)
except KeyboardInterrupt:
    pass
finally:
    node.destroy_node()
    if rclpy.ok():
        rclpy.shutdown()
```

You should see a position about once per second. Ctrl+C stops the observer and leaves the simulator running.

## Send a bounded thruster command

Save this as `pulse_thruster.py`, then run `python3 pulse_thruster.py` in the same environment. It waits for the command subscriber, applies **5 N to thruster 1** for about two seconds of host time, then sends zero. The thruster's mounting axis determines the resulting force and torque.

```python
import time
import rclpy
from rclpy.node import Node
from rclpy.signals import SignalHandlerOptions
from std_msgs.msg import Float64

rclpy.init(signal_handler_options=SignalHandlerOptions.NO)
node = Node('posim_thruster_pulse')
publisher = node.create_publisher(
    Float64, '/model/rexrov/joint/thruster1_joint/cmd_thrust', 10)
deadline = time.monotonic() + 10.0
try:
    while publisher.get_subscription_count() == 0:
        if time.monotonic() >= deadline:
            raise RuntimeError('Start REXROV and load its ROS workspace first.')
        rclpy.spin_once(node, timeout_sec=0.1)
    print('Applying 5 N to thruster 1', flush=True)
    for _ in range(20):
        publisher.publish(Float64(data=5.0))
        rclpy.spin_once(node, timeout_sec=0.1)
except KeyboardInterrupt:
    pass
finally:
    for _ in range(5):
        publisher.publish(Float64(data=0.0))
        rclpy.spin_once(node, timeout_sec=0.1)
    print('Thruster command returned to zero', flush=True)
    node.destroy_node()
    rclpy.shutdown()
```

The explicit signal option lets Ctrl+C reach the cleanup block while the ROS publisher is still available. If the process is forcibly terminated, stop the simulator or send neutral commands using [Thrusters & joystick control](robot-control.md).

## Observe the actuator and vehicle

In another sourced terminal, inspect the first thruster while the pulse runs:

```bash
ros2 topic echo --no-daemon /model/rexrov/joint/thruster1_joint/ang_vel std_msgs/msg/Float64
```

The angular-velocity feedback should respond and return toward zero. Watch odometry alongside it to distinguish the actuator response from the whole vehicle's motion. Press Ctrl+C to stop the viewer.

## Build a controller

Use the measured state and your control law to allocate forces across all eight REXROV thrusters. Keep topic names, mounting axes and units consistent. Use simulation time for experiment logic and send neutral values to every actuator your controller commands when stopping.

BlueROV2 uses the [Thrusters & joystick control](robot-control.md) MAVROS/ArduSub path. For saved observations and comparisons, use [View & record sensor data](visualization.md).
