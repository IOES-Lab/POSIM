# 실행 환경 선택

현재 사용하는 운영 체제에 맞춰 설치 방법을 고르세요. **Ubuntu 26.04 네이티브 설치를 가장 권장합니다.** Gazebo 그래픽, ROS 도구, NVIDIA·CUDA 장치를 직접 사용할 수 있습니다.

## 운영 체제별 설치 경로

| 운영 체제 | 설치 방법 | 설치 가이드 |
|---|---|---|
| Ubuntu 26.04 | Ubuntu에 POSIM을 직접 빌드하고 실행 | [Ubuntu 네이티브 설치](install.md) |
| Ubuntu 또는 Docker를 지원하는 다른 Linux, x86-64 | Ubuntu 기반 POSIM 이미지를 Docker로 실행하고 Linux 바탕 화면에 Gazebo 표시 | [Linux Docker 설치](install-ubuntu-docker.md) |
| Windows 10/11, x86-64 | WSL 2에 Ubuntu 26.04를 설치한 뒤 POSIM을 직접 빌드; WSLg로 Linux 창 표시 | [Windows WSL 설치](install-windows.md) |
| Windows 10/11, x86-64 | WSL 2 기반 Docker Desktop에서 POSIM 이미지 실행; WSLg로 창 표시 | [Windows WSL·Docker 설치](install-windows-docker.md) |
| Apple silicon Mac | ARM64 Docker 이미지를 실행하고 RDP로 Linux 데스크톱에 접속 | [macOS Docker·RDP 설치](install-macos.md) |

Docker 이미지는 자체 Ubuntu 환경을 포함하므로 Linux 호스트와 이미지의 Ubuntu 버전이 같을 필요는 없습니다. 호스트 운영 체제는 Docker의 지원 조건을 확인하세요. macOS 경로는 Apple silicon을 기준으로 하며, Intel Mac의 ARM64 에뮬레이션은 이 안내에 포함하지 않습니다.

## 시뮬레이션 환경의 소프트웨어

모든 경로에서 **ROS 2 Lyrical**과 **Gazebo Sim — Gazebo Jetty**를 사용합니다. 네이티브 설치는 Ubuntu에 이들을 준비하고, Docker 이미지는 이들을 포함합니다. 호스트 그래픽 드라이버는 선택한 설치 가이드에서 안내합니다.

## 그래픽과 센서

Gazebo 창과 카메라·깊이 센서를 사용하려면 렌더러가 정상 작동해야 합니다. 먼저 로봇 한 대와 기본 센서로 시작하세요. CUDA 멀티빔 소나에는 호환 NVIDIA GPU, CUDA 도구와 소나 라이브러리가 추가로 필요합니다. [소나 설정과 실험](sonar-tuning.md)을 참고하세요. macOS의 Docker는 Apple Metal을 Linux의 NVIDIA·CUDA 장치로 제공하지 않습니다.

## 자원과 다운로드

필요한 자원은 로봇 수, 지형 세부 수준, 영상 크기와 센서 갱신 빈도에 따라 달라집니다. 설정을 늘리기 전에 메모리·GPU 부하와 Gazebo의 실시간 비율을 확인하세요. 소스·빌드 파일, Docker 이미지, Fuel 리소스, 녹화 데이터가 차지할 디스크 공간도 확보합니다.

설치 중에는 ROS·Gazebo 패키지 저장소, GitHub, Docker Hub에 접속해야 합니다. 일부 월드는 처음 실행할 때 Fuel 모델을 내려받습니다. 설치 가이드의 데모가 작동하면 [터미널 준비](terminals.md)로 이어가세요.
