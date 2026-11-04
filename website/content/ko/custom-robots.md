# 로봇 추가

구조가 비슷한 POSIM 모델을 참고해 로봇을 만듭니다. SDF는 기계적 구성과 시스템을, ROS 설정은 관측값과 제어 명령의 연결을 정의합니다.

## 1. 참고 모델 선택

Gazebo 추진기를 사용하는 로봇은 [REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov)를, ArduSub·MAVROS 통합은 [BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2)를 참고합니다. 다른 구조는 [Slocum 가이드](gliders.md)에서 살펴볼 수 있습니다.

`~/posim_ws`에서 REXROV의 완전한 사본을 만드세요. 메시·브리지 설정·토픽 이름을 함께 복사합니다. `my_robot`이 이미 있으면 스크립트가 중단됩니다.

```python
from pathlib import Path
import shutil

root = Path('src/posim/models/posim_robot_models')
directories = ('description', 'config', 'meshes')
if any((root / directory / 'my_robot').exists() for directory in directories):
    raise SystemExit('my_robot already exists; choose another model name.')
for directory in directories:
    shutil.copytree(root / directory / 'rexrov', root / directory / 'my_robot')
for relative in ('description/my_robot/model.sdf',
                 'description/my_robot/model.config',
                 'config/my_robot/robot_config.py'):
    target = root / relative
    target.write_text(target.read_text().replace('rexrov', 'my_robot'))
```

`copy_robot.py`로 저장하고 `python3 copy_robot.py`를 실행하세요. 생성된 구조에는 다음 파일이 포함됩니다.

```text
models/posim_robot_models/
  description/my_robot/model.sdf
  description/my_robot/model.config
  meshes/my_robot/RexROV_no_props.dae
  config/my_robot/robot_config.py
```

`my_robot`은 사용자가 정하는 식별자입니다. Launch는 `description/<namespace>/model.sdf`와 해당 설정을 찾습니다.

## 2. 물리 모델과 자산 정의

첫 실행에서는 복사한 물리 설정을 유지하세요. 선체를 바꿀 때는 다음 설정을 함께 검토합니다.

| SDF 경로 | 단위 | 확인할 내용 |
|---|---|---|
| `model/@name`, 링크·관절 이름 | 식별자 | 플러그인과 ROS 설정이 사용하는 엔티티 이름과 일치 |
| `link/inertial/mass` | kg | 움직이는 각 링크의 양수 질량 |
| `link/inertial/pose` | m, rad | 질량 중심과 관성 좌표계의 방향 |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | 관성 좌표계 각 축에 대한 관성 모멘트 |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | 비대각 성분; 전체 텐서가 물리적으로 유효해야 함 |
| `link/visual`, `link/collision` | m, rad | 메시 배율과 형상의 정렬 |
| `joint/axis`, 추진기 설정 | 플러그인별 단위 | 구동기 방향과 힘 분배 |

STL은 형상만 제공합니다. 질량과 동역학은 SDF에서 정의하고, 메시의 단위와 배율을 확인하세요.

패키지의 메시 예제처럼 설치 후에도 찾을 수 있는 리소스 URI를 사용합니다. 화면과 충돌 형상의 위치를 맞춥니다. 복잡한 외관이 접촉 계산에도 필요하지 않다면 충돌에는 단순한 형상을 사용하세요.

## 3. 시스템과 ROS 연결

유체역학, 부력, 추진기 축과 센서 위치를 자신의 로봇에 맞게 변경합니다. 다른 로봇의 계수를 그대로 쓰면 움직임이 부정확할 수 있습니다. 자신의 로봇 물성에 맞는 계수를 선택하세요.

일정하거나 수심에 따라 달라지는 흐름은 [해류](currents.md)에 따라 연결합니다. 모델의 센서 배치, 월드 시스템과 ROS 경로는 [센서 추가와 설정](custom-sensors.md)을 따라 구성한 뒤 해당 플러그인의 파라미터를 확인하세요. 위도·경도가 필요한 실험에서는 [위도·경도 좌표](coordinates.md)를 사용해 월드의 지리적 원점을 설정합니다.

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

작은 제어 명령으로 각 축의 동작을 확인합니다. 소스 디렉터리 밖에서도 실행하세요.

- 설치된 자산 경로
- 질량·관성·평형 자세
- 충돌 형상과 반응
- 좌표계와 메시지

실행 후 환경을 불러온 두 번째 터미널에서 복사한 모델의 토픽을 확인하세요.

```bash
ros2 topic echo --no-daemon /model/my_robot/odometry nav_msgs/msg/Odometry --once
```

새 이름의 모델에서 위치와 속도가 들어와야 합니다. 먼저 사본을 정상 실행한 뒤 물리 설정, 센서와 추진기를 하나씩 바꾸세요. [센서 데이터 보기와 기록](visualization.md)으로 실험을 비교하고, 리소스 오류는 [문제 해결](troubleshooting.md)에서 확인합니다.
