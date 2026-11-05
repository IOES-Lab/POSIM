# 해류

POSIM은 월드의 해류를 만들고 각 로봇 위치에서 유속을 계산하며, ROS로 설정을 조절할 수 있습니다. 균일한 해류와 수심에 따라 달라지는 층상 해류는 별도 토픽과 설정을 사용합니다.

## 예제 실행

선택한 POSIM 환경(Ubuntu 네이티브, WSL, Docker 또는 macOS 원격 데스크톱)에서 [터미널 A와 B](terminals.md)를 준비하세요. 터미널 A에서 예제를 실행합니다.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=ocean_current_plugin z:=-5 paused:=false \
  gui:=true headless:=false use_teleop:=false use_web_joystick:=false
```

터미널 B에서 Gazebo와 ROS 데이터를 차례로 확인하세요.

```bash
gz topic -e -t /ocean_current
```

Ctrl+C로 Gazebo 확인 도구를 종료한 뒤 ROS 명령을 실행합니다.

```bash
ros2 topic echo /hydrodynamics/currentVelocityTopic --once
```

## 균일·수심별 해류 설정하기

[예제 월드 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world)를 따라 두 플러그인을 `<world>` 바로 안에 넣습니다. 월드 해류 플러그인을 ROS 플러그인보다 먼저 배치하세요. 비어 있지 않은 `<namespace>`와 `<constant_current>`, `<transient_current>` 블록이 모두 필요합니다. 필수 설정이 없으면 오류를 기록하고 해류를 발행하지 않습니다. 균일 해류는 속도와 수평·수직 각도의 확률 모델을 각각 사용합니다.

| 설정 | 의미 / 기본값 |
| --- | --- |
| 두 플러그인의 `namespace` | 공통 월드 네임스페이스. 예제에서는 `hydrodynamics` 지정 |
| `constant_current/use_constant_current` | 균일 해류 발행 활성화. 기본값 false |
| `constant_current/topic` | Gazebo 균일 해류 토픽. 기본값 `ocean_current` |
| `transient_current/topic_stratified` | 네임스페이스 아래의 Gazebo 층별 토픽. 기본값 `stratified_current_velocity` |
| `transient_current/databasefileName` | 설치된 `posim_worlds/worlds` 아래의 CSV. 기본값 `transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | 선택적인 사용자 CSV 경로. 지정하면 `databasefileName`보다 우선함 |
| 각 확률 모델의 `mean` | 초기 평균 속도(m/s) 또는 각도(rad). 기본값 0 |
| 각 확률 모델의 `min`, `max` | 하한·상한. 기본값 −1·1. min < max를 유지하고 mean을 범위 안에 지정 |
| 각 확률 모델의 `noiseAmp` | 무작위 증분 크기. 기본값 0 |
| 각 확률 모델의 `mu` | Gauss–Markov 감쇠 계수. 기본값 0 |
| `tidal_oscillation` | 선택형 조류 입력 블록. 아래 깊이·조류 설명 참고 |

```xml
<plugin filename="OceanCurrentWorldPlugin"
        name="posim_gz_world_plugins::OceanCurrentWorldPlugin">
  <namespace>hydrodynamics</namespace>
  <constant_current>
    <use_constant_current>true</use_constant_current>
    <topic>ocean_current</topic>
    <velocity><mean>0.3</mean><min>0</min><max>0.6</max><mu>0</mu><noiseAmp>0</noiseAmp></velocity>
    <horizontal_angle><mean>0</mean><min>-3.14</min><max>3.14</max><mu>0</mu><noiseAmp>0</noiseAmp></horizontal_angle>
    <vertical_angle><mean>0</mean><min>-1.57</min><max>1.57</max><mu>0</mu><noiseAmp>0</noiseAmp></vertical_angle>
  </constant_current>
  <transient_current>
    <topic_stratified>stratified_current_velocity</topic_stratified>
    <databasefileName>transientOceanCurrentDatabase.csv</databasefileName>
  </transient_current>
</plugin>
<plugin filename="OceanCurrentPlugin" name="posim_ros_gz_plugins::OceanCurrentPlugin">
  <namespace>hydrodynamics</namespace>
</plugin>
```

[층상 해류 CSV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv)는 북쪽 속도(m/s), 동쪽 속도(m/s), 수심(m)의 순서입니다. 첫 세 설명·헤더 행을 유지하세요. 깊이 층은 두 개 이상이어야 하며, 각 데이터 행에는 유한한 수 세 개만 넣습니다. 수심은 0 이상이고 행마다 증가해야 합니다. 파일이 없거나 형식이 잘못되면 오류를 기록하고 해류를 발행하지 않습니다.

월드 ROS 플러그인은 전체 데이터베이스를 전달하고, 모델 해류 플러그인은 차량 수심에서 해류를 계산합니다. 모델의 `<namespace>`와 hydrodynamics가 구독하는 `/model/<namespace>/ocean_current`를 맞추세요. 차량 속도에 대한 상대 유속이 유체역학 응답에 영향을 줍니다.

### 로봇 수심의 해류 적용

REXROV 빠른 시작은 균일한 `/ocean_current`를 사용합니다. 수심별 프로파일을 사용하려면 다음 플러그인을 `<model>` 바로 안에 추가하고, 기존 `gz-sim-hydrodynamics-system` 플러그인 안에 `<namespace>rexrov</namespace>`를 넣습니다. 유체역학 플러그인의 링크·밀도·항력 계수는 유지하세요. [REXROV 모델](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf)에는 모델 해류 블록이 주석 처리되어 있습니다. 중복 없이 활성화하고 작업 공간을 빌드한 뒤 환경을 불러옵니다.

```xml
<plugin filename="OceanCurrentModelPlugin"
        name="posim_gz_model_plugins::OceanCurrentModelPlugin">
  <namespace>rexrov</namespace>
  <flow_velocity_topic>ocean_current</flow_velocity_topic>
  <transient_current>
    <topic_stratified>/hydrodynamics/stratified_current_velocity_topic_database</topic_stratified>
    <velocity_north><mean>0</mean><mu>0</mu><noiseAmp>0</noiseAmp><noiseFreq>0</noiseFreq></velocity_north>
    <velocity_east><mean>0</mean><mu>0</mu><noiseAmp>0</noiseAmp><noiseFreq>0</noiseFreq></velocity_east>
    <velocity_down><mean>0</mean><mu>0</mu><noiseAmp>0</noiseAmp><noiseFreq>0</noiseFreq></velocity_down>
  </transient_current>
  <tide_oscillation>false</tide_oscillation>
</plugin>
```

| 모델 플러그인 설정 | 의미 / 기본값 |
| --- | --- |
| `namespace` | 필수 토픽 namespace. 여기서는 `rexrov`이며 hydrodynamics의 namespace와 일치시킴 |
| `flow_velocity_topic` | `/model/<namespace>/` 아래의 토픽 이름. 기본값 `ocean_current` |
| `transient_current/topic_stratified` | ROS 데이터베이스 입력. 기본값 `hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | 선택적인 방향별 확률 설정. 생략한 방향은 잡음을 사용하지 않음 |
| 각 방향의 `mean` | 초기 속도(m/s). 기본값 0. 데이터 수신 후 보간한 프로파일 값으로 대체 |
| 각 방향의 `mu` | 0~1 범위의 감쇠 계수. 경과한 시뮬레이션 시간(초)을 기준으로 적용하며 기본값은 0 |
| 각 방향의 `noiseAmp` | 보간한 속도에서 허용하는 편차(m/s). 기본값 0 |
| 각 방향의 `noiseFreq` | 갱신마다 더하는 무작위 증분의 크기. 기본값 0. 이름과 달리 Hz 단위의 주파수가 아님 |
| `tide_oscillation` | 전달받은 조류 데이터를 모델에 적용. 기본값 false |

모델 플러그인은 `/model/rexrov/ocean_current`를 Gazebo(`gz.msgs.Vector3d`)와 ROS(`geometry_msgs/msg/TwistStamped`)로 발행합니다. 프로파일을 활성화한 뒤 이 출력과 차량 응답을 함께 확인하세요.

깊이는 플러그인이 부착된 모델의 월드 위치에서 구하며 수면 위에서는 0입니다. 층 사이의 흐름은 보간하고 프로파일 범위 밖에서는 가장 가까운 층의 값을 유지합니다. 일시 정지 중이거나 시뮬레이션 시각이 반복되면 해류를 갱신하지 않습니다. 잘못된 데이터베이스 메시지는 전체를 거부하고 마지막 유효한 프로파일을 유지합니다.

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
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS `posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS `geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS `posim_interfaces/msg/StratifiedCurrentDatabase` |

정확한 namespace와 토픽 이름은 월드 SDF를 확인합니다. 로봇의 유체역학도 사용하려는 해류 입력에 맞춰 연결하세요.

## 속도 설정

서비스 이름과 자료형을 확인한 뒤 호출합니다.

```bash
ros2 service list -t
ros2 service call /hydrodynamics/set_current_velocity posim_interfaces/srv/SetCurrentVelocity \
  '{velocity: 0.3, horizontal_angle: 0.0, vertical_angle: 0.0}'
```

속도는 m/s, 두 각도는 라디안입니다. 예제는 두 각도를 0으로 설정해 수평 흐름을 만듭니다. 반환된 성공 값과 이후 해류 데이터를 확인하세요.

속도·수평각·수직각 모델 서비스는 `GetCurrentModel`, `SetCurrentModel`을 사용합니다. 평균, 상·하한, 잡음 진폭과 `mu`를 설정합니다. 깊이별 서비스는 stratified 자료형을 사용합니다. 예를 들어 특정 층을 변경하기 전에 층 속도 요청과 응답의 필드를 확인합니다.

```bash
ros2 interface show posim_interfaces/srv/SetStratifiedCurrentVelocity
```

각 요청은 전체가 적용되거나, 해류를 변경하지 않고 `success: false`를 반환합니다. 값은 유한하고 설정한 범위 안에 있어야 합니다. 속도 범위는 0 이상, 수평각 범위는 −π~π, 수직각 범위는 −π/2~π/2로 지정합니다. 모델 설정은 전역 모델과 모든 깊이 층에 적용되며 각 층의 평균은 유지됩니다. 따라서 새 범위에는 각 층의 평균도 포함되어야 합니다.

### 확률 모델과 층별 유속 조정하기

예제의 ROS namespace는 `/hydrodynamics`입니다. `ros2 service list -t`에서 실제 이름을 확인한 뒤 호출하세요.

```bash
ros2 service call /hydrodynamics/get_current_velocity_model \
  posim_interfaces/srv/GetCurrentModel '{}'
ros2 service call /hydrodynamics/set_current_velocity_model \
  posim_interfaces/srv/SetCurrentModel \
  '{mean: 0.3, min: 0.0, max: 0.6, noise: 0.0, mu: 0.0}'
ros2 service call /hydrodynamics/set_stratified_current_velocity \
  posim_interfaces/srv/SetStratifiedCurrentVelocity \
  '{layer: 0, velocity: 0.2, horizontal_angle: 0.0, vertical_angle: 0.0}'
```

| 서비스 | 인터페이스 | 입력 |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | 빈 요청 |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

속도는 m/s, 각도는 rad이며 `layer`는 데이터베이스의 0부터 시작하는 층 인덱스입니다. 응답의 `success`와 관측 토픽을 함께 확인하세요. 해류 SDF의 `noiseAmp`에 대응하는 ROS 서비스 필드는 `noise`입니다.

## 조류 입력 설정하기

월드 플러그인은 설정한 깊이 데이터베이스를 읽고 확률적 해류 모델을 사용합니다. 선택형 조류 구성에는 데이터베이스 또는 조화 성분, 시작 시각과 창·낙조 방향을 지정합니다. 이들은 실험 입력이며 실시간 관측 자료는 아닙니다.

프로파일을 변경할 때는 데이터 단위, 보간 범위, 월드 기준 좌표와 시간 기준을 기록합니다. 같은 초기 자세에서 수신 해류와 로봇 반응을 비교한 뒤 항법 결과를 해석하세요.

조석을 사용할 때 `<tidal_oscillation>`에 CSV 입력 또는 M2·S2·N2 같은 조화 성분, 낙조·창조(ebb/flood) 방향과 GMT 시작 시각을 설정합니다. 예제의 조화 성분 단위는 진폭 m, 위상 도, 각속도 도/시간입니다. 조석 방향 설정과 해류 서비스의 rad 단위를 구분하세요.

조류 CSV는 머리글 한 행 다음에 `YYYY-MM-DD HH:MM,event,speed` 형식의 행을 둡니다. event는 `flood`, `ebb`, `slack`이며 speed는 유한한 cm/s 값입니다. 건너뛰는 slack 행에는 `-`를 사용할 수 있습니다. 날짜는 유효하고 엄격히 증가해야 합니다. 로더는 양·음의 극값을 번갈아 유지하며 최소 두 개가 필요합니다. 시간 범위 밖에서는 첫 번째 또는 마지막 극값을 유지합니다. 파일 누락, 불완전한 설정 또는 잘못된 데이터는 오류를 남기고 월드 해류 플러그인을 비활성화합니다.

## 해류 흐름과 모델 응답 보기

<figure><a href="{{ASSET_PREFIX}}media/notion/currents-fcfc9419.gif"><img width="720" height="480" src="{{ASSET_PREFIX}}media/notion/currents-fcfc9419.gif" alt="수심별 해류와 차량 응답" loading="lazy" decoding="async"></a><figcaption>수심별 해류와 차량 응답</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-edbc9419.jpeg"><img width="5175" height="4763" src="{{ASSET_PREFIX}}media/notion/currents-edbc9419.jpeg" alt="월드·ROS·모델 해류 플러그인의 데이터 흐름" loading="lazy" decoding="async"></a><figcaption>월드·ROS·모델 해류 플러그인의 데이터 흐름</figcaption></figure>
<figure><video controls preload="none" playsinline aria-label="균일 해류의 차량 응답 영상" width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/currents-0e3c9419.mp4"></video><figcaption>균일 해류의 차량 응답 영상</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-e41c9419.gif"><img width="720" height="480" src="{{ASSET_PREFIX}}media/notion/currents-e41c9419.gif" alt="층상 해류의 차량 응답" loading="lazy" decoding="async"></a><figcaption>층상 해류의 차량 응답</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-120c9419.png"><img width="1120" height="281" src="{{ASSET_PREFIX}}media/notion/currents-120c9419.png" alt="ROS 균일 해류 출력" loading="lazy" decoding="async"></a><figcaption>ROS 균일 해류 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-9a7c9419.png"><img width="1194" height="907" src="{{ASSET_PREFIX}}media/notion/currents-9a7c9419.png" alt="ROS 층상 해류 출력" loading="lazy" decoding="async"></a><figcaption>ROS 층상 해류 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-d27c9419.png"><img width="1822" height="698" src="{{ASSET_PREFIX}}media/notion/currents-d27c9419.png" alt="층상 해류 데이터베이스 출력" loading="lazy" decoding="async"></a><figcaption>층상 해류 데이터베이스 출력</figcaption></figure>
<figure><video controls preload="none" playsinline aria-label="모델 수심의 해류 적용 영상" width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/currents-54dc9419.mp4"></video><figcaption>모델 수심의 해류 적용 영상</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-ae1c9419.png"><img width="1084" height="255" src="{{ASSET_PREFIX}}media/notion/currents-ae1c9419.png" alt="모델별 해류 속도 출력" loading="lazy" decoding="async"></a><figcaption>모델별 해류 속도 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-4a8c9419.png"><img width="801" height="111" src="{{ASSET_PREFIX}}media/notion/currents-4a8c9419.png" alt="모델 해류의 ROS 토픽" loading="lazy" decoding="async"></a><figcaption>모델 해류의 ROS 토픽</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/currents-7bec9419.png"><img width="857" height="85" src="{{ASSET_PREFIX}}media/notion/currents-7bec9419.png" alt="모델 해류의 Gazebo 토픽" loading="lazy" decoding="async"></a><figcaption>모델 해류의 Gazebo 토픽</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
