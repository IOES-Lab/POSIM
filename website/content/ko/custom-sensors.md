# 센서 추가와 설정

센서에는 모델 정의, 이를 갱신하는 시스템과 데이터를 전달할 경로가 필요합니다. 한 번에 하나씩 추가하고 측정값을 확인한 뒤 다음 센서를 붙이세요.

## 이 문서를 실행할 위치

[터미널 준비](terminals.md)에서 설정한 Linux 터미널을 사용합니다. Ubuntu 네이티브, WSL의 Ubuntu, POSIM 컨테이너에서 같은 순서로 진행할 수 있습니다. macOS는 RDP로 접속한 Linux 데스크톱 안의 터미널을 사용하세요. 파일 수정과 빌드도 그 환경 안에서 수행하며, `POSIM_WS`는 해당 작업 공간의 경로입니다.

## 설정 위치 선택하기

| 측정 대상 | 모델 안의 위치 | 월드 시스템 / ROS 경로 |
|---|---|---|
| IMU | `link/sensor type="imu"` | Gazebo IMU 시스템; `gz.msgs.IMU`를 ROS `Imu`로 연결 |
| [수중 카메라](camera.md) | RGBD 센서와 영상 효과 플러그인 | 렌더러가 있는 Gazebo Sensors 시스템; 설정된 카메라 토픽 |
| [DVL](dvl.md) | DVL 센서와 빔 설정 | DVL 시스템과 제공되는 음향 메시지 브리지 |
| [수압](pressure.md) | `model` 바로 아래의 플러그인 | ROS 압력·수심을 직접 발행 |
| [해양 GNSS](gnss.md) | 로봇 모델의 수신기 플러그인 | 수신기에 설정된 ROS 출력 |
| [USBL 위치 추정](usbl.md) | 송수신기와 응답기 플러그인 | 일치하는 음향 ID와 설정된 ROS 인터페이스 |
| [멀티빔 소나](sonar.md) | 링크 아래의 사용자 정의 소나 센서 | Sensors 렌더러와 멀티빔 월드 시스템; CUDA 백엔드 |

각 참고 문서의 SDF와 파라미터 표를 사용하세요. 월드 시스템은 센서 내부에 넣지 않으며, 모델 플러그인은 자신이 동작할 모델에 부착해야 합니다.

## 예제 월드에 IMU 추가하기

[월드 만들기](custom-worlds.md)의 `calm_water.sdf`에서 시작하세요. 물리 시스템과 나란히 `<world>` 바로 아래에 다음 시스템을 추가합니다.

```xml
<plugin filename="gz-sim-imu-system" name="gz::sim::systems::Imu"/>
```

`pressure_station` 모델의 `<link name="base_link">` 안에 다음 센서를 추가합니다.

```xml
<sensor name="imu" type="imu">
  <pose>0 0 0.1 0 0 0</pose>
  <always_on>true</always_on>
  <update_rate>20</update_rate>
  <topic>/tutorial/imu</topic>
  <imu/>
</sensor>
```

장착 pose는 링크를 기준으로 하며 위치는 m, 회전은 rad입니다. 갱신 주기는 시뮬레이션 시간 기준 Hz입니다. 이 관측점은 고정되어 있으므로 각속도는 0에 가까워야 하며 가속도에는 중력에 대한 응답이 포함됩니다.

## 연결하고 확인하기

기존 월드를 종료하고 수정한 파일을 검증한 뒤 터미널 A에서 다시 실행합니다.

```bash
gz sdf -k calm_water.sdf
gz sim -r calm_water.sdf
```

환경을 불러온 **터미널 B**에서 브리지를 실행하고 유지하세요.

```bash
ros2 run ros_gz_bridge parameter_bridge \
  '/tutorial/imu@sensor_msgs/msg/Imu[gz.msgs.IMU'
```

환경을 불러온 **터미널 C**에서 ROS 측정값을 확인합니다.

```bash
ros2 topic echo --no-daemon /tutorial/imu sensor_msgs/msg/Imu --once
```

측정 시각, 프레임 ID, 각속도와 가속도를 확인하세요. 연속으로 보려면 `--once`를 빼고 실행합니다. Ctrl+C는 확인 도구를 종료합니다. 실험이 끝나면 브리지와 시뮬레이터도 종료하세요. [ROS 터미널과 토픽](ros.md)에서 터미널별 환경 설정을 설명합니다.

## 설정을 바꾸고 결과 확인하기

`update_rate`를 `20`에서 `5`로 바꾼 뒤 월드와 브리지를 다시 실행하세요. 연속된 메시지의 측정 시각 차이를 비교합니다. 시뮬레이션 주기가 일정하면 약 `0.05초`에서 `0.2초`로 바뀝니다. 실제 시간 기준 수신 주기는 실시간 배율과 전송 환경에도 영향을 받습니다.

카메라는 해상도·시야각·근거리와 원거리 범위를 함께 확인하세요. 스캔 센서는 빔 형상과 거리 한계를 확인합니다. 링크 좌표계와 방향을 확인한 다음 센서를 이동하세요.

## 통합 항목 확인하기

- 모델: 유효한 장착 pose, 센서 설정과 고유한 토픽.
- 월드: 필요한 시스템과 영상 센서용 렌더러.
- ROS: 올바른 브리지 방향, 메시지 자료형, 프레임과 QoS.
- 관측: 새 측정 시각과 통제된 장면 변화에 대한 응답.

해저를 측정할 때는 [높이맵 지형](heightmaps.md)으로 지형 배치를 확인하세요. 소나는 [멀티빔 소나](sonar.md)의 파라미터를 참고하면서 [소나 설정과 실험](sonar-tuning.md)에 따라 반복 가능한 조건에서 비교합니다.
