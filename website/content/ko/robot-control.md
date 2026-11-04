# 추진기와 조이스틱 제어

로봇에 맞는 명령 인터페이스를 선택하세요. 먼저 센서 데이터를 확인한 뒤 제어기를 하나씩 활성화합니다.

| 차량 | 명령 전달 경로 | 다음 단계 |
|---|---|---|
| REXROV | ROS 추력 명령 → Gazebo 추진기 | [Python으로 제어하기](python-control.md) |
| BlueROV2 | ROS·조이스틱 → MAVROS → ArduSub | [ROV와 BlueROV2](rovs.md) |
| WAM-V | 수상 로봇 통합과 차량 제어기 | [수상 로봇과 파도](surface.md) |

## REXROV 추진기 명령

REXROV 설정은 8개의 `cmd_thrust` 토픽을 `std_msgs/msg/Float64`로 연결합니다. 값은 뉴턴 단위의 추력 명령입니다. 첫 추진기의 중립 명령은 다음과 같습니다.

```bash
ros2 topic pub --once /model/rexrov/joint/thruster1_joint/cmd_thrust \
  std_msgs/msg/Float64 '{data: 0.0}'
```

제어기는 추진기의 위치와 축을 사용하여 힘을 분배해야 합니다. 로봇의 속도를 제어하려면 여러 추진기의 출력을 함께 조절해야 합니다. 실험을 끝낼 때는 명령을 보낸 모든 추진기에 중립 값을 전송합니다.

## BlueROV2와 autopilot

BlueROV2는 MAVROS를 통해 ArduSub와 연결합니다. [BlueROV2 예제](rovs.md#bluerov2-실행)를 시작한 뒤 연결 상태가 될 때까지 기다리고 명령 인터페이스를 확인합니다.

```bash
ros2 topic echo --no-daemon /mavros/state mavros_msgs/msg/State \
  --qos-reliability best_effort --qos-durability volatile --once --filter 'm.connected'
ros2 service list --no-daemon -t
ros2 topic info --no-daemon /mavros/manual_control/send --verbose
```

volatile 내구성 설정은 새로 발행되는 상태를 받습니다. 필터는 자동조종기 연결 협상 뒤 `connected: true`가 될 때까지 기다립니다. 계속 기다리면 MAVROS와 ArduSub 실행 로그, 연결 설정을 확인하세요.

수동 제어 어댑터는 `/joy`, `/keyboard/joy`를 MAVROS 명령으로 변환합니다.

- 발행 주기: **20 Hz**
- 입력 타임아웃: **0.3초**

명령 입력은 하나씩 사용하고 Arm·항법 상태를 확인하세요. 키보드·WebSocket 설정은 [ROV 예제](rovs.md)를 참고하세요.
