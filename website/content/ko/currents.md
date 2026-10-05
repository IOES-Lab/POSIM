# 해류

POSIM은 월드 해류 생성, 로봇 위치의 해류 처리와 ROS 인터페이스를 나누어 구성합니다. 균일한 흐름과 깊이에 따라 달라지는 흐름은 토픽·설정 블록이 다릅니다.

## 예제 실행

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=ocean_current_plugin z:=-5 paused:=false \
  gui:=false headless:=true use_teleop:=false use_web_joystick:=false
```

두 번째 터미널에서 각 통신 체계를 확인합니다.

```bash
gz topic -e -t /ocean_current
ros2 topic echo /hydrodynamics/currentVelocityTopic --once
```

첫 확인 도구를 종료한 뒤 다음 명령을 실행합니다.

## 시스템과 토픽

| 구성 요소 | 역할 |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | 월드 흐름과 깊이별 데이터 생성 |
| 모델 해류 플러그인 | 로봇 위치의 설정된 해류를 보간·적용 |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | ROS 관측값 발행과 설정 서비스 |

| 예제 토픽 | 통신·자료형 |
| --- | --- |
| `/ocean_current` | Gazebo `gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo 깊이별 데이터 |
| `/hydrodynamics/currentVelocityTopic` | ROS `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS `posim_interfaces/msg/StratifiedCurrentDatabase` |

정확한 namespace와 토픽 이름은 월드 SDF를 확인합니다. 로봇의 유체역학도 사용하려는 해류 입력에 맞춰 연결하세요.

## 속도 설정

서비스 이름과 자료형을 확인한 뒤 호출합니다.

```bash
ros2 service list -t
ros2 service call /set_current_velocity posim_interfaces/srv/SetCurrentVelocity \
  '{velocity: 0.3, horizontal_angle: 0.0, vertical_angle: 0.0}'
```

속도는 m/s, 두 각도는 라디안입니다. 예제는 0각도 방향의 수평 흐름을 요청합니다. 반환된 성공 값과 이후 해류 데이터를 확인하세요.

속도·수평각·수직각 모델 서비스는 `GetCurrentModel`, `SetCurrentModel`을 사용합니다. 평균, 상·하한, 잡음 진폭과 `mu`를 설정합니다. 깊이별 서비스는 stratified 자료형을 사용합니다. 특정 층을 변경하기 전에 `ros2 interface show`로 필드를 확인하세요.

## 깊이 변화와 조류 입력

월드 플러그인은 설정한 깊이 데이터베이스를 읽고 확률적 해류 모델을 사용합니다. 선택형 조류 구성에는 데이터베이스 또는 조화 성분, 시작 시각과 창·낙조 방향을 지정합니다. 이들은 실험 입력이며 실시간 관측 자료는 아닙니다.

프로파일을 변경할 때는 데이터 단위, 보간 범위, 월드 기준 좌표와 시간 기준을 기록합니다. 같은 초기 자세에서 수신 해류와 로봇 반응을 비교한 뒤 항법 결과를 해석하세요.
