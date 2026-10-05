# ROS 2 and control

Read observations and send commands using the interfaces configured for the selected vehicle. Gazebo Transport and ROS 2 are separate transports; a bridge or ROS-enabled plugin connects them.

## Inspect the graph

Use a second terminal in the same environment as the simulator:

```bash
ros2 topic list -t
ros2 service list -t
gz topic -l
ros2 topic info /model/rexrov/odometry --verbose
```

Check types, frame IDs, rates and QoS before connecting your node. Nodes using simulation timestamps should set `use_sim_time:=true` and receive the simulation clock.

## REXROV thruster commands

The included REXROV configuration bridges eight `cmd_thrust` topics as `std_msgs/msg/Float64`. The values are thrust commands in newtons. A neutral command for the first thruster is:

```bash
ros2 topic pub --once /model/rexrov/joint/thruster1_joint/cmd_thrust \
  std_msgs/msg/Float64 '{data: 0.0}'
```

A controller must allocate forces across the thrusters using their positions and axes. One thruster is not a vehicle-level velocity controller. End an experiment by sending neutral values to every commanded thruster.

## BlueROV2 and autopilot

BlueROV2 connects to ArduSub through MAVROS. Inspect state and available command interfaces:

```bash
ros2 topic echo /mavros/state --once
ros2 service list -t
ros2 topic info /mavros/manual_control/send --verbose
```

The included manual-control adapter maps `/joy` and `/keyboard/joy` to MAVROS manual control. It publishes at 20 Hz and applies a 0.3-second input timeout. Use one active command source and respect arming/navigation checks. Keyboard and local WebSocket setup are described in [ROVs](rovs.md).

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
        self.get_logger().info(f'x={p.x:.2f}, y={p.y:.2f}, z={p.z:.2f}')

rclpy.init()
node = Observer()
try:
    rclpy.spin(node)
except KeyboardInterrupt:
    pass
finally:
    node.destroy_node()
    rclpy.shutdown()
```

## Visualization and recording

Use RViz displays matching the message type: Image for cameras, PointCloud2 for point clouds, and TF/pose displays for frames and state. POSIM-specific messages are defined in `posim_interfaces`; install/source these definitions on any separate ROS client.

```bash
ros2 bag record /model/rexrov/odometry /model/rexrov/imu
```

A remote ROS client needs compatible message definitions, reachable DDS discovery/data paths and matching domain/discovery settings. A documentation website or HTTP tunnel does not expose ROS DDS automatically. Session URLs and remote-access gateways are responsibilities of the application hosting the simulator.
