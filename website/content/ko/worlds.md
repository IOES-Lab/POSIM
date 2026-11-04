# 월드 목록

월드는 `models/posim_worlds/worlds`에 있습니다. 파일 이름에서 `.world` 확장자를 뺀 값으로 선택합니다.

## 월드 시작

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=true
```

파일 이름과 월드 내부 이름은 다를 수 있습니다. `posim_ocean_waves.world`의 내부 이름은 `oceans_waves`이며 Gazebo 토픽·서비스 경로에 사용됩니다. 경로를 만들기 전에 `<world name="…">`를 확인하세요.

## 실험에 맞는 월드

| 실험 | 월드 |
| --- | --- |
| 첫 로봇 구성 | `posim_ocean_waves` |
| 수중 영상 효과 | `camera_tutorial` |
| DVL 관측 | `dvl_world` |
| USBL 요청·응답 | `usbl_tutorial` |
| 해류 설정 | `ocean_current_plugin` |
| 해안·지리 장면 | `posim_Santorini` |
| 소나 장면 | `posim_multibeam_sonar`, 이름에 sonar가 있는 해양 월드 |
| 조작·작업 물체 | `posim_bimanual_example`, `posim_plug_and_socket`, `posim_electrical_mating` |

소나 월드에는 [소나 실행 환경](sonar-tuning.md)이 필요합니다. Fuel을 참조하는 월드는 장면이 준비되기 전에 모델을 내려받을 수 있습니다.

## 소스 목록

아래 목록은 사이트를 빌드할 때 소스의 월드 파일에서 생성합니다. 파일을 열어 시스템, 포함 모델, 기준 좌표, 조명과 카메라 구성을 확인하세요.

| 월드 SDF |
| --- |
{{WORLD_CATALOG}}

## 다른 환경 구성

적합한 SDF를 `models/posim_worlds/worlds`에 복사하고 새 파일 이름과 내부 월드 이름을 지정합니다. 로컬 모델에는 설치 후에도 찾을 수 있는 URI를 사용합니다. 다시 빌드하고 환경을 불러온 뒤 새 파일 이름으로 실행하세요. 지형은 [높이맵 지형](heightmaps.md), 작업 물체는 [물체와 작업 장면](objects.md)을 참고합니다.

## 최초 Fuel 다운로드 확인

`posim_integrated`와 `posim_ocean_models`는 외부 Fuel 모델을 포함합니다. 시계 토픽이
발견되는 것만으로 장면 로딩 완료를 판단하지 마세요. 첫 실행의 다운로드가 끝나기
전에 반복 실행하지 말고, `verbose:=true`로 리소스 로딩 로그를 확인하세요.

같은 환경을 설정한 다른 터미널에서 장면을 조회합니다.

```bash
# posim_integrated.world의 내부 월드 이름은 posim_integrated입니다.
gz service -s /world/posim_integrated/scene/info \
  --reqtype gz.msgs.Empty --reptype gz.msgs.Scene --timeout 5000 --req ''
# posim_ocean_models.world는 /world/posim_ocean_model/scene/info를 사용합니다.
```

모델이 포함된 장면 응답, 시뮬레이션 시간 증가, 기대한 센서 출력은 각각 확인해야 합니다.
다운로드가 멈추면 상세 로그를 보존하고 Gazebo를 실행하는 PC/컨테이너의 네트워크를
확인하세요. 다운로드 중 중단된 실행을 정상 실행·종료 시험의 통과로 세면 안 됩니다.

## Launch 인수 선택하기

로봇·센서 launch에서 `--show-args`로 사용할 수 있는 인수를 확인합니다.

```bash
ros2 launch posim_demos posim_sensor.launch.py --show-args
```

| 인수 | 의미 | 센서 launch 기본값 |
| --- | --- | --- |
| `namespace` | 설치된 모델 디렉터리 선택 | 빈 값; 직접 지정 |
| `world_name` | 월드 파일 선택 | `empty.sdf` |
| `paused` | 일시 정지 상태로 시작 | `true` |
| `gui` / `headless` | Gazebo GUI 표시 / 숨김 | `true` / `false` |
| `use_sim_time` | ROS 노드의 시뮬레이션 시계 사용 | `true` |
| `x`, `y`, `z` | 초기 위치, m | 각각 `0.0` |
| `roll`, `pitch`, `yaw` | 초기 자세, rad | 각각 `0.0` |
| `debug` / `verbosity_level` | 상세 로그 / 로그 수준 | `false` / `1` |
| `use_ned_frame` | NED 프레임 사용 | `false` |

센서 예제에서는 `paused:=false`를 지정해 데이터가 생성되도록 합니다. 로봇·월드 launch의 인수는 해당 launch의 `--show-args`로 확인하세요.
