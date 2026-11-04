# Windows 설치

Windows Subsystem for Linux 2(WSL 2)의 Ubuntu 26.04에서 POSIM을 실행합니다. Ubuntu 안내의 명령은 PowerShell이 아닌 Linux 배포판 안에서 실행합니다.

## 1. WSL과 Ubuntu 설치

관리자 권한으로 연 PowerShell에서 WSL과 Ubuntu 26.04를 설치합니다.

```powershell
wsl --install -d Ubuntu-26.04
```

요청하면 Windows를 다시 시작합니다. Ubuntu를 열고 Linux 사용자 이름과 비밀번호를 만듭니다. WSL이 이미 설치되어 있다면 다른 배포판 설치와 문제 해결을 다루는 [Microsoft WSL 설치 안내](https://learn.microsoft.com/en-us/windows/wsl/install)를 참고하세요. PowerShell에서 설치된 배포판과 WSL 버전을 확인합니다.

```powershell
wsl --list --verbose
```

Ubuntu 배포판에 버전 2가 표시되어야 합니다. Ubuntu 터미널에서 배포판 버전을 확인합니다.

```bash
cat /etc/os-release
```

## 2. Ubuntu 안에 POSIM 설치

Ubuntu 터미널에서 [Ubuntu 설치](install.md)의 **소스 가져오기**부터 **빌드와 환경 불러오기**까지 진행합니다. 작업 공간은 Linux 홈 디렉터리에 둡니다. 설치 스크립트는 `sudo`를 사용하고 패키지 저장소를 설정하며 배포판 안에서 전체 패키지 업그레이드를 실행하므로 먼저 내용을 확인하세요.

## 3. 시뮬레이션 실행과 화면 확인

각 Ubuntu 터미널에서 Ubuntu 안내대로 환경을 불러온 뒤 [첫 시뮬레이션](quickstart.md)을 실행합니다. Gazebo 창에는 WSLg와 호환 그래픽 드라이버가 필요합니다. Microsoft의 [Linux GUI 앱 설정](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)을 참고하세요. 월드 실행에서는 `headless:=false`, 로봇·센서 실행에서는 `gui:=true headless:=false`를 설정합니다. 실행을 중지할 때까지 터미널을 열어 둡니다.

WSL 렌더링과 DDS 네트워크 동작은 Windows와 GPU 설정에 따라 달라집니다. 영상 토픽은 보이지만 프레임이 멈추면 선택적으로 [대용량 영상 전송 설정](troubleshooting.md#영상-토픽은-보이지만-수신이-멈출-때)을 적용하세요. 멀티빔 소나에는 [소나 설정](sonar-tuning.md)에 설명된 호환 NVIDIA/CUDA 환경도 필요합니다.

Ubuntu에 POSIM 스택을 직접 설치하는 대신 컨테이너를 사용하려면 [Docker 환경](docker.md)을 참고하세요.
