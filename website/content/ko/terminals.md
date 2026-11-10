# 터미널 준비

**터미널**은 명령을 입력하고 실행 결과를 읽는 창입니다. 이 문서에서는 일반 터미널 창 두 개를 **터미널 A**, **터미널 B**라고 부릅니다. A는 시뮬레이션을 실행하고, B는 센서 데이터를 읽거나 제어 명령을 보냅니다. 서로 다른 프로그램 이름이 아니라 역할을 구분하기 위한 이름입니다.

## 설치 방법에 맞는 터미널 열기

| 설치 방법 | 터미널 A와 B를 여는 위치 |
|---|---|
| [Ubuntu 네이티브](install.md) | Ubuntu의 터미널 앱에서 창 두 개 열기 |
| [Linux Docker](install-ubuntu-docker.md) | Linux 터미널 두 개를 열고, 각 창에서 아래 명령으로 같은 컨테이너에 접속 |
| [Windows WSL 네이티브](install-windows.md) | Windows Terminal에서 같은 WSL 배포판의 Ubuntu 탭 두 개 열기 |
| [Windows WSL·Docker](install-windows-docker.md) | Ubuntu WSL 탭 두 개를 열고, 각각 같은 Docker 컨테이너에 접속 |
| [macOS Docker·RDP](install-macos.md) | RDP로 Linux 데스크톱에 접속한 뒤 그 안에서 터미널 창 두 개 열기 |

Linux·Windows Docker 설치 가이드에서 만든 컨테이너 이름은 `posim`입니다. 호스트 터미널 두 개에서 각각 다음 명령을 실행해 접속하세요.

```bash
docker exec -it posim bash
```

macOS의 RDP 데스크톱은 이미 컨테이너 안에 있습니다. 그 데스크톱의 Linux 터미널에서 ROS 명령을 실행하세요. PowerShell과 macOS 터미널은 Docker 컨테이너 시작 같은 호스트 작업에 사용합니다.

## 작업 공간 이해하기

**ROS 작업 공간**은 소스 코드와 빌드한 패키지를 모아 두는 디렉토리입니다. `src/`에는 소스 패키지, `build/`에는 빌드 중간 파일, `install/`에는 ROS가 불러올 패키지가 들어 있습니다.

| 설치 방법 | 작업 공간 경로 |
|---|---|
| Ubuntu·WSL 네이티브 | `~/posim_ws` |
| AMD64 Docker 이미지 | `/opt/posim_ws` |
| ARM64 RDP 이미지 | `/home/docker/posim_ws` |

경로의 `~`는 현재 Linux 사용자의 홈 디렉토리를 뜻합니다. 아래의 `POSIM_WS`는 작업 공간 경로를 저장하는 변수입니다. 이 변수를 사용하면 이후 명령을 각 설치 환경에서 같은 형태로 실행할 수 있습니다.

## 두 터미널에서 환경 불러오기

**Ubuntu·WSL 네이티브 설치**에서는 A와 B 각각에서 실행합니다.

```bash
export POSIM_WS="$HOME/posim_ws"
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source "$POSIM_WS/install/setup.bash"
cd "$POSIM_WS"
```

**Docker 설치**에서는 RDP로 접속한 Linux 데스크톱을 포함해 A와 B 각각에서 실행합니다.

```bash
export POSIM_WS="${POSIM_WS:-${POSIM_UNDERLAY:-/home/docker/posim_ws}}"
source /opt/ros/lyrical/setup.bash
source "$POSIM_WS/install/setup.bash"
cd "$POSIM_WS"
```

`source`는 해당 터미널이 ROS와 POSIM을 찾을 수 있도록 경로와 설정을 불러오는 명령입니다. 새 터미널을 열 때마다 반복하세요. Docker 이미지는 작업 공간 변수와 ArduSub 실행 경로를 제공합니다.

## 실행 전 확인

```bash
ros2 pkg prefix posim_demos
ros2 pkg prefix ros_gz_sim
```

두 경로 모두 작업 공간의 `install/` 아래를 가리켜야 합니다. 패키지를 찾지 못하면 설치 가이드를 다시 확인하세요. `ros_gz_sim`이 `/opt/ros/lyrical` 아래를 가리키면 작업 공간 설정을 불러오지 않은 상태입니다.

이제 [첫 시뮬레이션](quickstart.md)을 진행하세요. 명령이 계속 결과를 출력하고 있다면 실행 중인 것입니다. **그 터미널에서 Ctrl+C**를 누르고 명령 입력 대기 상태로 돌아온 뒤 다음 명령을 실행합니다.
