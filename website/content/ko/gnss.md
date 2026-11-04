# 해양 GNSS

공개 `posim_gz_gnss` 시스템은 모델의 안테나 링크에서 위성항법 수신기를 모의합니다. 안테나의 월드 위치를 지리 좌표로 변환하고 설정 가능한 가우스 위치·속도 오차를 더한 뒤, 타임스탬프와 유효 상태를 하나의 측정 메시지로 발행합니다. 수중 수신 상실, 명시적 차단 입력, 재수신과 지연을 표현합니다.

## 수신기 빌드와 장착

설정된 POSIM ROS 작업 공간에서 실행합니다.

```bash
source /opt/ros/lyrical/setup.bash
colcon build --merge-install --packages-select posim_gz_gnss posim_robot_models
source install/setup.bash
```

모델에는 원하는 위치와 차량 연결을 갖춘 안테나 링크가 필요합니다. 일반적인 이름은 `gnss_link`입니다. 월드에는 WGS 84 [위도·경도 좌표](coordinates.md)를 설정합니다. 모델 SDF 안에 다음 플러그인을 넣고 토픽·링크 이름을 맞춥니다.

```xml
<plugin filename="libposim_gnss.so" name="posim::Gnss">
  <link_name>gnss_link</link_name>
  <topic>/model/my_robot/gnss</topic>
  <enabled>true</enabled>
  <water_level>0</water_level>
  <update_rate>5</update_rate>
  <latency>0.2</latency>
  <reacquire_seconds>3</reacquire_seconds>
  <dry_margin>0.05</dry_margin>
</plugin>
```

[수신기 프로필](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json)은 `base_link` 기준 장착 위치를 제공합니다. WAM-V는 z = 1.8 m, BlueROV2 계열은 0.3 m, REXROV는 0.8 m입니다. WAM-V 프로필은 수신기를 켜고 수중 차량 프로필은 기본적으로 끕니다. 이 JSON 프로필은 모델 생성 코드가 사용하는 데이터입니다. 설치만으로 모든 로봇에 링크·플러그인이 삽입되지는 않습니다. WAM-V에는 외부 [Wave Sim 모델](surface.md)도 필요합니다.

## 설정값

| SDF 설정 | 단위·의미 | 기본값 |
| --- | --- | --- |
| `update_rate` | 시뮬레이션 1초당 표본 수, 0 초과 100 Hz 이하 | 5 |
| `latency` | 측정 전달 지연, 시뮬레이션 시간 0–10 s | 0.2 |
| `reacquire_seconds` | 연속 수면 위 상태가 필요한 수신 준비 시간, s | 3 |
| `dry_margin` | 수신 준비에 필요한 수면 위 여유 높이, m | 0.05 |
| `horizontal_stddev` | 수평 위치 표준편차, m | 0.7 |
| `vertical_stddev` | 수직 위치 표준편차, m | 1.2 |
| `velocity_stddev` | 속도 표준편차, m/s | 0.1 |
| `water_level` | 파도 토픽이 없을 때 평균 수면 z, m | 0 |
| `seed` | 수신기 난수 시드 | 42 |
| `enabled` | 수신기 사용 여부 | true |
| `link_name` | 모델 안의 안테나 링크 | `gnss_link` |
| `topic` | Gazebo 측정 토픽 | `/model/<model-name>/gnss` |
| `wave_topic` | 선택형 수면 높이 표본 토픽 | 비어 있음 |

수신기의 수치 설정은 모두 유한해야 합니다. 지연 시간, 수신 준비 시간, 수면 위 여유 높이와 표준편차는 0 이상이어야 합니다.

`wave_topic`이 비어 있으면 평균 수면을 사용합니다. 토픽을 지정하면 안테나 주변의 최신 높이 표본이 필요하며, 누락되거나 잘못된 데이터는 `surface unavailable` 상태가 됩니다. 통신 형식은 `gz.msgs.Double_V`이며 스키마 `1`, 격자 수, 주기 영역 X/Y 크기, 풍속·풍향, 파도 경사도, 시뮬레이션 시각, 행 우선 높이 배열 순서입니다. 격자 수는 2~256 사이의 정수이며 높이 값은 격자 수의 제곱 개가 필요합니다. 영역 크기는 유한한 양수여야 합니다. 시각은 유한해야 하며 현재 시뮬레이션 시각보다 최대 0.6초 전까지 허용하고 미래 시각은 허용하지 않습니다. 수신기는 이 격자를 읽으므로 호환 발행기를 별도로 구성합니다. 바람 메타데이터는 수신 여부를 결정하지 않습니다.

## 측정과 차단 상태 확인

예제 토픽의 출력을 확인합니다.

```bash
gz topic -e -t /model/my_robot/gnss
```

메시지는 double 값 15개입니다. 인덱스는 0부터 시작합니다. 설정한 지연 뒤에 도착한 측정도 취득 시각을 유지해서 사용합니다.

| 인덱스 | 필드·단위 |
| --- | --- |
| 0–3 | 스키마 `1`, 재설정 세대, 취득 시뮬레이션 시간(s), 상태 |
| 4–6 | 위도(도), 경도(도), 고도(m) |
| 7–9 | 동·북·상향 속도, m/s |
| 10–12 | 수평·수직 위치 분산(m²), 속도 분산((m/s)²) |
| 13–14 | 안테나 월드 z와 해당 위치 수면 높이, m |

상태는 `0 disabled`, `1 submerged`, `2 acquiring`, `3 fix`, `4 blocked`, `5 surface unavailable`입니다. 유효하지 않은 측정의 좌표·속도는 NaN입니다. 수면 접촉이나 차단은 대기 중인 측정을 비웁니다. 재수신에는 여유 높이보다 높은 상태를 연속 유지해야 합니다. 재설정하면 타이머·지연 표본을 비우고 세대 번호를 올립니다.

차단 입력을 발행합니다. 해제할 때는 `false`를 사용합니다.

```bash
gz topic -t /model/my_robot/gnss/blocked -m gz.msgs.Boolean -p 'data: true'
```

## 해석과 확인 방법

정지한 수면 위 안테나, 잠긴 안테나, 차단된 안테나와 수신 회복을 비교합니다. 수신 준비 시간, 지연, 재설정 후 세대 번호와 증가하는 측정 시각을 확인하세요. 데이터를 받는 쪽은 오래되거나 미래인 측정을 거부하고 유효하지 않은 상태를 처리해야 합니다. 이 플러그인은 Gazebo 측정을 발행합니다. ROS `NavSatFix`, autopilot 입력과 항법 추정에는 별도 어댑터가 필요합니다.

수신기는 위치 기반의 단순화된 모델입니다. 위성 배치, 전파 전파, 물리적 다중경로 또는 수중 음향 위치 추정을 계산하지 않습니다. 정확도 설정값은 시뮬레이션 입력이며 제조사 성능 보증이 아닙니다. [수신기 구현](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc)과 [패키지 안내](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss)를 참고하세요.
