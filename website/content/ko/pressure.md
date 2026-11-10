# 수압

수압 플러그인은 로봇의 깊이로 정수압을 계산해 ROS `FluidPressure` 메시지로 발행합니다. 압력에서 구한 깊이 추정값도 발행할 수 있습니다.

## REXROV에서 수압 확인

[준비한 터미널 A](terminals.md)에서 REXROV를 실행합니다.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=rexrov world_name:=posim_ocean_waves z:=-5 paused:=false \
  gui:=true headless:=false use_teleop:=false use_web_joystick:=false
```

같은 방식으로 준비한 터미널 B에서 확인합니다.

```bash
ros2 topic echo /model/rexrov/sea_pressure sensor_msgs/msg/FluidPressure --once
ros2 topic echo /model/rexrov/sea_pressure_depth geometry_msgs/msg/PointStamped --once
```

압력 단위는 **Pa**, 분산은 Pa²입니다. 깊이 메시지는 미터를 사용합니다.

## 모델에 플러그인 추가

깊이를 측정할 `<model>` 바로 안에 다음 블록을 넣습니다. `namespace`는 출력 토픽을 구분하는 식별자입니다.

```xml
<plugin filename="sea_pressure_sensor"
        name="posim_gz_sensor_plugins::SubseaPressureSensorPlugin">
  <namespace>my_robot</namespace>
  <topic>sea_pressure</topic>
  <standard_pressure>101.325</standard_pressure>
  <kPa_per_meter>9.80638</kPa_per_meter>
  <estimate_depth_on>true</estimate_depth_on>
  <update_rate>10</update_rate>
</plugin>
```

출력 경로는 `/model/<namespace>/<topic>`, 깊이 출력에는 `_depth`가 붙습니다. 플러그인이 부착된 모델을 측정하므로 `namespace`는 모델 이름과 달라도 됩니다.

## 설정과 단위

| 설정 | 의미 |
| --- | --- |
| `namespace` | 필수 출력 접두 경로. 유효한 ROS 토픽 경로를 구성해야 함 |
| `topic` | 출력 이름. 기본값 `sea_pressure` |
| `standard_pressure` | 유한한 0 이상의 수면 기준 압력, **kPa**. 기본값 101.325 |
| `kPa_per_meter` | 유한한 양수인 깊이당 압력 증가, **kPa/m**. 기본값 9.80638 |
| `estimate_depth_on` | 추정 깊이 발행. 기본값 true |
| `update_rate` | 유한한 발행 빈도(Hz). 기본값 0. 0 이하는 시뮬레이션 시간이 진행되는 매 물리 업데이트 |
| `saturation` | 읽어 들이는 값의 기본값은 3000. 구현에서 압력 제한에 사용하지 않음 |

모델은 월드 좌표에서 `depth = max(0, -z)`를 사용하며, 중첩 모델의 상위 모델 변환도 반영합니다. 압력의 기준 수면은 파도 높이와 무관하게 월드 z = 0 평면입니다. kPa로 계산한 값을 Pa로 바꾸어 발행합니다.

ROS 압력·추정 깊이와 Gazebo 압력 메시지는 같은 시뮬레이션 타임스탬프를 사용합니다. 일시정지 중이거나 같은 시각이 반복되면 새 측정값을 발행하지 않으며, 시간이 진행되거나 되감기면 발행을 재개합니다. 설정이 잘못되면 플러그인은 비활성 상태로 남습니다. 모델 자세가 없거나 유한하지 않은 경우, 또는 계산값이 표현 범위를 넘는 경우에는 압력 샘플을 발행하지 않습니다.

소스는 고정 분산 9,000,000 Pa²를 발행하며 압력에 무작위 잡음을 더하지 않습니다. 추정기에 분산을 적용할 때 이 특성을 고려하세요. 압력·깊이 계산은 [플러그인 구현](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc)에서 확인할 수 있습니다.

## 깊이 실험 확인

깊이당 압력 증가량을 고정하고, 알려진 두 z 위치에서 값을 비교하세요. 압력 차이는 설정한 증가량에 두 위치의 깊이 차이를 곱한 값이어야 합니다. 추정 깊이는 같은 압력 모델에서 계산하므로 독립된 깊이 측정값은 아닙니다. 추정기의 단위와 기준 압력도 맞춰주세요.

## 수압과 수심 출력 보기

<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-ba8c9419.png"><img width="1419" height="923" src="{{ASSET_PREFIX}}media/notion/pressure-ba8c9419.png" alt="ROS 수압 관측 출력" loading="lazy" decoding="async"></a><figcaption>ROS 수압 관측 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-8cdc9419.png"><img width="1842" height="892" src="{{ASSET_PREFIX}}media/notion/pressure-8cdc9419.png" alt="Gazebo 수압 관측 화면" loading="lazy" decoding="async"></a><figcaption>Gazebo 수압 관측 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/pressure-6c1c9419.png"><img width="1526" height="830" src="{{ASSET_PREFIX}}media/notion/pressure-6c1c9419.png" alt="수압으로 계산한 수심 출력" loading="lazy" decoding="async"></a><figcaption>수압으로 계산한 수심 출력</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
