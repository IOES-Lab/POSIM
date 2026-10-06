# 로봇 추가

가까운 POSIM 모델을 참고하여 별도의 로봇 모델을 만듭니다. SDF는 기계적 구성과 시스템을, ROS 설정은 관측값과 제어 명령의 연결을 정의합니다.

## 1. 참고 모델 선택

Gazebo 추진기를 사용하는 로봇은 [REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov)를, ArduSub·MAVROS 통합은 [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2)를 참고합니다. 다른 구조는 [Slocum 가이드](gliders.md)에서 살펴볼 수 있습니다.

모델을 새 디렉터리에 복사합니다. 예시는 다음과 같습니다.

```text
models/posim_robot_models/
  description/my_robot/model.sdf
  description/my_robot/model.config
  meshes/my_robot/hull.stl
  config/my_robot/robot_config.py
```

`my_robot`은 사용자가 정하는 식별자입니다. Launch는 `description/<namespace>/model.sdf`와 해당 설정을 찾습니다.

## 2. 물리 모델과 자산 정의

모델·링크 이름, 질량(kg), 관성(kg·m²), 위치·회전(m·rad)과 충돌 형상을 설정합니다. STL은 형상이며 질량이나 동역학을 정의하지 않습니다. 원본 메시의 단위를 확인하고 배율을 명시하세요.

패키지의 메시 예제처럼 설치 후에도 찾을 수 있는 리소스 URI를 사용합니다. 화면과 충돌 형상의 위치를 맞춥니다. 복잡한 외관이 접촉 계산에도 필요하지 않다면 충돌에는 단순한 형상을 사용하세요.

## 3. 시스템과 ROS 연결

유체역학, 부력, 추진기 축과 센서 위치를 자신의 로봇에 맞게 변경합니다. 다른 로봇의 계수를 그대로 사용하면 다른 동역학을 모사하게 됩니다. 모델의 물성에 따라 계수를 선택하세요.

적합한 `config/<namespace>/robot_config.py`를 복사하고 엔티티·관절 이름, 토픽, 좌표계와 제어기 포트를 함께 수정합니다. 패키지는 이미 `description`, `meshes`, `config`와 리소스 훅을 설치합니다. 생성된 `install/` 파일을 직접 고치지 않습니다.

## 4. 빌드와 실행

환경을 불러온 작업 공간에서 실행합니다.

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 launch posim_demos posim_robot.launch.py --show-args
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=my_robot world_name:=posim_ocean_waves z:=-2 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

설치된 자산 경로, 질량·관성, 평형 자세, 충돌, 좌표계, 메시지와 각 축의 작은 제어 명령을 확인합니다. 소스 디렉터리 밖에서도 실행하세요. 모델 커밋과 관측값을 보관하면 비교 실험을 반복할 수 있습니다. 기록은 [ROS 2와 제어](ros.md), 자산 오류는 [문제 해결](troubleshooting.md)을 참고합니다.
