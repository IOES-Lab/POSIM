# Windows WSL·Docker 설치

Docker Desktop에서 준비된 POSIM 이미지를 실행하고 WSLg로 Gazebo 창을 표시합니다. 이 페이지의 Docker 명령은 PowerShell이 아니라 **WSL의 Ubuntu 터미널**에서 실행하세요.

## 1. WSL과 그래픽 드라이버 준비

[Windows WSL 설치](install-windows.md)의 처음 두 단계를 완료하세요. WSL2에 Ubuntu 26.04를 설치한 뒤 WSL과 **Windows용** 그래픽 드라이버를 업데이트합니다. NVIDIA 그래픽카드도 마찬가지입니다. WSL 안에 Linux용 디스플레이 드라이버를 설치하지 마세요.

[WSL2 백엔드를 사용하는 Docker Desktop](https://docs.docker.com/desktop/features/wsl/)을 설치합니다. Docker Desktop에서 **Settings → Resources → WSL Integration → Ubuntu-26.04**를 켜세요. Docker Desktop의 엔진을 사용하므로 Ubuntu 안에 Docker Engine을 따로 설치할 필요는 없습니다.

Ubuntu 터미널에서 서버와 화면 연결 소켓을 확인합니다.

```bash
docker version --format '{{.Server.Version}}'
test -S /mnt/wslg/.X11-unix/X0
printf '%s\n' "$DISPLAY"
```

소켓이 없다면 먼저 [WSLg 설정](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)을 완료하세요.

## 2. POSIM 이미지 받기

```bash
docker pull ioeslab/posim:latest
```

이미지 버전과 소스 빌드는 [Docker 환경](docker.md)에서 설명합니다. 아래 기본 설정은 소프트웨어 렌더링을 사용합니다. 지원되는 NVIDIA 하드웨어로 GPU가 필요한 예제를 실행하려면 먼저 [Docker Desktop의 GPU 요구 사항](docker.md#graphics-and-gpu-access)을 확인하세요.

## 3. 컨테이너 만들기

```bash
docker run -d --name posim --init --shm-size=1g \
  -e DISPLAY -e QT_QPA_PLATFORM=xcb -e LIBGL_ALWAYS_SOFTWARE=1 \
  --mount type=bind,source=/mnt/wslg/.X11-unix,target=/tmp/.X11-unix,readonly \
  --mount type=volume,source=posim-data,target=/work \
  --entrypoint bash ioeslab/posim:latest -lc 'exec sleep infinity'
```

직접 만든 파일은 `/work`에 저장하면 `posim-data` 볼륨에 보관됩니다. 컨테이너를 멈추거나 다시 켤 때는 `docker stop posim`, `docker start posim`을 사용합니다.

이후 예제에서는 [Ubuntu 터미널 두 개를 열고](terminals.md) 둘 다 같은 컨테이너에 접속하세요. Gazebo는 Windows 데스크톱에 창으로 표시되므로 별도의 원격 데스크톱은 필요하지 않습니다.

## 4. 데모 월드 실행

컨테이너에 접속합니다.

```bash
docker exec -it posim bash
```

컨테이너 안에서 설치를 확인하는 마지막 명령을 실행합니다.

```bash
source /opt/ros/lyrical/setup.bash
source /opt/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

Gazebo 창에 해양 월드가 나타나야 합니다. 실행 중에는 터미널을 열어 두고, **Ctrl+C**로 종료하세요. 다음은 [첫 로봇 실행](quickstart.md)입니다.
