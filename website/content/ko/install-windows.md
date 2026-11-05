# Windows WSL 설치

Windows Subsystem for Linux 2(WSL2)의 Ubuntu 26.04에 POSIM을 직접 설치합니다. Gazebo 창은 WSLg를 통해 Windows 바탕 화면에 표시됩니다. 준비된 이미지를 사용하려면 [Windows WSL·Docker 설치](install-windows-docker.md)를 선택하세요.

## 1. WSL과 Ubuntu 설치

관리자 권한으로 PowerShell을 열고 실행합니다.

```powershell
wsl --install -d Ubuntu-26.04
```

안내에 따라 Windows를 재시작한 뒤 Ubuntu를 열어 Linux 사용자 이름과 암호를 만드세요. PowerShell에서 Ubuntu 배포판의 버전이 2인지 확인합니다.

```powershell
wsl --list --verbose
```

WSL이 이미 설치되어 있거나 설치가 실패하면 [Microsoft WSL 설치 안내](https://learn.microsoft.com/en-us/windows/wsl/install)를 참고하세요.

## 2. 그래픽 드라이버와 WSLg 준비

사용하는 Intel·AMD·NVIDIA GPU에 맞는 **Windows용 그래픽 드라이버**를 설치하거나 업데이트합니다. NVIDIA GPU도 이 단계가 필요합니다. WSLg는 Windows 드라이버를 통해 Linux 그래픽을 가속합니다. [Microsoft GUI 앱 설정](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)을 따르세요.

NVIDIA는 [CUDA on WSL 안내](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)에 따라 Windows 드라이버를 설치합니다. WSL 안에 Linux 디스플레이 드라이버를 설치하지 마세요. 소나 플러그인을 빌드할 때 필요한 CUDA 툴킷은 별도로 준비합니다.

PowerShell에서 WSL을 업데이트합니다.

```powershell
wsl --update
```

재시작 안내가 나오면 실행 중인 WSL 터미널의 작업을 먼저 저장하세요. 업데이트를 마친 뒤 Ubuntu를 다시 엽니다.

## 3. Ubuntu 터미널에서 POSIM 설치

Linux 명령은 Windows Terminal의 **Ubuntu** 프로필에서 실행합니다. 배포판 정보를 확인하세요.

```bash
cat /etc/os-release
```

[Ubuntu 네이티브 설치](install.md)의 소스 다운로드부터 의존성 설치와 빌드까지 진행합니다. 작업 공간은 Linux 홈 디렉토리의 `~/posim_ws`에 만드세요. 설치 도구는 `sudo`를 사용하고 Ubuntu 내부의 패키지를 업데이트합니다.

영상 토픽은 보이지만 프레임이 멈추면 [큰 영상 전송 문제 해결](troubleshooting.md#영상-토픽은-보이지만-수신이-멈출-때)을 참고하세요. 이후 실습에서 사용할 Ubuntu 터미널 두 개는 [터미널 준비](terminals.md)에서 설명합니다.

## 4. 데모 월드 실행

Ubuntu 터미널에서 실행합니다. Windows 바탕 화면에 Gazebo 창이 열리고 해양 장면이 나타나야 합니다. 처음에는 Fuel 모델을 내려받을 수 있습니다. 다음 예제를 시작하기 전에 Ctrl+C로 종료하세요.

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```
