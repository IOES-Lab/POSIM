# Linux Docker 설치

Ubuntu 버전이 다르거나 다른 Linux 배포판을 사용한다면 배포된 POSIM 이미지를 이용할 수 있습니다. 이 문서는 **x86-64 데스크톱에서 AMD64 이미지**를 실행하고, X11 또는 XWayland로 Gazebo 창을 표시하는 방법을 다룹니다. Ubuntu 26.04에서는 [네이티브 설치](install.md)를 가장 권장합니다.

## 1. Docker와 데스크톱 준비

사용 중인 배포판에 맞게 [Docker Engine](https://docs.docker.com/engine/install/)을 설치하세요. Ubuntu라면 [Ubuntu용 설치 안내](https://docs.docker.com/engine/install/ubuntu/)를 따릅니다. 현재 계정으로 `docker`를 실행할 수 없다면 Docker의 [Linux 설치 후 설정](https://docs.docker.com/engine/install/linux-postinstall/)도 완료하세요.

데스크톱에서 터미널을 열고 배포판의 패키지 관리자로 `xauth`를 설치합니다. Ubuntu에서는 다음과 같습니다.

```bash
sudo apt update
sudo apt install -y xauth
```

아래 명령에서 디스플레이 주소와 Docker 서버 버전이 출력되어야 합니다.

```bash
printf '%s\n' "$DISPLAY"
docker version --format '{{.Server.Version}}'
```

## 2. POSIM 이미지 받기

```bash
docker pull ioeslab/posim:latest
```

소스 리비전 태그, 이미지 확인, 직접 빌드하는 방법은 [Docker 환경](docker.md)에 있습니다. 아래 명령은 소프트웨어 렌더링을 사용하므로 처음 실행할 때 GPU 제조사별 컨테이너 설정이 필요하지 않습니다. 렌더링 속도는 네이티브 설치보다 느릴 수 있습니다.

## 3. 컨테이너의 화면 연결 허용

현재 데스크톱 세션의 X11 인증 정보를 본인만 읽을 수 있는 파일에 복사합니다. 다른 사용자에게 X 서버를 개방하지 않고 컨테이너에서 화면에 연결할 수 있습니다.

```bash
mkdir -p "$HOME/.cache/posim"
export POSIM_XAUTH="$HOME/.cache/posim/xauthority"
touch "$POSIM_XAUTH"
chmod 600 "$POSIM_XAUTH"
xauth nlist "$DISPLAY" | sed -e 's/^..../ffff/' | xauth -f "$POSIM_XAUTH" nmerge -
test -s "$POSIM_XAUTH"
```

마지막 명령이 실패하면 현재 로그인한 데스크톱에서 터미널을 열었는지, XWayland를 사용할 수 있는지 확인하세요. 인증 파일이 비어 있는 상태로 다음 단계로 진행하지 마세요.

## 4. 컨테이너 만들기

```bash
docker run -d --name posim --init --shm-size=1g \
  -e DISPLAY -e QT_QPA_PLATFORM=xcb -e LIBGL_ALWAYS_SOFTWARE=1 \
  -e XAUTHORITY=/tmp/posim.xauth \
  --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix,readonly \
  --mount "type=bind,source=$POSIM_XAUTH,target=/tmp/posim.xauth,readonly" \
  --mount type=volume,source=posim-data,target=/work \
  --entrypoint bash ioeslab/posim:latest -lc 'exec sleep infinity'
```

직접 만든 모델과 기록 파일은 `/work`에 저장하세요. 이 경로의 이름 있는 볼륨은 컨테이너를 교체해도 유지됩니다. 컨테이너를 멈추거나 다시 켤 때는 `docker stop posim`, `docker start posim`을 사용합니다. 데스크톱에 다시 로그인한 뒤 화면 연결이 안 되면 인증 파일을 갱신하세요.

이후 예제에서는 같은 컨테이너에 연결된 [터미널 두 개를 준비](terminals.md)합니다. 하드웨어 가속이나 CUDA 소나를 사용하려면 먼저 [GPU 설정](docker.md#graphics-and-gpu-access)을 확인하세요.

## 5. 데모 월드 실행

컨테이너에 접속합니다.

```bash
docker exec -it posim bash
```

접속한 터미널에서 설치를 확인하는 마지막 명령을 실행합니다.

```bash
source /opt/ros/lyrical/setup.bash
source /opt/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```

Gazebo 창에 해양 월드가 나타나야 합니다. 실행 중에는 터미널을 열어 두세요. **Ctrl+C**로 종료한 뒤 [첫 로봇 시뮬레이션](quickstart.md)으로 진행합니다.
