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
