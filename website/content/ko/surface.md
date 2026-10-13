# 수상 로봇과 파도

수상 로봇 통합은 업스트림 Wave Sim 의존성, WAM-V 모델과 ArduRover의 보트 제어를 연결합니다. 기본 POSIM 설치 도우미와 Docker 구성에서 외부 파도 라이브러리를 함께 빌드합니다.

## WAM-V 통합 빌드

선택형 수상 로봇 이미지는 ARM64 기본 이미지 구조를 사용합니다. POSIM 소스 디렉터리에서 [Docker 환경](docker.md)에 따라 `posim:dev-arm64-rdp`를 먼저 만든 뒤 실행합니다.

```bash
docker build -f extras/surface/Dockerfile \
  --build-arg POSIM_BASE_IMAGE=posim:dev-arm64-rdp -t posim:surface .
docker run --rm -it --entrypoint bash posim:surface -lc \
  'source /opt/ros/lyrical/setup.bash; source /home/docker/posim_ws/install/setup.bash; ros2 launch /opt/posim-surface/wamv.launch.py headless:=true'
```

이미지에는 ArduSub와 ArduRover가 각각 별도 바이너리로 들어 있습니다. 단일 로봇 예제에서는 ArduRover의 보트 구성(`FRAME_CLASS=2`)을 선택합니다. 여러 로봇을 실행할 때는 컨트롤러 식별자와 통신 포트를 각각 배정해야 합니다.

## 모델, 제어기와 위치 정보

`extras/surface/wamv.py`가 외부 의존성의 모델을 읽고 통합 설정을 적용합니다. 뒤쪽 두 추진기는 출력 1과 3으로 구동합니다. 예제 로봇은 수면에서 시작합니다.

외부 항법 어댑터는 **Gazebo의 모의 위치·속도**를 MAVROS에 발행합니다. 임무를 실행하기 전에 제어기 상태·항법 유효성·추진 방향을 확인하세요.

## 파도 조절

수상 월드의 파도 제어 토픽은 `/world/wwos_gebco/waves`이며 `gz.msgs.Param` 메시지를 사용합니다. 조절하는 값은 모두 double입니다.

| 설정 | 단위 | 의미 |
| --- | --- | --- |
| `wind_speed` | m/s | 파도 모델의 풍속 |
| `wind_angle` | 도 | 파도 모델의 풍향 |
| `steepness` | 무차원 | 파도 기울기 |

화면과 유체역학 설정을 일치시키세요. 스펙트럼 파도 모델이 수면 움직임을 만들며 [해류](currents.md)는 별도로 설정합니다. 파도 라이브러리를 설치한 뒤에도 사용할 월드에 해당 시스템을 포함하고 설정해야 합니다.

## 의존성 관리

`extras/surface/dependency.json`에서 사용할 커밋을 지정합니다. 도우미는 업스트림 소스를 받아 `/opt/waves`에 라이브러리를 설치하고 `/opt/asv_wave_sim`에 소스를 보관합니다. Git 서브모듈이 아니라 빌드 의존성입니다.

자세한 빌드는 [수상 로봇 통합 소스 안내](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)를, 구성 요소의 출처는 [라이선스 안내](citation.md)를 참고하세요.
