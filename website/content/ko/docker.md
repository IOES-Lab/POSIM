# Docker 환경

실행하려는 소스 커밋에서 이미지를 빌드합니다. 저장소에는 Linux AMD64와 ARM64용 구성이 각각 있습니다.

## 로컬 빌드와 게시 이미지 구분

아래의 `posim:dev-*`는 로컬에서 빌드하는 태그이며 정식 배포 태그가 아닙니다. 빌드 전에 `git rev-parse HEAD`로 소스 커밋을 기록합니다. 게시된 `ioeslab/posim` 이미지를 대신 사용한다면 소스 커밋·아키텍처·패키지 이름이 문서와 맞는지 확인하고 이미지 digest를 남깁니다.

이전 `validation-pr5-*` 이미지는 과거 검증용 후보이며 정식 릴리스나 현재 카메라 수정만 남긴 PR #5의 빌드가 아닙니다. 과거 이미지의 시험 결과를 현재 `main`이나 이후 PR 커밋의 결과로 사용하면 안 됩니다. 이 가이드는 해당 이미지를 요구하지 않습니다.

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

현재 POSIM Launch의 `headless:=true`는 서버 전용 모드(`-s`)이며 Gazebo의 EGL `--headless-rendering` 모드가 아닙니다. `QT_QPA_PLATFORM=offscreen`만으로 OGRE 렌더링 환경이 준비되지는 않습니다. 카메라·깊이 센서 예제에는 작동하는 디스플레이 또는 별도로 구성한 headless 렌더러가 필요합니다.

소프트웨어 렌더링으로 첫 수신을 확인할 때는 가상 X 디스플레이를 사용할 수 있습니다. 호스트 터미널에서 일회용 컨테이너에 필요한 도구를 설치합니다.

```bash
docker exec -u root posim-quickstart bash -lc \
  'apt-get update && apt-get install -y --no-install-recommends xvfb xauth'
```

이어서 컨테이너의 터미널 A에서 가상 디스플레이를 사용하는 셸을 엽니다.

```bash
xvfb-run -a -s "-screen 0 1280x720x24" bash
```

이 셸에서 3단계 환경을 다시 불러온 뒤 카메라 예제를 실행합니다. 터미널 B에서 ROS 데이터를 확인하는 동안 이 셸을 유지하세요. 소프트웨어 렌더링용 구성이며 GPU 연결이나 CUDA·WGPU 소나 시험은 아닙니다. EGL은 [Gazebo headless 렌더링 문서](https://gazebosim.org/api/sim/10/headless_rendering.html)를 참고하세요. 현재 POSIM Launch 인자에는 해당 플래그가 노출되어 있지 않습니다.

## 그래픽, 파일 보관과 종료

NVIDIA GPU를 사용하려면 호스트 드라이버와 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)을 구성합니다. 소프트웨어 렌더링을 강제하는 대신 컨테이너 렌더러를 연결하세요. CUDA 소나는 이미지 내부에도 툴킷과 플러그인 바이너리가 필요합니다.

macOS의 Docker는 Linux 가상 머신입니다. 컨테이너 렌더링과 호스트의 직접 GPU 사용은 [시스템 요구 사항](requirements.md)에서 구분하여 확인하세요.

`--rm`은 셸 종료 후 컨테이너를 삭제합니다. ROS bag과 사용자 자산을 보관하려면 명시적으로 호스트 디렉터리를 마운트합니다. 토픽 확인 도구를 먼저 종료하고 시뮬레이션에서 Ctrl+C를 누른 뒤 컨테이너 셸을 종료합니다. Xvfb를 사용한다면 시뮬레이션이 끝나 셸 프롬프트가 돌아온 뒤 해당 셸을 닫으세요. Gazebo가 종료되는 동안에는 가상 디스플레이를 유지해야 합니다.
