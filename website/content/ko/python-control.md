# Python으로 제어하기

터미널 A에서 [REXROV 예제](rovs.md#start-rexrov)를 실행한 채로 두세요. 아래 프로그램은 [준비한 터미널 B](terminals.md)에서 실행합니다. 먼저 위치를 읽고, 이어서 추진기에 짧은 명령을 보냅니다.

## Python에서 구독

REXROV의 모의 위치를 출력하는 최소 예제입니다. `observe.py`로 저장하고 ROS 환경을 불러온 터미널에서 `python3 observe.py`로 실행합니다.

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

약 1초마다 위치가 출력되어야 합니다. Ctrl+C는 관찰 프로그램만 종료하며 시뮬레이터는 계속 실행됩니다.

## 제한된 시간 동안 추진기 명령 보내기

아래 내용을 `pulse_thruster.py`로 저장하고 같은 환경에서 `python3 pulse_thruster.py`를 실행하세요. 명령 구독자가 연결될 때까지 기다린 뒤 **1번 추진기에 5 N**을 호스트 시간 기준 약 2초 동안 보내고 0으로 되돌립니다. 추진기의 장착 축에 따라 힘과 토크의 방향이 결정됩니다.

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

시그널 옵션을 명시하여 Ctrl+C를 눌러도 ROS 발행자가 살아 있는 동안 종료 명령을 보내도록 합니다. 프로세스를 강제로 종료했다면 시뮬레이터를 정지하거나 [추진기와 조이스틱 제어](robot-control.md)에 따라 중립 명령을 보내세요.

## 추진기와 차량 반응 확인하기

명령이 실행되는 동안 환경을 불러온 다른 터미널에서 1번 추진기를 확인합니다.

```bash
ros2 topic echo --no-daemon /model/rexrov/joint/thruster1_joint/ang_vel std_msgs/msg/Float64
```

각속도 피드백이 명령에 반응한 뒤 0에 가까워져야 합니다. Odometry도 함께 보면서 추진기의 반응과 차량 전체의 움직임을 구분하세요. Ctrl+C로 확인 도구를 종료합니다.

## 제어기로 확장하기

측정 상태와 제어 법칙을 사용해 REXROV의 추진기 8개에 힘을 분배하세요. 토픽 이름, 장착 축과 단위를 일치시킵니다. 실험 로직에는 시뮬레이션 시간을 사용하고, 종료할 때 제어한 모든 추진기에 중립 값을 보내세요.

BlueROV2는 [추진기와 조이스틱 제어](robot-control.md)의 MAVROS·ArduSub 경로를 사용합니다. 관측 데이터를 저장하고 비교하려면 [센서 데이터 보기와 기록](visualization.md)을 참고하세요.
