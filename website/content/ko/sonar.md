# 멀티빔 소나

POSIM 멀티빔 소나는 음선(ray) 기반 점 산란 모델로 강도·거리 데이터와 소나 영상을 생성합니다. 하나의 플러그인에서 CPU, WGPU, CUDA 계산을 선택할 수 있습니다. WGPU는 Linux의 Vulkan 또는 macOS 네이티브 환경의 Metal을 통해 여러 제조사의 GPU를 사용합니다. [계산 환경 준비와 선택](sonar-tuning.md)을 참고하세요.

## 소나 예제 실행

[POSIM 터미널 A와 B](terminals.md)를 준비하세요. 소나가 설치되어 있지 않으면 먼저 [소나 패키지를 빌드](sonar-tuning.md#build-the-sonar-packages)합니다. A에서 시뮬레이션을 실행하고 B에서 출력을 확인합니다. 첫 명령은 계산 방식을 자동으로 선택하며, 아래 전용 예제는 WGPU를 지정하고 RViz도 엽니다.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=blueview_p900 world_name:=posim_multibeam_sonar \
  paused:=false x:=4 z:=2.0 yaw:=3.14 gui:=true headless:=false
```

`posim_multibeam_sonar_demo` 패키지의 별도 예제도 있습니다.

```bash
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

예제는 하나씩 실행합니다. `wgpu`를 `cpu`, `cuda`, `auto`로 바꾸면 계산 방식을 선택할 수 있습니다. 로봇형 예제는 적절한 소나 월드에서 `bluerov2_heavy_multibeam_sonar`를 선택합니다.

## 출력 확인

```bash
ros2 topic list -t
ros2 topic echo /sensor/multibeam_sonar/sonar_image \
  sensor_msgs/msg/Image --once --no-arr --qos-reliability reliable
```

영상 발행자는 reliable QoS를 사용합니다. RViz의 Image 디스플레이를 포함해 큰 영상 프레임을 확인하는 구독자도 reliable로 설정하세요. `/sensor/multibeam_sonar/point_cloud`는 PointCloud2 디스플레이로 확인합니다. 원시 반사값은 `marine_acoustic_msgs`의 음향 메시지를 사용합니다. 정확한 자료형은 `ros2 topic list -t`로 찾습니다.

BlueView 센서 Launch는 Gazebo 시뮬레이션 시간을 ROS `/clock`으로 전달합니다. `use_sim_time`을 사용하는 도구에서는 월드가 실행 중이고 `/clock`이 증가하는지 확인하세요.

## 센서 구성

다음 시스템을 `<world>` 바로 안에 넣습니다.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="libmultibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

다음 P900 센서를 모델의 `<link>` 안에 넣습니다. Gazebo 렌더링과 CUDA를 모두 준비하세요.

```xml
<sensor name="multibeam_sonar" type="custom" gz:type="multibeam_sonar">
  <always_on>true</always_on>
  <update_rate>30.0</update_rate>
  <topic>/sensor/multibeam_sonar</topic>
  <visualize>true</visualize>
  <gz:multibeam_sonar>
  <ray degrees="false">
    <scan>
      <horizontal>
        <beams>512</beams>
        <min_angle>-1.13447</min_angle>
        <max_angle>1.13447</max_angle>
      </horizontal>
      <vertical>
        <rays>300</rays>
        <min_angle>-0.10472</min_angle>
        <max_angle>0.10472</max_angle>
      </vertical>
    </scan>
    <range>
      <min>0.1</min>
      <max>10.0</max>
    </range>
  </ray>
    <spec>
        <!-- Sonar properties -->
      <verticalFOV>12</verticalFOV>
      <sonarFreq>900e3</sonarFreq>
      <bandwidth>29.9e3</bandwidth>
      <soundSpeed>1500</soundSpeed>
      <sourceLevel>220</sourceLevel>
      <maxDistance>10</maxDistance>
      <raySkips>10</raySkips>
      <sensorGain>0.02</sensorGain>
      <blazingSonarImage>true</blazingSonarImage>
      <writeLog>false</writeLog>
      <debugFlag>false</debugFlag>
      <writeFrameInterval>5</writeFrameInterval>
      <!-- ROS publication topics -->
      <pointCloudTopicName>point_cloud</pointCloudTopicName>
      <sonarImageRawTopicName>sonar_image_raw</sonarImageRawTopicName>
      <sonarImageTopicName>sonar_image</sonarImageTopicName>
      <frameName>forward_sonar_optical_link</frameName>
    </spec>
  </gz:multibeam_sonar>
</sensor>
```

모델은 `type="custom" gz:type="multibeam_sonar"`를, 월드는 `multibeam_sonar_system`을 사용합니다. BlueView P900 예제의 설정은 다음과 같습니다.

| 설정 | 값 |
| --- | --- |
| 수평 빔 | 512개, 약 130° 시야 |
| 수직 음선 | 300개, 약 12° 시야 |
| 범위 | 0.1–10 m |
| 주파수·대역폭 | 900 kHz / 29.9 kHz |
| 음속 | 1500 m/s |
| 설정한 센서 갱신 빈도 | 30 Hz |

실제 수신 빈도는 계산량과 렌더링에 따라 달라집니다. 전체 값은 [센서 모델](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf)을 확인하세요.

## 변경할 설정

| `<spec>` 설정 | 의미 | 코드 기본값 | P900 예제 |
| --- | --- | --- | --- |
| `verticalFOV` | 음향 수직 빔 폭(도) | 10 | 12 |
| `sonarFreq` | 음향 주파수(Hz) | 900,000 | 900,000 |
| `bandwidth` | 신호 대역폭(Hz) | 29,500,000 | 29,900 |
| `soundSpeed` | 음속(m/s) | 1500 | 1500 |
| `sourceLevel` | 음원 준위(dB) | 220 | 220 |
| `maxDistance` | 처리 거리(m). ray 범위와 맞춤 | 60 | 10 |
| `raySkips` | 수직 샘플링 간격. 양의 정수 사용 | 10 | 10 |
| `sensorGain` | CPU 반사 진폭 이득; WGPU·CUDA에서는 사용하지 않음 | 0.02 | 0.02 |
| `blazingSonarImage` | 실제 시각에 따라 난수 시드 변경. false이면 1234 사용 | false | true |
| `pointCloudTopicName` | Gazebo 점군 토픽 접미사 | `point_cloud` | 동일 |
| `sonarImageRawTopicName` | ROS 원시 관측 토픽 접미사 | `sonar_image_raw` | 동일 |
| `sonarImageTopicName` | ROS 영상 토픽 접미사 | `sonar_image` | 동일 |
| `frameName` | 영상 광학 좌표계 ID | `forward_sonar_optical_link` | 동일 |
| `writeLog` | 실행별 별도 디렉터리에 원시 CSV 기록 | false | false |
| `writeFrameInterval` | CSV 기록 사이의 프레임 수. 양의 정수 사용 | 10 | 5 |
| `debugFlag` | 진단 콘솔 출력 | false | false |
| `plotScaler` | 읽고 출력하지만 영상 생성에는 사용하지 않음 | 10 | 생략 |

### 토픽과 좌표계

토픽 접미사는 센서의 `<topic>` 뒤에 붙습니다. 예제에서는 `/sensor/multibeam_sonar`입니다. 원시 관측값은 `marine_acoustic_msgs/msg/ProjectedSonarImage`를 사용합니다. 점군은 Gazebo `gz.msgs.PointCloudPacked`이며, 포함된 ROS 브리지가 필요합니다. `pointCloudTopicName`은 `sensor_config.py`와 일치시키고, `frameName`에는 소나 광학 좌표계를 지정하세요.

### 음선의 범위와 각도

`<ray>` 블록의 기본값은 수평 빔 256개, 수직 ray 3개, 수평 각도 ±π/4, 수직 각도 ±π/8, 범위 0.1–5 m입니다. 스캔 각도는 `degrees="true"`이면 도, 그 외에는 rad 단위입니다. `<gz:multibeam_sonar>` 안의 선택 항목 `<reference_frame>`은 영 자세가 기본값입니다. `<ray>`와 `<spec>` 블록을 모두 유지하세요. 위 기본값은 블록 내부 항목에 적용됩니다. 파싱과 발행 방식은 [구현](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc)에서 확인할 수 있습니다.

음선의 거리 범위와 `maxDistance`를 일치시키세요.

### 반복 실험 설정

`writeLog`, `writeFrameInterval`, `debugFlag`로 진단 기록과 처리 시간을 확인할 수 있습니다. 기록 자체가 실험 목적이 아니라면 처리율 측정 시 기록을 끄세요. 반복 가능한 조절 순서는 [소나 설정과 실험](sonar-tuning.md), 모델의 연구 출처는 [인용 안내](citation.md)에 있습니다.

### 계산 백엔드 {#compute-backend}

| 선택값 | 동작 |
| --- | --- |
| `auto` | CUDA → WGPU → CPU 순으로 사용 가능한 방식을 선택합니다. 기본값입니다. |
| `wgpu` | 호환 Vulkan·Metal 장치를 사용합니다. 초기화할 수 없으면 센서가 시작되지 않습니다. |
| `cuda` | NVIDIA GPU와 빌드·설치된 CUDA 백엔드를 사용합니다. 사용할 수 없으면 초기화에 실패합니다. |
| `cpu` | 음향 반사값을 CPU로 계산합니다. Gazebo 렌더링은 계속 필요합니다. |

전용 예제에서는 `compute_backend`를 지정합니다. 다른 Launch에서는 환경 변수 `POSIM_SONAR_COMPUTE_BACKEND`로 선택합니다. 시작 출력의 `Using sonar compute backend:`에서 실제 선택값을 확인하세요. [소나 설정과 실험](sonar-tuning.md)에서 장치 선택, 운영 환경과 계산 결과의 차이를 설명합니다.

## 소나 센서와 월드 추가하기

`models/posim_sensor_models/description/blueview_p900/`을 새 센서 이름으로 복사합니다. 같은 이름의 `config/<센서 이름>/sensor_config.py`도 준비하고, SDF의 모델·링크·프레임과 브리지 토픽을 함께 맞춥니다. `<ray>`의 수평·수직 샘플 수와 각도, 거리 범위를 센서 사양에 맞게 설정합니다.

월드에는 Gazebo 렌더링 센서 시스템과 소나 시스템을 포함합니다. 물리·장면·사용자 센서의 나머지 구성은 [소나 월드 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world)를 참고하세요.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="libmultibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

계산 백엔드와 렌더링 환경을 준비한 뒤 작업 공간을 다시 빌드하고 환경을 불러오세요. [센서 launch 인수](quickstart.md)로 새 센서 모델을 선택합니다. ROS 점군 브리지는 `sensor_msgs/msg/PointCloud2`와 Gazebo `gz.msgs.PointCloudPacked`를 연결합니다. `pointCloudTopicName`과 `sensor_config.py`에 지정한 경로를 일치시키세요.

## 원시 소나 데이터 기록하기

`writeLog=true`이면 센서마다 시뮬레이터의 작업 디렉터리 아래에 별도의 `SonarRawData_<run>_<suffix>/` 디렉터리를 만들고 경로를 출력합니다. 번호가 붙은 원시 데이터 CSV와 `SonarRawData_beam_angles.csv`가 저장되며 기존 기록은 보존됩니다. `writeFrameInterval`에는 양의 프레임 수를 지정합니다. 1보다 작은 값은 1로 보정합니다. 디렉터리를 만들 수 없으면 기록을 비활성화하고 오류를 출력합니다.

출력된 기록 디렉터리에서 설치된 [시각화 도구](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py)로 첫 프레임을 엽니다. 빔 수, 거리와 빔 각도는 CSV 파일에서 읽습니다.

```bash
ros2 run posim_multibeam_sonar_demo plotdata.py SonarRawData_000001.csv
```

그림을 PNG 파일로 저장하려면 다음 명령을 사용합니다.

```bash
ros2 run posim_multibeam_sonar_demo plotdata.py SonarRawData_000001.csv \
  --output-dir plots --no-show
```

소나 영상과 균등하게 선택한 다섯 빔의 프로파일을 저장합니다. 기록된 빔이 다섯 개보다 적으면 모두 표시합니다. 색상은 해당 프레임의 최댓값에 대한 상대 반사 수준을 나타내므로 실행 간 비교에는 원시 CSV 값을 사용하세요. `--range-max`는 표시할 최대 거리(m), `--beam-step`은 빔 샘플링 간격, `--dynamic-range`는 최댓값 아래의 표시 범위(dB, 기본값 60)입니다. 시각화 도구는 CUDA가 없어도 설치됩니다.

디스크 사용량을 확인하세요. `sonarImageRawTopicName`은 원시 음향 관측, `sonarImageTopicName`은 표시용 영상의 이름을 지정합니다. 반복 실험을 비교할 때는 음향 설정과 함께 `blazingSonarImage`도 기록합니다.

CSV의 첫 열은 거리이며, 나머지 열은 빔별 복소 반사값입니다. 그래프 스크립트는 파일에서 빔 수를 읽습니다. 각 기록에 대응하는 빔 각도 CSV를 함께 보관하고, 그래프에는 기록된 거리 값을 사용하세요.

## 소나 장면과 관측 영상

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png"><img width="1051" height="549" src="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png" alt="점 산란 기반 멀티빔 소나의 계산 흐름" loading="lazy" decoding="async"></a><figcaption>점 산란 기반 멀티빔 소나의 계산 흐름</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="같은 장면의 RGB·깊이·소나 영상과 점군" loading="lazy" decoding="async"></a><figcaption>같은 장면의 RGB·깊이·소나 영상과 점군</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif"><img width="764" height="472" src="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif" alt="소나 센서 예제의 영상 변화" loading="lazy" decoding="async"></a><figcaption>소나 센서 예제의 영상 변화</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif"><img width="800" height="561" src="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif" alt="소나 관측과 장면 변화" loading="lazy" decoding="async"></a><figcaption>소나 관측과 장면 변화</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif"><img width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif" alt="BlueROV2 탑재 소나의 관측" loading="lazy" decoding="async"></a><figcaption>BlueROV2 탑재 소나의 관측</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif"><img width="800" height="367" src="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif" alt="소나로 주변 장면을 탐색하는 예제" loading="lazy" decoding="async"></a><figcaption>소나로 주변 장면을 탐색하는 예제</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
