# 첫 시뮬레이션

월드를 먼저 실행하고 로봇이나 센서를 추가해보겠습니다. 같은 [Ubuntu](install.md) 또는 [Docker](docker.md) 환경을 불러온 터미널 두 개를 준비하세요. A에서는 시뮬레이션을 실행하고 B에서는 데이터를 확인합니다.

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

디스플레이와 렌더러가 준비된 환경에서는 로봇·센서 Launch에 `gui:=true headless:=false`를 지정합니다. 월드 Launch는 `headless:=false`를 사용합니다. Gazebo 데스크톱 창이 열립니다. 이 문서를 보여주는 브라우저에서 시뮬레이션이 실행되는 것은 아닙니다.

## 여기서 확인하는 범위

시뮬레이션 시간, Odometry 메시지와 영상 메시지는 각각 해당 경로를 확인합니다. 위 명령은 텔레오퍼레이션과 브라우저 조이스틱을 끄며, 실물 게임패드·CUDA/WGPU 소나·모든 월드·센서 수치 정확도를 시험하지 않습니다. GUI 동작은 디스플레이가 준비된 환경에서 별도로 확인합니다.

## 다음 실험

[DVL](dvl.md), [USBL](usbl.md), [해류](currents.md)와 [월드 목록](worlds.md)을 살펴보세요. 첫 Fuel 자산 다운로드로 시작이 지연될 수 있습니다. 데이터가 오지 않으면 시뮬레이션 시간부터 확인한 뒤 [문제 해결](troubleshooting.md)에 따라 플러그인과 리소스 로그를 확인합니다.
