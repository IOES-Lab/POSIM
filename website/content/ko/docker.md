# Docker 환경

실행하려는 소스 커밋에서 이미지를 빌드합니다. 저장소에는 Linux AMD64와 ARM64용 구성이 각각 있습니다.

## 1. 호스트에서 이미지 빌드

아직 소스가 없다면 먼저 내려받습니다.

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
```

AMD64에서는 다음 명령을 사용합니다.

```bash
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
```

Apple Silicon Docker 호스트를 포함한 ARM64에서는 다음 명령을 사용합니다.

```bash
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

두 구성 모두 지정된 외부 파도 의존성을 설치합니다. ARM64 구성에는 RDP 데스크톱도 포함되지만 아래 서버 전용 예제를 실행할 때 RDP 연결은 필요하지 않습니다.

## 2. Headless 컨테이너 열기

빌드한 이미지를 지정합니다.

```bash
export POSIM_IMAGE=posim:dev-arm64-rdp
docker run --rm -it --init --name posim-quickstart --shm-size=1g \
  --entrypoint bash -e LIBGL_ALWAYS_SOFTWARE=1 \
  -e QT_QPA_PLATFORM=offscreen -e GZ_IP=127.0.0.1 "$POSIM_IMAGE"
```

AMD64에서는 `POSIM_IMAGE`를 `posim:dev-amd64`로 바꿉니다. 이 구성은 소프트웨어 렌더링을 사용하며 호스트 포트를 열지 않습니다.

## 3. 컨테이너 안에서 환경 불러오기

```bash
source /opt/ros/lyrical/setup.bash
export POSIM_WS="${POSIM_WS:-${POSIM_UNDERLAY:-}}"
source "$POSIM_WS/install/setup.bash"
ros2 pkg prefix posim_demos
```

AMD64 작업 공간은 `/opt/posim_ws`, ARM64 작업 공간은 `/home/docker/posim_ws`입니다. 위 환경변수로 해당 이미지의 작업 공간을 선택합니다.

## 4. 두 번째 터미널에서 확인

첫 번째 컨테이너 셸을 열어둔 상태에서 다른 호스트 터미널을 엽니다.

```bash
docker exec -it posim-quickstart bash
```

두 번째 컨테이너 셸에서도 환경을 불러온 뒤 [첫 시뮬레이션](quickstart.md)의 확인 명령을 실행합니다. 두 셸은 같은 컨테이너와 통신 설정을 사용해야 합니다.

## 그래픽, 파일 보관과 종료

NVIDIA GPU를 사용하려면 호스트 드라이버와 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)을 구성합니다. 소프트웨어 렌더링을 강제하는 대신 컨테이너 렌더러를 연결하세요. CUDA 소나는 이미지 내부에도 툴킷과 플러그인 바이너리가 필요합니다.

macOS의 Docker는 Linux 가상 머신입니다. 컨테이너 렌더링과 호스트의 직접 GPU 사용은 [시스템 요구 사항](requirements.md)에서 구분하여 확인하세요.

`--rm`은 셸 종료 후 컨테이너를 삭제합니다. ROS bag과 사용자 자산을 보관하려면 명시적으로 호스트 디렉터리를 마운트합니다. 토픽 확인 도구를 먼저 종료하고 시뮬레이션에서 Ctrl+C를 누른 뒤 컨테이너 셸을 종료합니다.
