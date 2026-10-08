# Slocum 글라이더

`glider_slocum` 모델에는 몸체, 추진기, 유체역학, IMU, 항법과 수압 인터페이스가 포함됩니다. 글라이더 실험이나 사용자 로봇 모델의 출발점으로 사용할 수 있습니다.

## 해양 월드에서 실행

환경을 불러온 뒤 실행합니다.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=glider_slocum world_name:=posim_ocean_waves x:=4 z:=-1.5 \
  paused:=false gui:=false headless:=true \
  use_teleop:=false use_web_joystick:=false
```

빈 월드에서 모델 구성을 확인하려면 `world_name:=empty.sdf z:=0.2`로 바꿉니다. 디스플레이가 준비된 데스크톱에서는 `gui:=true headless:=false`로 모델을 볼 수 있습니다.

## 관측값 확인

```bash
ros2 topic list -t
gz topic -l
```

모델은 `/model/glider_slocum/` 아래에 IMU·NavSat Gazebo 토픽을 정의합니다. 대응하는 ROS 토픽과 자료형은 설치된 브리지 설정에서 확인하세요. [수압 플러그인](pressure.md)과 Gazebo Odometry 발행기도 함께 사용합니다.

Launch로 모델과 환경을 실행하고, 사용자 제어 노드로 잠수·활강 임무를 수행합니다.

## 모델 변경

[Slocum SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf)와 브리지 설정을 함께 확인하세요. 질량, 관성, 충돌 형상과 유체역학 계수를 함께 검토합니다. 추정기나 제어기를 연결하기 전에 좌표계도 확인하세요.

별도 모델을 만들려면 [로봇 추가](custom-robots.md)를, 관측값 기록과 제어는 [ROS 2와 제어](ros.md)를 따라 진행합니다. 원래 모델은 비교 기준으로 남겨두세요.
