# DVL

도플러 속도계(DVL)는 해저 또는 주변 물에 대한 상대 속도를 추정합니다. POSIM은 Gazebo DVL 시스템과 ROS 브리지를 사용하여 관측값을 `posim_interfaces/msg/DVL`로 변환합니다.

## 센서 실행

[설치 방식에 맞게 준비한 터미널 A](terminals.md)에서 실행합니다.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world z:=-30 \
  paused:=false gui:=true headless:=false
```

준비한 터미널 B에서 Gazebo 데이터를 확인하고 **Ctrl+C**로 확인 도구를 종료한 뒤 ROS 데이터를 확인합니다.

```bash
gz topic -e -t /dvl/velocity
ros2 topic echo /dvl/velocity posim_interfaces/msg/DVL --once
```

새 센서를 만들었다면 모델의 토픽 이름에 맞춥니다.

## SDF 구성

DVL 처리와 렌더링을 위해 다음 시스템을 `<world>` 바로 안에 넣습니다.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="gz-sim-dvl-system"
        name="gz::sim::systems::DopplerVelocityLogSystem"/>
```

POSIM 브리지는 센서의 `<model>` 안에 넣습니다. 토픽은 `<link>` 안의 DVL 센서 `<topic>`과 일치시킵니다.

```xml
<plugin filename="DVLBridge" name="posim_ros_gz_plugins::DVLBridge">
  <topic>/dvl/velocity</topic>
</plugin>
```

[전체 DVL500-300 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf)에 `type="custom" gz:type="dvl"` 센서와 네 빔, 추적 블록이 있습니다. [월드 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world)는 해저 추적용 지형을 제공합니다.

## 설정값과 단위

`DVLBridge`에는 기본값이 `/dvl/velocity`인 `topic` 매개변수 하나가 있습니다. 해당 경로로 Gazebo 데이터를 구독하고 ROS 데이터를 발행합니다. 아래 나머지 설정은 Gazebo 센서에 속하며 DVL500-300 예제 값을 보여줍니다.

| 센서 SDF 항목 | 의미 | 예제 값 |
| --- | --- | --- |
| `topic`, `update_rate` | 관측 경로와 요청 갱신 빈도(Hz) | `/dvl/velocity`, 8 |
| `gz:dvl/type` | 변환기 배치 | `phased_array` |
| `arrangement` 빔의 `aperture`, `rotation`, `tilt` | 각도. `degrees="true"`이면 도 단위 | 개구각 3°, 회전 −135/135/45/−45°, 기울기 25° |
| `minimum_range`, `maximum_range` | 음향 관측 범위(m) | 0.3, 200 |
| `reference_frame` | 기준 자세: 위치(m), 회전(rad) | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | 추적 방식 선택 | 모두 `best` |
| 추적 `noise/stddev` | 속도 잡음 표준편차(m/s) | 해저 0.005, 수괴 0.015 |
| 수괴 `water_velocity` | x/y/z 속도 성분(m/s) | 0/0/0 |
| 수괴 `boundaries/near`, `boundaries/far`, `bins` | 샘플 구간 경계(m)와 구간 수 | 10, 100, 10 |
| 추적 `visualize` | 샘플링 형상 표시 | true |

메시지 변환은 [브리지 구현](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc)을 참고하세요. 깊이 센서는 Gazebo 렌더링을 사용하므로 먼저 [실행 환경](requirements.md)을 준비하세요.

## 포함된 모델

| 모델 | 구성 계열 |
| --- | --- |
| `nortek_dvl500_300` | Nortek DVL500 |
| `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300` | Nortek DVL1000 |
| `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000` | Teledyne Explorer |
| `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL과 [소나](sonar-tuning.md) |

센서 모델의 샘플링과 형상은 SDF에서 확인하세요.

## DVL 설정

사용자 센서는 `gz:type="dvl"`로 설정합니다. 빔 각도·기울기, 해저·수괴 추적, 속도 잡음과 기준 좌표를 지정합니다.

- DVL500-300 설정 갱신 빈도: **8 Hz**
- 관측 범위: **0.3–200 m**

예제는 `<reference_frame>`에서 ENU를 설정된 전방·우현·아래 방향으로 변환합니다. ROS 브리지는 Gazebo의 좌표계 이름을 유지하며 `::` 구분자를 `/`로 바꿉니다. 추정기가 같은 좌표계를 사용하고 해저 추적과 수괴 추적을 구분하는지 확인하세요.

종합 속도와 각 빔에는 ROS의 6×6 twist 공분산이 포함됩니다. 측정된 3×3 선속도 공분산은 왼쪽 위 블록에 들어가며 단위는 m²/s²입니다. DVL이 각속도를 추정하지 않으므로 나머지 항목은 0입니다. 이를 불확실성이 0인 측정값으로 해석하지 마세요.

## 관측값 확인

메시지 정의는 `ros2 interface show posim_interfaces/msg/DVL`로 확인합니다. 정지 상태와 제어된 이동에서 속도, 좌표계와 상태를 비교하세요. 해저 형상, 범위와 빔 교차에 따라 관측값이 달라집니다. 디스플레이가 준비된 환경에서는 모델의 시각화 설정을 켜 빔을 확인할 수 있습니다.

## DVL 빔과 출력 보기

<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="해저를 향하는 DVL 음향 빔" loading="lazy" decoding="async"></a><figcaption>해저를 향하는 DVL 음향 빔</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif"><img width="754" height="476" src="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif" alt="DVL 속도 관측 출력" loading="lazy" decoding="async"></a><figcaption>DVL 속도 관측 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png"><img width="473" height="417" src="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png" alt="Gazebo DVL 토픽 확인 화면" loading="lazy" decoding="async"></a><figcaption>Gazebo DVL 토픽 확인 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png"><img width="528" height="87" src="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png" alt="ROS DVL 토픽 확인 화면" loading="lazy" decoding="async"></a><figcaption>ROS DVL 토픽 확인 화면</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
