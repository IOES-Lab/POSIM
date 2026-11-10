# 월드 만들기

지형과 움직이는 로봇을 넣기 전에 외부 리소스 없이 실행되는 작은 월드부터 만드세요. 이 예제는 압력 센서를 알려진 수심에 고정하여 월드를 수정한 결과를 측정합니다.

## 이 문서를 실행할 위치

[터미널 준비](terminals.md)에서 설정한 Linux 터미널을 사용합니다. Ubuntu 네이티브, WSL의 Ubuntu, POSIM 컨테이너에서 같은 순서로 진행할 수 있습니다. macOS는 RDP로 접속한 Linux 데스크톱 안의 터미널을 사용하세요. 파일 수정과 빌드도 그 환경 안에서 수행하며, `POSIM_WS`는 해당 작업 공간의 경로입니다.

## 완전한 월드 파일 저장하기

POSIM 환경을 불러온 작업공간에서 아래 내용을 `calm_water.sdf`로 저장하세요. 기본 도형과 설치된 플러그인만 사용하므로 메시를 다운로드할 필요가 없습니다.

```xml
<?xml version="1.0"?>
<sdf version="1.12">
  <world name="calm_water">
    <physics name="physics" type="ignored">
      <max_step_size>0.001</max_step_size>
      <real_time_factor>1.0</real_time_factor>
    </physics>
    <gravity>0 0 -9.80665</gravity>
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
    <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands"/>
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
    <scene>
      <ambient>0.4 0.5 0.6 1</ambient>
      <background>0.05 0.15 0.25 1</background>
    </scene>
    <light name="sun" type="directional">
      <direction>-0.5 0.2 -1</direction>
      <diffuse>0.8 0.8 0.8 1</diffuse>
    </light>
    <model name="seabed">
      <static>true</static>
      <pose>0 0 -20 0 0 0</pose>
      <link name="ground">
        <collision name="collision">
          <geometry><box><size>40 40 0.2</size></box></geometry>
        </collision>
        <visual name="visual">
          <geometry><box><size>40 40 0.2</size></box></geometry>
          <material><diffuse>0.6 0.5 0.3 1</diffuse></material>
        </visual>
      </link>
    </model>
    <model name="pressure_station">
      <static>true</static>
      <pose>0 0 -2 0 0 0</pose>
      <link name="base_link">
        <visual name="marker">
          <geometry><box><size>0.2 0.2 0.2</size></box></geometry>
        </visual>
      </link>
      <plugin filename="sea_pressure_sensor"
              name="posim_gz_sensor_plugins::SubseaPressureSensorPlugin">
        <namespace>pressure_station</namespace>
        <topic>sea_pressure</topic>
        <standard_pressure>101.325</standard_pressure>
        <kPa_per_meter>9.80638</kPa_per_meter>
        <estimate_depth_on>true</estimate_depth_on>
        <update_rate>10</update_rate>
      </plugin>
    </model>
  </world>
</sdf>
```

이 월드에는 다음 요소가 들어 있습니다.

- **Physics:** 최대 시간 간격 0.001초, 목표 실시간 비율 1, 아래 방향 중력입니다.
- **Light와 Scene:** 방향성 태양광, 주변광과 배경색을 정의합니다.
- **Seabed:** z = −20 m에 놓인 고정 상자입니다. 시각 형상과 충돌 형상을 함께 정의합니다.
- **Pressure station:** z = −2 m의 고정 관측점입니다. 모델에 [수압](pressure.md) 플러그인이 연결되어 압력과 추정 수심을 발행합니다.


고정된 관측점은 수심 측정을 확인하기 위한 장치입니다. 압력 기준 수면은 월드의 z = 0 평면입니다. 이 예제가 정상 동작한 뒤 파도 형상, 부력과 차량 운동을 추가할 수 있습니다.

## 검증하고 실행하기

터미널 A에서 파일이 있는 디렉터리로 이동한 뒤 실행하세요.

```bash
gz sdf -k calm_water.sdf
gz sim -r calm_water.sdf
```

검증에 성공하면 Gazebo 창이 열립니다. `-r`은 시뮬레이션 시간을 진행하는 옵션입니다. 개체 목록에서 `pressure_station`을 선택하고 해당 개체에 시점을 맞추면 관측점 표시를 찾을 수 있습니다.

환경을 불러온 두 번째 터미널에서 확인합니다.

```bash
ros2 topic echo --no-daemon /model/pressure_station/sea_pressure sensor_msgs/msg/FluidPressure \
  --once
ros2 topic echo --no-daemon /model/pressure_station/sea_pressure_depth geometry_msgs/msg/PointStamped \
  --once
```

약 **120,938 Pa**, 수심 **2 m**가 나와야 합니다. [수압](pressure.md) 플러그인이 ROS 메시지를 직접 발행하므로 별도 ROS 브리지는 필요하지 않습니다.

## 수정한 결과 확인하기

1. 터미널 A에서 Ctrl+C로 종료합니다.
2. 관측점의 pose를 `0 0 -2 0 0 0`에서 `0 0 -4 0 0 0`으로 바꿉니다.
3. 파일을 다시 검증하고 실행합니다.
4. 두 토픽 확인 명령을 다시 실행합니다. 약 **140,551 Pa**, 수심 **4 m**가 나와야 합니다.

비교하는 동안 압력 계수는 그대로 유지하세요. 압력 증가량은 **19,612.76 Pa**여야 합니다. 원본·수정 파일과 실험 관측값을 함께 저장하세요.

## 설정이 속하는 위치 이해하기

| SDF 경로 | 단위 | 이 예제의 설정 |
|---|---|---|
| `world/physics/max_step_size` | s | 최대 적분 간격: `0.001` |
| `world/physics/real_time_factor` | — | 실제 시간 대비 시뮬레이션 시간의 목표 비율: `1.0` |
| `world/gravity` | m/s² | 중력 벡터 |
| `world/scene`, `world/light` | RGBA / 방향 | 배경, 환경광과 태양광 |
| `model/pose` | m, rad | 월드 안에서 모델의 위치와 자세 |
| `model/static` | boolean | 관측점과 해저 고정 |
| `model/plugin` | 플러그인별 단위 | 압력 측정 설정; [수압](pressure.md) 참고 |

실시간 배율은 목표값이며 실제 속도는 연산량에 따라 달라집니다. 모델의 센서 정의와 월드 시스템은 설정 위치가 다릅니다. [센서 추가와 설정](custom-sensors.md)에서 이 파일에 IMU와 ROS 브리지를 추가해 보세요.

## 해양 지형과 시스템 추가하기

- [높이맵 지형](heightmaps.md): 단순한 해저를 교체하고 시각·충돌 형상의 일치를 확인합니다.
- [로봇 추가](custom-robots.md): 기체 역학, 추진기와 로봇의 ROS 설정을 준비합니다.
- [해류](currents.md): 월드의 흐름과 이에 대한 모델의 반응을 구성합니다.
- [위도·경도 좌표](coordinates.md): 지리 원점을 설정하고 좌표 변환 서비스를 사용합니다.
- [수중 장면 효과](underwater-rendering.md): 렌더링 시스템을 물리와 구분하여 설정합니다.

패키지에 포함할 월드는 `models/posim_worlds/worlds`, 리소스는 해당 패키지의 리소스 디렉터리에 두세요. 리소스를 추가하면 작업공간을 다시 빌드하고 환경을 불러옵니다. 공유할 월드는 설치된 리소스 URI를 사용하세요.
