# 센서 데이터 보기와 기록

센서 하나를 실행하고 새 측정값이 들어오는지 확인한 뒤 뷰어를 열거나 짧은 실험을 기록하세요. 터미널 A에서는 시뮬레이터를 계속 실행하고 터미널 B에서 확인 도구를 사용합니다.

## 카메라 영상 받기

앞서 실행한 시뮬레이션을 종료하고 센서 렌더러가 준비되어 있는지 확인합니다. 디스플레이가 없는 컨테이너에서는 먼저 [Docker 렌더링 설정](docker.md)을 따르세요. 현재 Launch의 `headless:=true`는 GUI만 끕니다. 준비한 셸에서 아래 명령을 실행합니다.

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

## 시각화와 기록

RViz에서 자료형에 맞는 디스플레이를 선택합니다.

- 카메라: Image
- 점군: PointCloud2
- 좌표계·상태: TF와 위치 디스플레이

POSIM 전용 메시지는 `posim_interfaces`에 정의되어 있습니다. 별도 ROS 클라이언트에도 이를 설치하고 환경을 불러오세요.

센서 토픽과 메시지 자료형은 [카메라](camera.md), [압력](pressure.md), [DVL](dvl.md), [USBL](usbl.md), [소나](sonar.md) 참고 문서에서 확인합니다. [해류](currents.md)와 [구면 좌표](coordinates.md)는 실험 입력을 바꾸는 ROS 서비스도 제공합니다.

```bash
ros2 bag record -o rexrov_run --topics /model/rexrov/odometry /model/rexrov/imu
```

Ctrl+C를 눌러 기록을 마칩니다. 실행할 때마다 새 출력 디렉터리를 지정하세요. 저장된 토픽별 메시지 수를 확인합니다.

```bash
ros2 bag info rexrov_run
```

동일한 토픽으로 관측 데이터를 재생하기 전에 시뮬레이터를 종료합니다.

```bash
ros2 bag play rexrov_run
```
