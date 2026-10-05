# 물체와 작업 장면

POSIM에는 조작, 검사와 상호작용 실험용 물체 모델이 있습니다. `posim_object_models` 패키지에서 모델과 메시·설정 리소스를 함께 설치합니다.

## 물체 목록

로컬 모델은 POSIM 패키지에 설치됩니다. 예제 월드의 다른 작업 물체는 Gazebo Fuel에서 참조합니다. 아래 로컬 목록은 현재 소스의 `models/posim_object_models/description`에서 생성합니다. 디렉터리를 열어 SDF, 링크, 관절과 리소스를 확인하세요.

{{OBJECT_CATALOG}}

## 구성된 장면부터 실행

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_plug_and_socket headless:=true
```

다른 작업 장면에는 `posim_electrical_mating`과 `posim_bimanual_example`이 있습니다. Fuel 참조로 방폭 장치 패널, 암·수 플러그와 수중 항아리 검사 대상을 배치합니다. 각 자산 URI와 위치는 월드 SDF에 있습니다. 로봇 배치, 접촉과 관절 제어 실험을 선택하기 전에 살펴보세요.

## 자신의 월드에 물체 배치

환경을 불러온 패키지 리소스에서 찾을 수 있는 모델 URI를 사용합니다. 기본 구조는 다음과 같습니다. URI는 목록에서 선택한 모델의 실제 리소스 이름으로 바꿉니다.

```xml
<include>
  <uri>model://YOUR_OBJECT</uri>
  <name>task_object_1</name>
  <pose>3 0 -5 0 0 0</pose>
</include>
```

위치는 미터, 회전각은 라디안입니다. 여러 개를 넣을 때는 각 인스턴스에 다른 이름을 지정합니다. 고정된 검사 대상과 접촉으로 움직이는 물체는 동역학 조건이 다르므로 `static` 설정을 목적에 맞게 선택하세요.

## 상호작용 확인

Visual 형상은 외관을, collision 형상은 접촉을 결정합니다. 두 형상의 크기와 위치를 함께 확인하세요. 조작 결과를 해석하기 전에 질량, 관성, 관절 범위와 마찰을 검토합니다. 짧은 제어 실험으로 배치와 접촉을 확인한 뒤 [ROS 2](ros.md)로 로봇·물체 상태를 기록합니다.

모델·리소스 규칙은 [로봇 추가](custom-robots.md), 장면 구성은 [월드 목록](worlds.md)을 참고하세요.
