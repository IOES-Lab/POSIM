# 멀티빔 소나

POSIM 멀티빔 소나는 음선 기반 점 산란 모델로 강도·거리 데이터, 소나 영상과 점군을 생성합니다. 실행할 때 WGPU·CUDA·CPU 계산 방식을 선택합니다. 먼저 [소나 빌드와 렌더링 환경](sonar-tuning.md)을 준비하세요.

## 소나 예제 실행

POSIM 작업 공간의 환경을 불러온 터미널에서 WGPU로 BlueView P900 예제를 실행합니다.

```bash
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

예제는 Gazebo와 RViz를 엽니다. RViz에서 센서의 점군을 확인할 수 있습니다. RViz 창을 열지 않으려면 `rviz:=false`를 지정하세요. Launch는 하나씩 실행하고, 다음 Launch를 시작하기 전에 Ctrl+C로 종료합니다.

## 계산 방식 선택

예제를 실행할 때 `compute_backend`를 지정합니다. 설치된 플러그인은 다시 컴파일하지 않고 다음 방식을 선택할 수 있습니다.

| 값 | 선택 방식과 요구 사항 |
| --- | --- |
| `auto` (기본값) | CUDA, WGPU, CPU 순서로 사용 가능한 방식을 선택 |
| `wgpu` | WGPU 어댑터 사용. Linux는 Vulkan, macOS에서 직접 실행할 때는 Metal |
| `cuda` | NVIDIA GPU 사용. 플러그인의 CUDA 지원과 정상 작동하는 드라이버·런타임 필요 |
| `cpu` | CPU에서 음향 반사값 계산. Gazebo 렌더링도 필요 |

실행 로그의 `Using sonar compute backend:`와 선택된 WGPU 어댑터를 확인하세요. `wgpu`나 `cuda`를 직접 지정하면 해당 방식이 초기화되어야 합니다. 사용할 수 없는 방식이나 잘못된 이름을 지정하면 해당 소나 센서를 비활성화하고 오류를 출력합니다. 자동 선택을 허용하려면 `auto`, CPU 계산을 직접 선택하려면 `cpu`를 사용하세요.

일반 센서 Launch에서는 실행 환경의 `POSIM_SONAR_COMPUTE_BACKEND`를 설정합니다.

```bash
POSIM_SONAR_COMPUTE_BACKEND=cpu \
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=blueview_p900 world_name:=posim_multibeam_sonar \
  paused:=false x:=4 z:=2.0 yaw:=3.14 gui:=false headless:=true
```

서버 모드에도 렌더링 환경이 필요합니다. 가상 디스플레이나 GPU 연결은 [Docker 렌더링 안내](docker.md)를 따르세요. 로봇형 예제는 적절한 소나 월드에서 `bluerov2_heavy_multibeam_sonar`를 선택합니다.

## 출력 확인

```bash
ros2 topic list -t
ros2 topic echo /sensor/multibeam_sonar/sonar_image \
  sensor_msgs/msg/Image --once --no-arr
ros2 topic hz /sensor/multibeam_sonar/sonar_image
```

| 토픽 | ROS 2 메시지 | 용도 |
| --- | --- | --- |
| `/sensor/multibeam_sonar/sonar_image` | `sensor_msgs/msg/Image` | RViz의 Image 디스플레이에서 소나 강도 영상 표시 |
| `/sensor/multibeam_sonar/sonar_image_raw` | `marine_acoustic_msgs/msg/ProjectedSonarImage` | 거리·빔 정보를 포함한 음향 관측값 확인 |
| `/sensor/multibeam_sonar/point_cloud` | `sensor_msgs/msg/PointCloud2` | RViz의 PointCloud2 디스플레이에서 광선의 기하 정보 표시 |

소나는 영상과 원시 음향 메시지를 ROS 2에 직접 발행합니다. 센서 설정의 브리지는 Gazebo 점군을 ROS 2로 전달합니다. 토픽 이름을 바꾼 경우 `ros2 topic list -t`로 확인하세요.

## 센서 계산 흐름

Gazebo는 장면의 광선 거리와 표면 법선을 제공합니다. 소나 모델은 음향 후방 산란을 계산하고 빔 보정과 거리 압축을 수행합니다. 계산된 음향 데이터와 표시 영상은 같은 관측을 나타냅니다. 분석에는 원시 데이터를 사용하고, 영상과 함께 gain·표시 설정을 보관하세요.

## 센서 구성

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

`<spec>`에서 `sonarFreq`, `bandwidth`, `soundSpeed`, `sourceLevel`, `maxDistance`, `raySkips`, `sensorGain`과 출력 이름을 설정합니다. 음선 거리 범위와 처리 거리를 맞추세요. `pointCloudTopicName`은 ROS–Gazebo 브리지 설정과 일치시키고 `frameName`에 소나 광학 좌표계를 지정합니다.

`writeLog`, `writeFrameInterval`, `debugFlag`로 진단 기록과 처리 시간을 확인할 수 있습니다. 기록 자체가 실험 목적이 아니라면 처리율 측정 시 기록을 끄세요. 반복 가능한 조절 순서는 [소나 빌드와 성능](sonar-tuning.md), 모델의 연구 출처는 [인용 안내](citation.md)에 있습니다.

## 소나 센서와 월드 추가하기

`models/posim_sensor_models/description/blueview_p900/`을 새 센서 이름으로 복사합니다. 같은 이름의 `config/<센서 이름>/sensor_config.py`도 준비하고, SDF의 모델·링크·프레임과 브리지 토픽을 함께 맞춥니다. `<ray>`의 수평 빔·수직 광선 수와 각도, 거리 범위를 센서 사양에 맞게 설정합니다.

월드에는 Gazebo 렌더링 센서 시스템과 소나 시스템을 포함합니다. 물리·장면·사용자 센서의 나머지 구성은 [소나 월드 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world)를 참고하세요.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="multibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

선택한 계산 방식과 렌더링을 준비하고 작업 공간을 다시 빌드하고 환경을 불러온 뒤, [센서 launch 인수](quickstart.md)를 사용해 새 센서 모델을 선택합니다. ROS 점군 브리지는 `sensor_msgs/msg/PointCloud2`와 Gazebo `gz.msgs.PointCloudPacked`를 연결합니다. `pointCloudTopicName`과 `sensor_config.py`에 지정한 경로를 일치시키세요.

## 원시 소나 데이터 기록하기

`writeLog`를 켜면 실행 작업 폴더에 CSV가 생성됩니다. `writeFrameInterval`로 기록 간격을 지정하고, 디스크 사용량을 확인합니다. [plotdata.py](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py)로 기록을 분석할 수 있습니다. `sonarImageRawTopicName`은 원시 음향 관측, `sonarImageTopicName`은 표시용 영상의 이름을 지정합니다. `sensorGain`과 `blazingSonarImage` 같은 표시 설정의 영향도 실험 설정에 기록하세요.

## 소나 장면과 관측 영상

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png"><img width="1051" height="549" src="{{ASSET_PREFIX}}media/notion/sonar-3edc9419.png" alt="점 산란 기반 멀티빔 소나의 계산 흐름" loading="lazy" decoding="async"></a><figcaption>점 산란 기반 멀티빔 소나의 계산 흐름</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="같은 장면의 RGB·깊이·소나 영상과 점군" loading="lazy" decoding="async"></a><figcaption>같은 장면의 RGB·깊이·소나 영상과 점군</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif"><img width="764" height="472" src="{{ASSET_PREFIX}}media/notion/sonar-57bc9419.gif" alt="소나 센서 예제의 영상 변화" loading="lazy" decoding="async"></a><figcaption>소나 센서 예제의 영상 변화</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif"><img width="800" height="561" src="{{ASSET_PREFIX}}media/notion/sonar-c72c9419.gif" alt="소나 관측과 장면 변화" loading="lazy" decoding="async"></a><figcaption>소나 관측과 장면 변화</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif"><img width="1920" height="1080" src="{{ASSET_PREFIX}}media/notion/sonar-2e8c9419.gif" alt="BlueROV2 탑재 소나의 관측" loading="lazy" decoding="async"></a><figcaption>BlueROV2 탑재 소나의 관측</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif"><img width="800" height="367" src="{{ASSET_PREFIX}}media/notion/sonar-488c9419.gif" alt="소나로 주변 장면을 탐색하는 예제" loading="lazy" decoding="async"></a><figcaption>소나로 주변 장면을 탐색하는 예제</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
