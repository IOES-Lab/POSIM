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
