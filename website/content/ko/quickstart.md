# 첫 시뮬레이션

해양 월드에서 REXROV 한 대를 실행하고, 실시간 측정값을 받은 뒤 정상 종료해 봅니다. 첫 실행은 기본 로봇 모델과 월드를 그대로 사용하세요.

## 시작 전 준비

[Ubuntu 설치](install.md), [macOS 설치](install-macos.md) 또는 [Windows 설치](install-windows.md)를 완료하세요. [ROS 터미널과 토픽](ros.md)에 따라 같은 환경에서 터미널 두 개를 엽니다.

Docker에서는 [센서 렌더링 환경](docker.md#데스크톱-없이-영상-센서-렌더링)도 준비하세요. `headless:=true`는 Gazebo GUI를 숨기는 옵션이며 로봇의 영상 센서에는 렌더러가 필요합니다.

## 1. 로봇과 월드 시작하기

작업공간을 불러온 **터미널 A**에서 실행합니다.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

이 명령은 월드를 시작하고 z = 0보다 5 m 아래에 REXROV를 배치하며 ROS 브리지를 실행합니다. 터미널 A를 계속 실행하세요. 로봇 Launch가 월드도 시작하므로 하나의 시뮬레이션으로 실행합니다.

## 2. 실시간 위치 읽기

**터미널 B**에서 같은 작업공간을 불러온 뒤 실행합니다.

```bash
ros2 topic echo --no-daemon /model/rexrov/odometry nav_msgs/msg/Odometry --field pose.pose.position
```

위치 데이터가 계속 들어와야 합니다. 차량이 안정되는 동안 값이 조금만 변하거나 거의 같을 수 있습니다. 확인을 마치면 **터미널 B에서 Ctrl+C**를 누르세요.

## 3. 시뮬레이션 시간 확인하기

```bash
ros2 topic echo --no-daemon /clock rosgraph_msgs/msg/Clock
```

시뮬레이션 시간이 증가해야 합니다. Ctrl+C로 확인 도구를 종료하세요. 시간이 멈춰 있거나 메시지가 오지 않으면 [문제 해결](troubleshooting.md)을 확인합니다.

## 4. 세션 종료하기

**터미널 A에서 Ctrl+C**를 누르고 Launch 프로세스가 종료될 때까지 기다립니다. 토픽 확인 도구를 닫아도 시뮬레이터는 계속 실행됩니다. 다른 예제를 시작하기 전에 현재 시뮬레이션을 종료하세요.

## 이어서 배우기

- [센서 데이터 보기와 기록](visualization.md): 수중 카메라를 실행하고 영상을 확인합니다.
- [Python으로 제어하기](python-control.md): REXROV를 관찰하고 추진기 하나를 시험합니다.
- [ROV와 BlueROV2](rovs.md): ArduSub, 키보드 또는 로컬 조이스틱을 사용합니다.
- [월드 목록](worlds.md): 다른 환경을 선택하고 Launch 인수를 확인합니다.
