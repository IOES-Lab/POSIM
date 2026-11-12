# ROS 터미널과 토픽

한 터미널에서 시뮬레이터를 계속 실행하고 다른 터미널에서 데이터를 확인하거나 제어하세요. 두 터미널은 같은 ROS 배포판과 POSIM 작업공간을 불러와야 합니다.

## 데이터가 흐르는 경로

<div class="docs-flow"><div><strong>ROS 프로그램과 명령</strong><span>구독, 서비스와 제어 입력</span></div><div><strong>ROS 인터페이스</strong><span>ros_gz 브리지 또는 플러그인의 ROS 발행 기능</span></div><div><strong>Gazebo와 POSIM 플러그인</strong><span>물리, 로봇 상태와 센서 측정</span></div></div>

센서 데이터는 ROS 프로그램으로, 명령은 시뮬레이터로 전달됩니다. Gazebo 토픽은 브리지나 ROS 지원 플러그인이 발행해야 ROS에서도 보입니다. 로봇 Launch는 선택한 모델의 브리지 설정을 불러옵니다.

## 터미널 A와 B 준비

[터미널 준비](terminals.md)에서 설치 방법에 맞는 위치와 환경 설정을 확인하세요. 터미널 A에서는 [첫 시뮬레이션](quickstart.md)을 실행한 채로 두고, B에서 아래 확인 명령을 실행합니다.

아래 토픽은 소나를 끈 BlueROV2 수중 예제를 기준으로 합니다. 소나를 켰다면 `bluerov2`를 `bluerov2_heavy_multibeam_sonar`로, 수상 예제라면 `wamv`로 바꾸세요.

## 통신 구성 확인

확인 명령은 `--no-daemon`으로 ROS에 직접 연결하여 CLI 데몬의 캐시에 의존하지 않습니다.

```bash
ros2 topic list --no-daemon -t
ros2 service list --no-daemon -t
gz topic -l
ros2 topic info --no-daemon /model/bluerov2/odometry --verbose
```

`ros2 topic list`는 ROS 토픽, `gz topic -l`은 Gazebo Transport 토픽을 보여 줍니다. 프로그램을 연결하기 전에 메시지 자료형, 발행자, 구독자와 QoS를 확인하세요.

## 연속으로 값 확인하기

```bash
ros2 topic echo --no-daemon /model/bluerov2/odometry nav_msgs/msg/Odometry --field pose.pose.position
```

위치 데이터가 계속 들어와야 합니다. 로봇이 정지해 있으면 위치는 거의 같아도 측정 시각은 증가합니다. **터미널 B에서 Ctrl+C**를 누르면 확인 도구만 종료됩니다. 터미널 A의 시뮬레이션은 계속 실행됩니다.

메시지 하나만 확인하려면 `--once`를 사용하세요. 시뮬레이션 시각을 쓰는 노드는 `use_sim_time:=true`로 설정하고 `/clock`을 받아야 합니다.

## 다른 ROS 컴퓨터 연결하기

호환되는 ROS·메시지 패키지, 같은 도메인 설정과 접근 가능한 DDS 탐색·데이터 경로가 필요합니다. POSIM 전용 메시지를 구독하려면 `posim_interfaces`도 설치하세요. 발행자가 있지만 데이터가 오지 않으면 제어기를 바꾸기 전에 [문제 해결](troubleshooting.md)을 확인하세요.

## 다음 단계

- [Python으로 제어하기](python-control.md): 위치를 읽고 제한된 시간 동안 추진기 명령을 보냅니다.
- [추진기와 조이스틱 제어](robot-control.md): 차량에 맞는 제어 인터페이스를 선택합니다.
- [센서 데이터 보기와 기록](visualization.md): 영상을 표시하고 관측 데이터를 저장합니다.
