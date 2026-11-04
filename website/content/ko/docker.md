# Docker 환경

Docker Hub에서 기존 이미지를 받거나 실행하려는 소스 커밋에서 이미지를 빌드할 수 있습니다. 저장소에는 Linux AMD64와 ARM64용 구성이 각각 있습니다.

## Docker Hub에서 이미지 받기

[Docker Hub의 POSIM](https://hub.docker.com/r/ioeslab/posim/tags)에는 현재 아키텍처별 검증 이미지가 있습니다. Linux AMD64 호스트에서는 AMD64 태그를 받습니다.

```bash
docker pull ioeslab/posim:validation-pr5-abad9d70-amd64
```

Apple Silicon을 포함한 ARM64 호스트에서는 RDP 이미지를 받습니다.

```bash
docker pull ioeslab/posim:validation-pr5-abad9d70-arm64-rdp
```

이 이미지들은 검증용으로 게시된 빌드이며 현재 릴리스나 `latest` 태그가 아닙니다. 두 이미지의 레이블은 소스 커밋 `32eedbac`을 가리킵니다. 해당 버전의 데모 패키지 이름과 작업 공간 경로는 현재 저장소와 다릅니다. 이 이미지에는 [해당 버전의 Docker 안내](https://github.com/IOES-Lab/POSIM/blob/32eedbacf781a0fd1e517481571e76bfaab73c51/docs/docker.md)와 [빠른 시작](https://github.com/IOES-Lab/POSIM/blob/32eedbacf781a0fd1e517481571e76bfaab73c51/README.md)을 사용하고, 실행 명령의 로컬 빌드 태그를 내려받은 이미지 태그로 바꾸세요. 아래 소스 빌드 및 컨테이너 명령은 현재 저장소를 기준으로 하므로 그대로 따라 하려면 로컬에서 빌드하세요. 새 이미지가 있는지는 [태그 목록](https://hub.docker.com/r/ioeslab/posim/tags)에서 확인하고, 환경 재현을 위해 이미지 digest를 기록합니다.

## 소스에서 빌드

`git rev-parse HEAD`로 사용할 커밋을 기록하세요. 다음 단계는 현재 체크아웃에서 `posim:dev-*` 이미지를 빌드합니다.

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

## 데스크톱 없이 영상 센서 렌더링

`headless:=true`는 Gazebo 서버 모드(`-s`)로 실행합니다. 영상 센서는 별도의 렌더링 환경이 필요합니다.

- 가상 디스플레이: 아래 Xvfb 설정 사용
- EGL: [Gazebo headless 렌더링](https://gazebosim.org/api/sim/10/headless_rendering.html) 설정과 `--headless-rendering` 필요
- `QT_QPA_PLATFORM=offscreen`: Qt 창 설정

소프트웨어 렌더링으로 첫 수신을 확인할 때는 가상 X 디스플레이를 사용할 수 있습니다. 호스트 터미널에서 일회용 컨테이너에 필요한 도구를 설치합니다.

```bash
docker exec -u root posim-quickstart bash -lc \
  'apt-get update && apt-get install -y --no-install-recommends xvfb xauth'
```

이어서 컨테이너의 터미널 A에서 가상 디스플레이를 사용하는 셸을 엽니다.

```bash
xvfb-run -a -s "-screen 0 1280x720x24" bash
```

이 셸에서 3단계 환경을 불러온 뒤 카메라 예제를 실행합니다. 터미널 B에서 데이터를 확인하는 동안 셸을 유지하세요. Xvfb 구성은 소프트웨어 렌더링을 사용합니다. EGL 실행 플래그는 POSIM Launch 인자에 노출되어 있지 않습니다.

## 그래픽, 파일 보관과 종료

NVIDIA GPU를 사용하려면 호스트 드라이버와 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)을 구성합니다. 소프트웨어 렌더링을 강제하는 대신 컨테이너 렌더러를 연결하세요. CUDA 소나는 이미지 내부에도 툴킷과 플러그인 바이너리가 필요합니다.

macOS의 Docker는 Linux 가상 머신입니다. 컨테이너 렌더링과 호스트의 직접 GPU 사용은 [시스템 요구 사항](requirements.md)에서 구분하여 확인하세요.

`--rm`은 셸 종료 후 컨테이너를 삭제합니다. 기록과 자산을 보관하려면 호스트 디렉터리를 마운트하세요.

1. 토픽 확인 도구를 종료합니다.
2. 시뮬레이션에서 Ctrl+C를 누르고 종료를 기다립니다.
3. Xvfb 셸과 컨테이너 셸을 닫습니다.

Gazebo가 종료될 때까지 가상 디스플레이를 유지하세요.

Launch 로그에 Gazebo가 SIGINT 후 종료되지 않아 SIGTERM을 받았다고 표시되면 강제 종료가 있었더라도 Launch 명령 자체는 성공으로 끝날 수 있습니다. 호스트에서 `docker ps --filter name=posim-quickstart`로 컨테이너가 남아 있는지 확인하세요. 더 이상 사용하지 않는데 실행 중이라면 `docker stop posim-quickstart`로 종료합니다. `--rm`을 지정했으므로 종료 후 컨테이너는 삭제됩니다. 반복되는 종료 오류는 무인 실행에 사용하기 전에 원인을 확인하세요.
