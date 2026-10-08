# 첫 시뮬레이션

같은 [Ubuntu](install.md) 또는 [Docker](docker.md) 환경의 터미널 두 개를 준비하세요.

- **A:** 월드·로봇·센서 실행
- **B:** 시간과 데이터 확인

## 1. 해양 월드 실행

터미널 A에서 실행합니다.

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=true
```

터미널 B에서 시뮬레이션 시간을 확인합니다.

```bash
gz topic -e -t /world/oceans_waves/clock
```

시간이 계속 증가하는지 확인하세요. 파일 이름은 `posim_ocean_waves.world`이지만 내부 월드 이름은 `oceans_waves`입니다. 다음 예제를 시작하기 전에 토픽 확인 도구와 Launch를 차례로 Ctrl+C로 종료합니다.

## 2. REXROV 배치

로봇 Launch는 월드도 함께 시작합니다. 앞에서 실행한 월드를 종료한 뒤 진행하세요.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

터미널 B에서 위치·속도 데이터를 받습니다.

```bash
ros2 topic echo /model/rexrov/odometry nav_msgs/msg/Odometry --once
```

타임스탬프, 위치와 속도를 확인합니다. 움직이지 않는 로봇도 정상적인 Odometry를 발행할 수 있습니다. BlueROV2와 제어 인터페이스는 [ROV 예제](rovs.md)에서 이어서 다룹니다.

## 3. 카메라 영상 받기

REXROV를 종료하고 센서 렌더러가 준비되어 있는지 확인합니다. 디스플레이가 없는 컨테이너에서는 먼저 [Docker 렌더링 설정](docker.md)을 따르세요. 현재 Launch의 `headless:=true`는 GUI만 끕니다. 준비한 셸에서 아래 명령을 실행합니다.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

터미널 B에서 영상 메시지를 확인합니다.

```bash
ros2 topic echo /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
```

너비·높이가 0보다 큰지, 영상 형식과 타임스탬프가 있는지 확인합니다. `--no-arr`은 터미널에 픽셀 배열을 출력하지 않도록 하는 옵션입니다. 영상을 보려면 RViz의 Image 디스플레이에서 이 토픽을 선택하세요. 감쇠 설정은 [수중 카메라](camera.md)에서 설명합니다.

## 데스크톱에서 보기

디스플레이와 렌더러가 있는 환경에서 Gazebo 창을 엽니다.

- 로봇·센서 Launch: `gui:=true headless:=false`
- 월드 Launch: `headless:=false`

## 실행 결과 확인

각 예제에서 다음 결과를 확인하세요.

- 시뮬레이션 시간 증가
- Odometry 위치·속도 메시지
- 영상 크기·형식·타임스탬프

제어 방법은 [ROV](rovs.md), 소나는 [멀티빔 소나](sonar.md)를 참고하세요.

## 다음 실험

[DVL](dvl.md), [USBL](usbl.md), [해류](currents.md)와 [월드 목록](worlds.md)을 살펴보세요. 첫 Fuel 자산 다운로드로 시작이 지연될 수 있습니다. 데이터가 오지 않으면 시뮬레이션 시간부터 확인한 뒤 [문제 해결](troubleshooting.md)에 따라 플러그인과 리소스 로그를 확인합니다.
