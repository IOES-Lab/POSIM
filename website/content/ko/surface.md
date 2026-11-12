# 수상 로봇과 파도

WAM-V 수상 로봇에 Wave Sim의 파도와 ArduRover의 보트 제어를 적용해 시뮬레이션합니다. 기본 POSIM 설치 도우미와 Docker 구성에서 외부 파도 라이브러리를 함께 빌드합니다.

## 기본 튜토리얼

기본 POSIM 설치에는 카메라·라이다·RViz 설정을 갖춘 WAM-V 부표 코스 예제가 포함됩니다. 아래의 선택적 이미지를 따로 만들 필요는 없습니다. [준비한 POSIM 터미널](terminals.md)에서 실행하세요.

{{CODE:quickstart:4}}

장면, 라이다 표시, QGroundControl 영상과 조작은 [첫 시뮬레이션](quickstart.md)을 참고하세요.

## WAM-V 통합 빌드

이 선택형 이미지는 ARM64 데스크톱 구성을 사용합니다. Apple Silicon Mac에서 [Docker와 RDP 클라이언트](install-macos.md)를 준비한 뒤, macOS 터미널의 POSIM 소스 디렉토리에서 실행하세요.

```bash
docker pull ioeslab/posim:latest
docker build -f extras/surface/Dockerfile \
  --build-arg POSIM_BASE_IMAGE=ioeslab/posim:latest -t posim:surface .
docker run -d --name posim-surface --shm-size=1g \
  -p 127.0.0.1:3390:3389 posim:surface
```

RDP 클라이언트에서 **127.0.0.1:3390**에 `docker` 계정으로 접속합니다. 이 Linux 데스크톱 안의 터미널에서 로봇을 실행하세요.

```bash
source /opt/ros/lyrical/setup.bash
source /home/docker/posim_ws/install/setup.bash
ros2 launch /opt/posim-surface/wamv.launch.py headless:=false
```

이미지에는 ArduSub와 ArduRover가 각각 별도 바이너리로 들어 있습니다. 단일 로봇 예제에서는 ArduRover의 보트 구성(`FRAME_CLASS=2`)을 선택합니다. 한 환경에서 한 인스턴스만 실행하세요. 런처가 차량 이름, 생성 파일 경로와 제어기 포트를 고정해서 사용하므로 `world_name`만 바꿔서는 두 번째 인스턴스가 독립적으로 동작하지 않습니다. 여러 차량에는 파일·이름·제어기 식별자·통신 포트를 분리한 별도 통합 구성이나 격리된 환경이 필요합니다.

차량을 생성하고 IMU 데이터가 발행된 뒤 제어기가 시작됩니다. 차량 생성에 실패하거나 180초가 넘게 걸리는 경우, 또는 생성 후 180초 안에 IMU 샘플이 도착하지 않으면 Launch가 0이 아닌 상태로 종료되고 실패한 단계를 알립니다. 다시 실행하기 전에 Gazebo와 센서 로그를 확인하세요.

## 모델, 제어기와 위치 정보

`extras/surface/wamv.py`가 외부 의존성의 모델을 읽고 통합 설정을 적용합니다. 뒤쪽 두 추진기는 출력 1과 3으로 구동합니다. 예제 로봇은 수면에서 시작합니다.

외부 항법 어댑터는 **Gazebo의 모의 위치·속도**를 MAVROS에 발행합니다. 임무를 실행하기 전에 제어기 상태·항법 유효성·추진 방향을 확인하세요.

[해양 GNSS 시스템](gnss.md)으로 수신기 잡음, 수면 접촉과 재수신을 실험할 수도 있습니다. 안테나·플러그인을 명시적으로 연결하고 적절한 항법 어댑터를 구성합니다. 수상 예제의 외부 위치 어댑터는 별도의 항법 입력입니다.

## 파도 조절

파도 제어는 `/world/<world_name>/waves` 토픽에서 `gz.msgs.Param` 메시지를 사용합니다. `<world_name>`에는 SDF의 `<world name="...">` 값을 넣습니다. 토픽의 월드 이름은 지형 데이터 공급자가 아니라 실행 중인 월드를 식별합니다. 실행 중인 시뮬레이션의 토픽 목록은 다음 명령으로 확인합니다.

```bash
gz topic -l
```

위 [WAM-V 실행 파일](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py)의 기본 월드 이름은 `posim_surface`이므로 파도 토픽은 `/world/posim_surface/waves`입니다. 앱에서 사용하는 Gazebo 네임스페이스에 맞게 `world_name` 실행 인자를 지정할 수 있습니다. 환경 설정을 마친 수상 로봇 컨테이너 안에서 다음과 같이 실행합니다.

```bash
SURFACE_WORLD=harbour
ros2 launch /opt/posim-surface/wamv.launch.py headless:=false world_name:="$SURFACE_WORLD"
```

생성되는 SDF, 로봇 생성 요청, IMU 브리지와 시작 대기 명령이 모두 이 이름을 사용합니다. 위 예제의 파도 토픽은 `/world/harbour/waves`이고, ROS IMU 토픽은 `/model/wamv/imu`로 유지됩니다. 이 인자는 생성할 월드의 이름을 지정하며 지형 파일을 선택하지 않습니다.

이 예제는 z = −10 m의 평평한 해저면을 사용합니다. 직접 지형을 준비하려면 [높이맵 지형](heightmaps.md)을 참고하세요.

각 설정값의 자료형은 `double`입니다.

| 설정 | 단위 | 의미 |
| --- | --- | --- |
| `wind_speed` | m/s | 파도 모델의 풍속 |
| `wind_angle` | 도 | 파도 모델의 풍향 |
| `steepness` | 무차원 | 파형 경사도 |

화면과 유체역학 설정을 일치시키세요. 스펙트럼 파도 모델이 수면 움직임을 만들며 [해류](currents.md)는 별도로 설정합니다. 파도 라이브러리를 설치한 뒤에도 사용할 월드에 해당 시스템을 포함하고 설정해야 합니다.

## 사용 시 제한 사항

선택형 Dockerfile은 ARM64 기본 이미지 구조를 지원합니다. 선체·추진·저항 매개변수는 사용하는 선박과 운항 조건에 맞게 보정해야 합니다. 종료 시 프로세스 관리 도구로 MAVROS와 Gazebo를 강제 종료해야 할 수 있습니다. 새 세션을 시작하기 전에 두 프로세스가 모두 종료되었는지 확인하세요.

## 의존성 관리

`extras/surface/dependency.json`에서 사용할 커밋을 지정합니다. 도우미는 업스트림 소스를 받아 `/opt/waves`에 라이브러리를 설치하고 `/opt/asv_wave_sim`에 소스를 보관합니다. Git 서브모듈이 아니라 빌드 의존성입니다.

자세한 빌드는 [수상 로봇 통합 소스 안내](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)를, 구성 요소의 출처는 [라이선스 안내](citation.md)를 참고하세요.
