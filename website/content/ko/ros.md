# ROS 2와 제어

선택한 로봇에 설정된 인터페이스로 관측값을 받고 명령을 보냅니다. Gazebo Transport와 ROS 2는 별도의 통신 체계이며 브리지나 ROS 기능을 갖춘 플러그인이 연결합니다.

## 통신 구성 확인

시뮬레이터와 같은 환경의 두 번째 터미널에서 확인합니다.

```bash
ros2 topic list -t
ros2 service list -t
gz topic -l
ros2 topic info /model/rexrov/odometry --verbose
```

노드를 연결하기 전에 자료형, 좌표계, 주기와 QoS를 확인하세요. 시뮬레이션 타임스탬프를 사용하는 노드는 `use_sim_time:=true`로 설정하고 시뮬레이션 시계도 받아야 합니다.

## REXROV 추진기 명령

REXROV 설정은 8개의 `cmd_thrust` 토픽을 `std_msgs/msg/Float64`로 연결합니다. 값은 뉴턴 단위의 추력 명령입니다. 첫 추진기의 중립 명령은 다음과 같습니다.

```bash
ros2 topic pub --once /model/rexrov/joint/thruster1_joint/cmd_thrust \
  std_msgs/msg/Float64 '{data: 0.0}'
```

제어기는 추진기의 위치와 축을 사용하여 힘을 분배해야 합니다. 추진기 하나의 명령은 로봇 전체의 속도 제어가 아닙니다. 실험을 끝낼 때는 명령을 보낸 모든 추진기에 중립 값을 전송합니다.

## BlueROV2와 autopilot

BlueROV2는 MAVROS를 통해 ArduSub와 연결합니다. 상태와 사용할 명령 인터페이스를 확인합니다.

```bash
ros2 topic echo /mavros/state --once
ros2 service list -t
ros2 topic info /mavros/manual_control/send --verbose
```

수동 제어 어댑터는 `/joy`, `/keyboard/joy`를 MAVROS 명령으로 변환합니다.

- 발행 주기: **20 Hz**
- 입력 타임아웃: **0.3초**

명령 입력은 하나씩 사용하고 Arm·항법 상태를 확인하세요. 키보드·WebSocket 설정은 [ROV 예제](rovs.md)를 참고하세요.

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

## 시각화와 기록

RViz에서 자료형에 맞는 디스플레이를 선택합니다.

- 카메라: Image
- 점군: PointCloud2
- 좌표계·상태: TF와 위치 디스플레이

POSIM 전용 메시지는 `posim_interfaces`에 정의되어 있습니다. 별도 ROS 클라이언트에도 이를 설치하고 환경을 불러오세요.

```bash
ros2 bag record /model/rexrov/odometry /model/rexrov/imu
```

원격 ROS 클라이언트에는 다음 설정이 필요합니다.

- 호환 메시지 정의
- 접근 가능한 DDS 탐색·데이터 경로
- 일치하는 도메인·탐색 설정

원격 연결 게이트웨이는 시뮬레이터 응용 프로그램에서 구성합니다.
