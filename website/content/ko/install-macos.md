# macOS 설치

POSIM을 Linux 컨테이너에서 실행하고 RDP(원격 데스크톱 프로토콜)로 화면에 접속합니다. 데스크톱 이미지는 ARM64용이며 Apple Silicon Mac을 대상으로 합니다. Intel Mac에서는 ARM64 에뮬레이션이 필요하며 성능과 호환성은 Docker의 에뮬레이션 지원에 따라 달라집니다.

## 1. Docker와 원격 데스크톱 클라이언트 준비

[Docker Desktop for Mac](https://docs.docker.com/desktop/setup/install/mac-install/)을 설치하고 실행합니다. 직접 PC 연결을 지원하는 RDP 클라이언트도 설치합니다. 예를 들어 [macOS용 Windows App](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps)을 사용할 수 있습니다. 이미지 빌드와 시뮬레이션에 충분한 Docker 디스크 공간과 메모리를 할당하세요.

## 2. 데스크톱 이미지 빌드

macOS 터미널에서 저장소를 복제하고 ARM64 이미지를 빌드합니다.

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

ROS, Gazebo와 여러 의존성을 내려받으므로 빌드에 시간이 오래 걸릴 수 있습니다. 현재 체크아웃한 POSIM 리비전으로 빌드합니다. 화면 없는 실행과 AMD64 대안은 [Docker 환경](docker.md)을 참고하세요.

## 3. 데스크톱 시작과 접속

데스크톱 컨테이너를 시작합니다. RDP 포트는 이 Mac의 루프백 인터페이스에만 공개합니다.

```bash
docker run -d --name posim-rdp --shm-size=1g \
  -p 127.0.0.1:3389:3389 posim:dev-arm64-rdp
```

RDP 클라이언트에서 `127.0.0.1:3389`로 PC를 추가합니다. xrdp 로그인 화면에서 **Xorg**를 선택하고 이미지의 기본 사용자 이름 `docker`, 비밀번호 `docker`로 로그인합니다. macOS 계정이 아닌 컨테이너 계정입니다. 이 계정에는 비밀번호 없는 sudo 권한이 있으므로 포트를 루프백에만 바인딩하고 데스크톱 터미널에서 `passwd`로 비밀번호를 변경하세요. 3389 포트를 이미 사용 중이면 컨테이너 생성 시 `-p 127.0.0.1:3390:3389`를 사용하고 `127.0.0.1:3390`으로 접속합니다.

## 4. POSIM 실행

원격 데스크톱에서 터미널을 엽니다. 이미지의 `docker` 사용자 셸에는 ROS와 POSIM 작업 공간이 설정되어 있습니다. Gazebo 창과 함께 월드를 시작합니다.

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

첫 실행에서 Gazebo Fuel 모델을 내려받을 수 있습니다. 데스크톱을 닫기 전에 Ctrl+C로 실행을 중지하세요. 로봇과 센서 예제는 [첫 시뮬레이션](quickstart.md)을 참고하고, 해당 인수를 지원하는 로봇·센서 예제에서 `gui:=true headless:=false`를 사용하세요.

## 중지하고 다시 접속

macOS 터미널에서 `docker stop posim-rdp`로 컨테이너를 중지합니다. 나중에 `docker start posim-rdp`로 시작하고 다시 접속합니다. 컨테이너 내부 파일은 중지·재시작해도 유지됩니다. 컨테이너를 삭제하거나 교체할 때 보존할 작업은 별도로 마운트한 디렉터리나 볼륨에 저장하세요.

컨테이너는 Linux 렌더러를 사용하며 macOS Metal을 Gazebo에 직접 연결하지 않습니다. CUDA 소나에는 호환되는 NVIDIA/CUDA 환경이 필요합니다. [소나 설정](sonar-tuning.md)과 [문제 해결](troubleshooting.md)을 참고하세요.
