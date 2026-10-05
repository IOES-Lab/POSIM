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
