# macOS Docker·RDP 설치

Apple Silicon Mac에서는 ARM64 POSIM 이미지를 실행한 뒤 원격 데스크톱(RDP)으로 컨테이너의 Linux 데스크톱에 접속합니다. Gazebo와 터미널은 그 안에서 실행합니다. Intel Mac의 에뮬레이션 실행은 이 문서에서 다루지 않습니다.

## 1. Docker와 RDP 클라이언트 설치

[Mac용 Docker Desktop](https://docs.docker.com/desktop/setup/install/mac-install/)을 설치하고 실행하세요. RDP 클라이언트로 Microsoft의 [macOS용 Windows App](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps)을 설치합니다.

macOS 터미널을 열어 다음을 확인합니다.

```bash
uname -m
docker version --format '{{.Server.Version}}'
```

Apple Silicon에서는 `arm64`가 출력됩니다. 두 번째 명령에서는 Docker 서버 버전이 나와야 합니다.

## 2. POSIM 이미지 받고 실행하기

Docker가 ARM64 이미지를 자동으로 선택합니다.

```bash
docker pull ioeslab/posim:latest
docker run -d --name posim --shm-size=1g \
  -p 127.0.0.1:3389:3389 \
  --mount type=volume,source=posim-data,target=/work \
  ioeslab/posim:latest
docker exec -u root posim chown docker:docker /work
```

RDP 포트는 이 Mac에서만 접속할 수 있습니다. 직접 만든 모델과 기록 파일은 `/work`에 저장하면 `posim-data` 볼륨에 보관됩니다. 이미지 버전, 소스 빌드, 컨테이너 교체 방법은 [Docker 환경](docker.md)을 참고하세요.

## 3. Linux 데스크톱에 접속

Windows App에서 PC 주소를 **127.0.0.1:3389**로 등록하고 연결합니다. xrdp 로그인 화면에서 **Xorg**를 선택한 뒤 사용자 이름 **docker**, 비밀번호 **docker**로 로그인하세요.

이미지에는 이 로컬 개발용 계정이 설정되어 있습니다. Linux 데스크톱에서 터미널을 열고 비밀번호를 변경하세요.

```bash
passwd
```

POSIM 명령은 Linux 데스크톱 안의 터미널에서 실행합니다. macOS 터미널은 Docker 관리에 사용하세요. [터미널 준비](terminals.md)에서 두 환경의 차이와 작업 공간 경로를 설명합니다.

## 4. 종료·재시작과 예제 선택

RDP 창만 닫아도 시뮬레이션은 계속 실행됩니다. 먼저 실행 중인 터미널에서 **Ctrl+C**로 종료하세요. 이후 macOS 터미널에서 다음을 실행할 수 있습니다.

```bash
docker stop posim
docker start posim
```

컨테이너를 다시 켠 뒤 Windows App으로 재접속합니다. 이 이미지의 Linux GUI는 소프트웨어로 렌더링하므로 복잡한 장면은 느릴 수 있습니다. CUDA 멀티빔 소나는 지원되는 NVIDIA CUDA 환경이 필요하며, 이 ARM64 이미지에는 포함되지 않습니다. 먼저 [카메라와 압력 센서 예제](features.md)를 실행해 보세요.

## 5. 데모 월드 실행

**RDP 데스크톱 안의 터미널**에서 설치를 확인하는 마지막 명령을 실행합니다.

```bash
source /opt/ros/lyrical/setup.bash
source /home/docker/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

원격 데스크톱 안의 Gazebo 창에 해양 월드가 나타나야 합니다. 실행 중에는 터미널을 열어 두세요. **Ctrl+C**로 종료한 뒤 [첫 로봇 시뮬레이션](quickstart.md)으로 진행합니다.
