# 수중 카메라

수중 카메라 플러그인은 깊이 정보를 사용하여 렌더링된 RGB 영상에 거리별 색 감쇠를 적용합니다. 실험에서 표현하려는 물의 특성에 맞춰 감쇠와 배경색을 조절합니다.

## 예제 실행

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

RViz의 Image 디스플레이에서 결과를 확인하세요. GUI가 없어도 렌더러는 필요합니다.

## 플러그인 설정

모델은 RGBD 센서와 `UnderwaterCamera` 플러그인, `posim_gz_sensor_plugins::UnderwaterCamera` 클래스를 사용합니다. 센서·데이터 토픽을 모델 설정과 맞춥니다.

| SDF 설정 | 의미 | 코드 기본값 |
| --- | --- | --- |
| `attenuationR` | 미터당 빨강 채널 감쇠 | `1/30` |
| `attenuationG` | 미터당 초록 채널 감쇠 | `1/30` |
| `attenuationB` | 미터당 파랑 채널 감쇠 | `1/30` |
| `backgroundR` | 빨강 배경 채널 | `0` |
| `backgroundG` | 초록 배경 채널 | `0` |
| `backgroundB` | 파랑 배경 채널 | `0` |

예제 모델의 설정은 다음과 같습니다. SDF 설정값이 코드 기본값보다 우선합니다.

- RGB 감쇠: **0.8, 0.5, 0.2**
- RGB 배경: **85, 107, 47**
- 영상 크기: **320 × 240**
- 요청 주기: **10 Hz**

## 조절과 비교

알아보기 쉬운 물체를 알려진 거리에 배치합니다. 위치, 조명, 재질과 카메라 설정을 유지하고 감쇠 채널을 하나씩 변경합니다. 화면과 함께 영상 타임스탬프·수신 주기도 확인하세요.

카메라 감쇠는 센서 영상에 적용되며 Gazebo 화면 전체에 적용되지 않습니다. 월드 화면을 바꾸려면 조명·재질을 별도로 설정합니다. 입력 연결과 출력 이름은 소스와 전체 [카메라 모델](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf)에서 확인할 수 있습니다.

## RGBD 센서와 물 색 설정하기

RGBD 센서는 색 영상과 깊이 영상을 제공합니다. 플러그인은 깊이로 계산한 거리와 RGB 채널별 감쇠·배경값으로 수중 영상을 만듭니다. 감쇠는 채널별 지수 감쇠 모델이며, 배경 채널은 0–255 범위입니다.

감쇠 없는 비교 영상은 `attenuationR`, `attenuationG`, `attenuationB`와 세 배경값을 모두 0으로 지정합니다. 탁한 연안 설정은 위의 예제 값 0.8·0.5·0.2와 85·107·47을 사용합니다. 태그를 생략하면 코드 기본값이 적용됩니다.

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

수중에서는 빛의 흡수와 산란이 색·대비·가시거리에 영향을 줍니다. 이 센서의 구현은 거리별 RGB 감쇠와 배경 혼합을 사용합니다. 조명·입자 산란을 별도로 연구하려면 그 효과를 구현하는 장면과 센서 모델을 준비하세요.

## 수중 영상 비교

<figure><a href="{{ASSET_PREFIX}}media/notion/camera-200c9419.png"><img width="2848" height="1726" src="{{ASSET_PREFIX}}media/notion/camera-200c9419.png" alt="Gazebo 수중 카메라 실험 장면" loading="lazy" decoding="async"></a><figcaption>Gazebo 수중 카메라 실험 장면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-effc9419.png"><img width="1038" height="182" src="{{ASSET_PREFIX}}media/notion/camera-effc9419.png" alt="수중 카메라 영상 표시 화면" loading="lazy" decoding="async"></a><figcaption>수중 카메라 영상 표시 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="RGB 감쇠 0·배경 0으로 설정한 영상" loading="lazy" decoding="async"></a><figcaption>RGB 감쇠 0·배경 0으로 설정한 영상</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="RGB 감쇠 0.8·0.5·0.2와 배경 85·107·47로 설정한 탁한 연안 영상" loading="lazy" decoding="async"></a><figcaption>RGB 감쇠 0.8·0.5·0.2와 배경 85·107·47로 설정한 탁한 연안 영상</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
