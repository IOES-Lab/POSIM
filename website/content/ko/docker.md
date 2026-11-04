# Docker 환경

POSIM 이미지는 Ubuntu, ROS 2 Lyrical, Gazebo Jetty, POSIM, ArduSub, MAVROS와 QGroundControl을 포함합니다. 화면 연결과 드라이버 설정은 [실행 환경 선택](requirements.md)에서 운영 체제별 설치 가이드를 고르세요.

## 공개 이미지 내려받기

[Docker Hub의 POSIM 이미지](https://hub.docker.com/r/ioeslab/posim/tags)를 사용합니다. Docker가 컴퓨터에 맞는 AMD64 또는 ARM64 이미지를 선택합니다.

```bash
docker pull ioeslab/posim:latest
export POSIM_IMAGE=ioeslab/posim:latest
```

| 태그 | 용도 |
|---|---|
| `latest` | 공개된 main 빌드; AMD64·ARM64 제공 |
| `sha-<커밋-앞-12자리>` | 특정 소스 리비전 선택; Docker Hub에 실제로 있는 태그를 복사 |
| `sha-<커밋-앞-12자리>-amd64` / `-arm64` | 아키텍처까지 지정 |
| 배포된 릴리스 버전 | 해당 버전의 이미지 선택 |

이미지 라벨에는 전체 소스 리비전이 기록됩니다. 같은 이미지로 실험을 재현하려면 다이제스트도 기록하세요.

```bash
docker image inspect "$POSIM_IMAGE" \
  --format '{{index .Config.Labels "org.opencontainers.image.revision"}}'
docker image inspect "$POSIM_IMAGE" --format '{{json .RepoDigests}}'
```

선택한 리비전에 맞는 문서를 사용하세요. 새 이미지를 내려받아도 기존 컨테이너가 바뀌지는 않습니다. 새 이미지를 사용하려면 작업을 저장하고 컨테이너를 교체합니다.

## 소스에서 빌드

직접 수정한 소스를 사용할 때는 실행할 체크아웃을 빌드합니다. 호스트에서 실행하세요.

```bash
git clone https://github.com/IOES-Lab/POSIM.git
cd POSIM
git rev-parse HEAD
```

CPU에 맞는 빌드 명령 하나를 선택합니다.

```bash
docker build --platform linux/amd64 \
  -f .docker/lyrical.amd64.dockerfile -t posim:dev-amd64 .
```

```bash
docker build --platform linux/arm64 \
  -f .docker/lyrical.arm64v8.dockerfile -t posim:dev-arm64-rdp .
```

운영 체제별 설치 가이드의 `ioeslab/posim:latest` 대신 빌드한 로컬 태그를 사용하세요. 두 이미지는 전달한 소스와 고정된 Wave Sim 의존성을 빌드합니다. ARM64 이미지에는 XFCE 데스크톱과 RDP 서버도 들어 있습니다.

## 운영 체제별로 시작하기

- [Linux Docker 설치](install-ubuntu-docker.md): Linux 데스크톱의 화면 연결을 사용합니다.
- [Windows WSL·Docker 설치](install-windows-docker.md): WSLg로 Linux 창을 표시합니다.
- [macOS Docker·RDP 설치](install-macos.md): 이미지 안의 Linux 데스크톱에 접속합니다.

각 가이드에서는 `posim`이라는 컨테이너를 만듭니다. 명령을 실행할 위치와 작업 공간 설정은 [터미널 준비](terminals.md)에서 설명합니다.

## 작업 공간과 파일 보관

AMD64 이미지는 `/opt/posim_ws`, ARM64 이미지는 `/home/docker/posim_ws`를 사용합니다. 이미지의 `POSIM_WS` 또는 `POSIM_UNDERLAY` 변수에 해당 경로가 들어 있습니다.

운영 체제별 가이드는 Docker 볼륨 `posim-data`를 `/work`에 연결합니다. 독립 SDF 파일과 기록 데이터는 여기에 저장하세요. 그 밖의 컨테이너 내부 파일은 정지·재시작할 때 유지되지만 컨테이너를 삭제하면 함께 사라집니다. 수정한 소스는 교체 전에 Git에 커밋하거나 밖으로 복사하세요. 볼륨은 별도로 삭제하기 전까지 컨테이너와 독립적으로 유지됩니다.

## 정지·재시작·업데이트

실행 중인 터미널에서 Ctrl+C로 시뮬레이션을 종료한 뒤 호스트에서 Docker 명령을 실행합니다.

```bash
docker stop posim
docker start posim
```

업데이트할 때는 작업을 저장하고 컨테이너를 정지한 뒤 선택한 이미지를 내려받습니다. 정지된 컨테이너를 제거하고 운영 체제별 가이드의 생성 명령을 다시 실행하세요. `posim-data`를 재사용하면 `/work`의 파일을 유지할 수 있습니다.

## 그래픽과 GPU 사용 {#graphics-and-gpu-access}

Linux에서 GPU를 사용하려면 호스트 드라이버와 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)을 설정합니다. Windows는 [Docker Desktop의 WSL 2 GPU 설정](https://docs.docker.com/desktop/features/gpu/)을 따릅니다. CUDA 소나는 이미지 안에도 필요한 라이브러리가 있어야 합니다. [소나 설정과 실험](sonar-tuning.md)을 참고하세요. 기본 화면 연결 예제는 소프트웨어 렌더링을 사용하므로 GPU를 설정한 환경보다 느릴 수 있습니다.
