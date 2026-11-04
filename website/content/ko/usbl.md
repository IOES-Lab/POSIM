# USBL 위치 추정

초단기선(USBL) 위치 추정은 음향 신호로 수중 장비의 위치를 찾는 방식입니다. 이 예제는 송수신기(트랜시버) 하나와 응답기(트랜스폰더) 두 개를 연결합니다. 모든 응답기의 위치를 요청하거나 ID로 하나를 선택할 수 있습니다.

## 튜토리얼 월드 실행

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=usbl_tutorial headless:=true
```

송수신기와 응답기가 월드에 포함되어 있으므로 월드 Launch로 함께 실행하세요.

## 위치 받기

같은 환경의 두 번째 터미널에서 확인합니다.

```bash
ros2 topic echo /USBL/transceiver_manufacturer_168/transponder_location \
  posim_interfaces/msg/Location --once
ros2 topic echo /USBL/transceiver_manufacturer_168/transponder_location_cartesian \
  posim_interfaces/msg/Location --once
```

두 메시지 모두 `transponder_id`가 있지만 `x`, `y`, `z`의 의미는 다릅니다.

| 출력 | x | y | z |
| --- | --- | --- | --- |
| `transponder_location` | 방위각(도) | 거리(m) | 고도각(도) |
| `transponder_location_cartesian` | 상대 x(m) | 상대 y(m) | 상대 z(m) |

현재 구현은 월드 좌표의 위치 차이로 방향을 계산합니다. 로봇 본체 좌표의 데이터와 결합할 때 이 기준을 반영하세요.

## SDF 구성

송수신기 플러그인을 `box` 모델 안에 넣습니다. 트랜스폰더 ID는 `transponder_attached_object`의 같은 순서에 있는 모델 이름과 대응합니다.

```xml
<plugin filename="UsblTransceiver" name="posim_gz_sensor_plugins::UsblTransceiver">
  <namespace>USBL</namespace>
  <transceiver_device>transceiver_manufacturer</transceiver_device>
  <transceiver_ID>168</transceiver_ID>
  <transponder_device>transponder_manufacturer</transponder_device>
  <transponder_ID>1,2</transponder_ID>
  <transponder_attached_object>sphere,sphere2</transponder_attached_object>
  <sound_speed>1540</sound_speed>
  <enable_ping_scheduler>true</enable_ping_scheduler>
  <ping_frequency>0.5</ping_frequency>
  <interrogation_mode>common</interrogation_mode>
</plugin>
```

다음 플러그인은 `sphere` 안에 넣습니다. `sphere2`에는 `transponder_ID`를 `2`로 바꿔 추가합니다. 송수신기 이름·ID·네임스페이스는 양쪽에서 같아야 합니다.

```xml
<plugin filename="UsblTransponder" name="posim_gz_sensor_plugins::UsblTransponder">
  <namespace>USBL</namespace>
  <transceiver_device>transceiver_manufacturer</transceiver_device>
  <transceiver_ID>168</transceiver_ID>
  <transceiver_model>box</transceiver_model>
  <transponder_device>transponder_manufacturer</transponder_device>
  <transponder_ID>1</transponder_ID>
  <sound_speed>1540</sound_speed>
  <mu>0</mu>
  <sigma>0</sigma>
</plugin>
```

## 설정값과 기본값

| 설정 | 적용 대상 | 의미 / 기본값 |
| --- | --- | --- |
| `namespace` | 양쪽 | 필수 토픽 네임스페이스. 예: `USBL` |
| `transceiver_device`, `transceiver_ID` | 양쪽 | 토픽 경로를 구성하는 필수 장치 이름·ID |
| `transponder_device`, `transponder_ID` | 양쪽 | 필수 이름·ID. 송수신기에는 쉼표로 구분한 ID 목록 사용 |
| `transponder_attached_object` | 송수신기 | 필수 모델 이름 목록. ID와 같은 순서로 쉼표로 구분 |
| `transceiver_model` | 트랜스폰더 | 위치 조회에 사용할 필수 송수신기 모델 이름 |
| `sound_speed` | 양쪽 | 10 °C, z = 0에서의 기준 음속(m/s). 기본값 1540. 유한한 양수 사용 |
| `enable_ping_scheduler` | 송수신기 | 필수 불리언. false이면 핑을 직접 발행 |
| `ping_frequency` | 송수신기 | 시뮬레이션 시간 기준 자동 요청 빈도(Hz). 기본값 1, 예제 0.5. 유한한 양수 사용 |
| `interrogation_mode` | 송수신기 | `common` 또는 `individual`. 기본값 `common` |
| `mu`, `sigma` | 트랜스폰더 | 위치 잡음 평균·표준편차(m). 기본값 0·1. 유한한 값이며 sigma ≥ 0. 예제 값 0·0 |

실제 존재하는 모델 이름과 고유한 트랜스폰더 ID를 사용하세요. ID 목록과 모델 목록의 길이를 맞춥니다. 모델 형상과 배치는 [전체 예제 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world)를 참고하세요.

ID는 부호와 불필요한 앞자리 0 없이 표기한 0~2147483647 범위의 십진 정수입니다. 각 플러그인의 모델에는 링크가 있어야 합니다. 잘못된 설정이면 플러그인이 활성화되지 않습니다. 등록되지 않은 채널이나 명령 대상은 무시하며, 잘못된 채널 요청은 현재 선택을 유지합니다. 초기 채널은 목록의 첫 번째 트랜스폰더입니다. 자동 핑은 시뮬레이션 시간과 함께 일시 정지하며, 업데이트당 최대 한 번 발생합니다.

## 음향 전달 시간

위치 요청은 두 구간의 전달 시간을 모사합니다. 트랜스폰더가 요청 신호의 도달을 기다린 뒤, 송수신기가 위치 응답의 도달을 기다립니다. 각 구간의 지연은 거리를 해당 장치의 음속으로 나눈 값입니다. 소수 초 단위 지연은 시뮬레이션 시간에 따라 진행하고, 월드와 함께 일시 정지하며, 시뮬레이션 시간이 되돌아가면 취소됩니다. 각 구간은 트랜스폰더마다 한 건만 처리하며, 대기 중 추가로 받은 요청이나 응답은 무시합니다. 모든 요청의 응답을 받으려면 예상 왕복 시간보다 긴 핑 간격을 사용하세요.

유효 음속은 `sound_speed + 0.017 × depth + 4 × (temperature − 10)`입니다. 음속 단위는 m/s, 수심은 z = 0 아래의 거리(m), 수온 단위는 °C입니다. z = 0 위에서는 수심 항을 0으로 계산합니다. 기본 수온은 10 °C입니다. 예제 장치의 수온은 `/USBL/transceiver_manufacturer_168/temperature` 또는 `/USBL/transponder_manufacturer_1/temperature`에 `std_msgs/msg/Float64`를 발행하여 설정합니다. 변경값은 이후 전달 구간에 적용됩니다. 유한하지 않은 수온이나 유효 음속을 0 이하 또는 유한하지 않은 값으로 만드는 변경은 무시합니다.

이 단순 수심·수온 근사는 [DOSITS 음속 설명](https://dosits.org/decision-makers/tutorials/science/sound-speed/)을 따릅니다. 전달 거리는 직선으로 계산하며 굴절, 염분, 다중 경로, 음향 간섭은 모델링하지 않습니다. 응답 구간의 거리는 설정한 위치 잡음을 포함한 수신 위치로 계산합니다.

송수신기는 현재 interrogation 모드를 보관하여 나중에 연결된 트랜스폰더에도 전달합니다. Common 모드에서는 자동 핑마다 한 번 전체 요청을 보내고, individual 모드에서는 선택한 채널에 한 번 보냅니다.

## 모든 트랜스폰더 요청

```bash
ros2 topic pub --once /USBL/transceiver_manufacturer_168/interrogation_mode \
  std_msgs/msg/String "data: 'common'"
ros2 topic pub --once /USBL/common_interrogation_ping \
  std_msgs/msg/String "data: 'ping'"
```

Common 모드에서는 같은 출력 토픽에 여러 ID가 도착할 수 있습니다. 응답의 출처를 구분할 수 있도록 ID를 함께 보관하세요.

## 특정 트랜스폰더 선택

```bash
ros2 topic pub --once /USBL/transceiver_manufacturer_168/channel_switch \
  std_msgs/msg/String "data: '1'"
ros2 topic pub --once /USBL/transponder_manufacturer_1/individual_interrogation_ping \
  std_msgs/msg/String "data: 'ping'"
```

채널 전환은 individual 모드를 선택합니다. 채널을 대상 트랜스폰더 ID로 맞춥니다. 전체 요청을 다시 사용하려면 `common`으로 돌아갑니다.

## 장치 구성

SDF에서 장치와 요청 방식을 설정합니다. 양쪽 장치의 이름과 ID를 맞추세요.

- 장치: `namespace`, 이름·ID, 부착 물체
- 음향: 음속
- 정기 요청: `enable_ping_scheduler`, `ping_frequency`
- 튜토리얼 요청 주기: **0.5 Hz**

장치나 주기를 변경할 때 [튜토리얼 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world)를 참고하세요.

## 배치와 interrogation 모드

<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-5f8c9419.png"><img width="925" height="679" src="{{ASSET_PREFIX}}media/notion/usbl-5f8c9419.png" alt="USBL 송수신기와 트랜스폰더 배치" loading="lazy" decoding="async"></a><figcaption>USBL 송수신기와 트랜스폰더 배치</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-b388acb0.png"><img width="636" height="710" src="{{ASSET_PREFIX}}media/notion/usbl-b388acb0.png" alt="Common·individual interrogation 모드의 통신 흐름" loading="lazy" decoding="async"></a><figcaption>Common·individual interrogation 모드의 통신 흐름</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/usbl-a96c9419.png"><img width="1537" height="993" src="{{ASSET_PREFIX}}media/notion/usbl-a96c9419.png" alt="트랜스폰더 위치 관측 출력" loading="lazy" decoding="async"></a><figcaption>트랜스폰더 위치 관측 출력</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
