# 멀티빔 소나

POSIM 멀티빔 소나는 ray 기반 점 산란 모델로 강도·거리 데이터와 소나 영상을 생성합니다. 실행 전에 [NVIDIA/CUDA 환경](sonar-tuning.md)을 준비하세요.

## 소나 예제 실행

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=blueview_p900 world_name:=posim_multibeam_sonar \
  paused:=false x:=4 z:=2.0 yaw:=3.14 gui:=false headless:=true
```

`posim_multibeam_sonar_demo` 패키지의 별도 예제도 있습니다.

```bash
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py
```

Launch는 하나씩 실행합니다. 로봇형 예제는 적절한 소나 월드에서 `bluerov2_heavy_multibeam_sonar`를 선택합니다.

## 출력 확인

```bash
ros2 topic list -t
ros2 topic echo /sensor/multibeam_sonar/sonar_image \
  sensor_msgs/msg/Image --once --no-arr
```

RViz에서 소나 영상은 Image, `/sensor/multibeam_sonar/point_cloud`는 PointCloud2 디스플레이를 추가합니다. 원시 반사값은 `marine_acoustic_msgs`의 음향 메시지를 사용합니다. 정확한 자료형은 `ros2 topic list -t`로 찾습니다.

## 센서 구성

모델은 `type="custom" gz:type="multibeam_sonar"`를, 월드는 `multibeam_sonar_system`을 사용합니다. BlueView P900 예제의 설정은 다음과 같습니다.

| 설정 | 값 |
| --- | --- |
| 수평 빔 | 512개, 약 130° 시야 |
| 수직 ray | 300개, 약 12° 시야 |
| 범위 | 0.1–10 m |
| 주파수·대역폭 | 900 kHz / 29.9 kHz |
| 음속 | 1500 m/s |
| 센서 요청 주기 | 30 Hz |

실제 수신 주기는 계산량과 렌더링에 따라 달라집니다. 전체 값은 [센서 모델](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf)을 확인하세요.

## 변경할 설정

`<spec>`에서 `sonarFreq`, `bandwidth`, `soundSpeed`, `sourceLevel`, `maxDistance`, `raySkips`, `sensorGain`과 출력 이름을 설정합니다. Ray 범위와 처리 거리를 맞추세요. `pointCloudTopicName`은 ROS–Gazebo 브리지 설정과 일치시키고 `frameName`에 소나 광학 좌표계를 지정합니다.

`writeLog`, `writeFrameInterval`, `debugFlag`로 진단 기록과 처리 시간을 확인할 수 있습니다. 기록 자체가 실험 목적이 아니라면 처리율 측정 시 기록을 끄세요. 반복 가능한 조절 순서는 [소나 빌드와 성능](sonar-tuning.md), 모델의 연구 출처는 [인용 안내](citation.md)에 있습니다.

## 소나 센서와 월드 추가하기

`models/posim_sensor_models/description/blueview_p900/`을 새 센서 이름으로 복사합니다. 같은 이름의 `config/<센서 이름>/sensor_config.py`도 준비하고, SDF의 모델·링크·프레임과 브리지 토픽을 함께 맞춥니다. `<ray>`의 수평·수직 샘플 수와 각도, 거리 범위를 센서 사양에 맞게 설정합니다.

월드에는 Gazebo 렌더링 센서 시스템과 소나 시스템을 포함합니다. 물리·장면·사용자 센서의 나머지 구성은 [소나 월드 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world)를 참고하세요.

```xml
<plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
  <render_engine>ogre2</render_engine>
</plugin>
<plugin filename="multibeam_sonar_system" name="custom::MultibeamSonarSystem"/>
```

CUDA와 렌더링을 준비하고 작업 공간을 다시 빌드·source한 뒤, [센서 launch 인수](quickstart.md)를 사용해 새 descriptor를 선택합니다. ROS 점군 브리지는 `sensor_msgs/msg/PointCloud2`와 Gazebo `gz.msgs.PointCloudPacked`를 연결합니다. `pointCloudTopicName`과 `sensor_config.py`에 지정한 경로를 일치시키세요.

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
