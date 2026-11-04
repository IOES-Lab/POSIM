# 수중 카메라

`UnderwaterCamera` 플러그인은 깊이 정보를 이용해 거리가 멀어질수록 RGB 색의 강도를 줄입니다. 표현하려는 물의 특성에 맞춰 감쇠와 배경색을 조절하세요. 장면 전체의 흐림·흡수·햇빛 무늬·부유 입자(마린 스노)·해저 격자와 실시간 제어에는 공개 [수중 장면 렌더러](underwater-rendering.md)를 사용합니다.

## 예제 실행

이 센서 예제는 깊이 기반 카메라 필터를 사용합니다. 장면 렌더러는 별도의 [빌드·월드 플러그인·외관 설정](underwater-rendering.md)을 사용합니다.

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=underwater_camera world_name:=camera_tutorial \
  x:=10 z:=-93.5 pitch:=0.3 yaw:=3.14 \
  paused:=false gui:=false headless:=true
```

같은 환경의 두 번째 터미널에서 확인합니다.

```bash
ros2 topic echo /underwater_camera/simulated_image \
  sensor_msgs/msg/Image --once --no-arr
ros2 topic hz /underwater_camera/simulated_image
```

RViz의 Image 디스플레이에서 결과를 확인하세요. GUI가 없어도 렌더러는 필요합니다. 토픽은 검색되지만 영상 수신이 멈추면 [큰 영상 메시지의 전송 설정](troubleshooting.md#영상-토픽은-보이지만-수신이-멈출-때)을 확인하세요.

## 플러그인 설정

모델은 RGBD 센서와 `UnderwaterCamera` 플러그인, `posim_gz_sensor_plugins::UnderwaterCamera` 클래스를 사용합니다. 센서·데이터 토픽을 모델 설정과 맞춥니다.

모델의 `<link>` 안에 다음 센서를 넣습니다. 월드에는 `<render_engine>ogre2</render_engine>`를 지정한 `gz-sim-sensors-system`도 필요합니다. [카메라 월드](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world)를 참고하세요. 플러그인은 Gazebo RGB·깊이 영상을 읽고 ROS 출력 `/underwater_camera/simulated_image`를 직접 발행합니다.

```xml
<sensor name="underwater_camera" type="rgbd_camera">
  <update_rate>10</update_rate>
  <topic>underwater_camera</topic>
  <camera>
    <horizontal_fov>1.05</horizontal_fov>
    <image><width>320</width><height>240</height></image>
    <clip><near>0.1</near><far>10.0</far></clip>
  </camera>
  <plugin filename="UnderwaterCamera"
          name="posim_gz_sensor_plugins::UnderwaterCamera">
    <attenuationR>0.8</attenuationR>
    <attenuationG>0.5</attenuationG>
    <attenuationB>0.2</attenuationB>
    <backgroundR>85</backgroundR>
    <backgroundG>107</backgroundG>
    <backgroundB>47</backgroundB>
  </plugin>
</sensor>
```

| SDF 설정 | 의미 | 코드 기본값 |
| --- | --- | --- |
| `attenuationR` | 미터당 빨강 채널 감쇠, 0 이상의 유한값 | `1/30` |
| `attenuationG` | 미터당 초록 채널 감쇠, 0 이상의 유한값 | `1/30` |
| `attenuationB` | 미터당 파랑 채널 감쇠, 0 이상의 유한값 | `1/30` |
| `backgroundR` | 빨강 배경 채널, 0–255 정수 | `0` |
| `backgroundG` | 초록 배경 채널, 0–255 정수 | `0` |
| `backgroundB` | 파랑 배경 채널, 0–255 정수 | `0` |

센서의 `<topic>`은 유효한 ROS 토픽 이름을 만들어야 합니다. 생략하면 Gazebo의 `/world/<world>/model/<model>/link/<link>/sensor/<sensor>` 범위를 사용하며, 그 아래 `/simulated_image`에서 ROS 영상을 발행합니다. 중첩 모델에는 부모 모델 범위가 추가됩니다.

깊이를 거리로 바꿀 때는 직접 지정한 카메라 렌즈 투영값, 직접 지정한 렌즈 내부 파라미터, 영상 크기와 수평 시야각 순으로 적용합니다. 주점 좌표는 0으로 지정할 수 있습니다. 초점 거리는 양수이고 렌즈 값은 유한해야 하며, 미터 단위 절단 거리는 `0 < near < far`를 만족해야 합니다. 유효하지 않은 설정은 로그에 오류를 남기고 플러그인 출력을 비활성화합니다.

예제 모델의 설정은 다음과 같습니다. SDF 설정값이 코드 기본값보다 우선합니다.

- RGB 감쇠: **0.8, 0.5, 0.2**
- RGB 배경: **85, 107, 47**
- 영상 크기: **320 × 240**
- 설정한 갱신 빈도: **10 Hz**

## 조절과 비교

알아보기 쉬운 물체를 알려진 거리에 배치합니다. 위치, 조명, 재질과 카메라 설정을 유지하고 감쇠 채널을 하나씩 변경합니다. 화면과 함께 영상 타임스탬프·수신 주기도 확인하세요.

카메라 감쇠는 센서 영상에 적용됩니다. 장면과 카메라의 PBR 표면에 공통 수중 효과를 적용하려면 [장면 렌더러](underwater-rendering.md)를 사용합니다. 네이티브 시스템이 활성화된 경우 `POSIM_UNDERWATER_PLUGIN`은 이 호환 출력을 그대로 통과시켜 같은 픽셀의 중복 감쇠를 막습니다. 입력 연결과 출력 이름은 [카메라 모델](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf)에서 확인할 수 있습니다.

## RGBD 센서와 물 색 설정하기

RGBD 센서는 색 영상과 깊이 영상을 제공합니다. 플러그인은 깊이로 계산한 거리와 RGB 채널별 감쇠·배경값으로 수중 영상을 만듭니다. 감쇠는 채널별 지수 감쇠 모델이며, 배경 채널은 0–255 범위입니다.

감쇠 없는 비교 영상은 `attenuationR`, `attenuationG`, `attenuationB`와 세 배경값을 모두 0으로 지정합니다. 탁한 연안 설정은 위의 예제 값 0.8·0.5·0.2와 85·107·47을 사용합니다. 태그를 생략하면 코드 기본값이 적용됩니다.

이 센서는 거리별 RGB 감쇠와 배경 혼합을 사용합니다. [장면 렌더러](underwater-rendering.md)는 햇빛·해저 햇빛 무늬와 선택형 부유 입자를 시각적으로 표현합니다. 보정된 입자 산란을 연구하려면 이러한 시각 근사를 넘어서는 광학 모델이 필요합니다.

## 부유 입자

네이티브 렌더러는 RGB·RGBD 색 영상에 부유 입자를 표시할 수 있습니다. 입자는 기본적으로 꺼져 있습니다. 호스트 앱의 화면 효과 설정이나 렌더러의 [화면 효과 서비스](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering)로 켭니다.

입자는 월드 안의 공통 위치에 배치되며 시뮬레이션 시간에 따라 일정한 속도로 이동합니다. 같은 시각에는 서로 다른 카메라도 같은 위치의 입자를 관측합니다. 불투명한 표면 뒤의 입자는 가려집니다. 표시 거리 0.05–3 m 안에서 장면과 평균 수면 높이로 범위를 제한합니다. 수치 깊이와 음향 음선의 값은 유지됩니다.

입자는 절차적으로 생성하는 광학 효과이며, 이동은 시뮬레이션 해류와 독립적입니다. 영상 기반 항법을 평가할 때는 입자를 움직이는 관측 대상으로 취급하고, 비교 실험의 화면 효과 설정을 일정하게 유지하세요.

## 수중 영상 비교

<figure><a href="{{ASSET_PREFIX}}media/notion/camera-200c9419.png"><img width="2848" height="1726" src="{{ASSET_PREFIX}}media/notion/camera-200c9419.png" alt="Gazebo 수중 카메라 실험 장면" loading="lazy" decoding="async"></a><figcaption>Gazebo 수중 카메라 실험 장면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-effc9419.png"><img width="1038" height="182" src="{{ASSET_PREFIX}}media/notion/camera-effc9419.png" alt="수중 카메라 영상 표시 화면" loading="lazy" decoding="async"></a><figcaption>수중 카메라 영상 표시 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="RGB 감쇠 0·배경 0으로 설정한 영상" loading="lazy" decoding="async"></a><figcaption>RGB 감쇠 0·배경 0으로 설정한 영상</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="RGB 감쇠 0.8·0.5·0.2와 배경 85·107·47로 설정한 탁한 연안 영상" loading="lazy" decoding="async"></a><figcaption>RGB 감쇠 0.8·0.5·0.2와 배경 85·107·47로 설정한 탁한 연안 영상</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
