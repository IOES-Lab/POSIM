# 시스템 요구 사항

운영체제, ROS와 Gazebo 버전을 함께 맞춰 사용하세요. 현재 POSIM 빌드 구성은 Ubuntu 26.04, ROS 2 Lyrical, Gazebo Jetty를 기준으로 합니다.

## 실행 환경 선택

| 환경 | 설치 방법 | 렌더링 |
| --- | --- | --- |
| Ubuntu 직접 설치 | [소스 설치](install.md) | 호스트 그래픽 드라이버, GUI를 위한 디스플레이 |
| Linux AMD64 컨테이너 | AMD64 Docker 구성 | 소프트웨어 렌더링 또는 별도로 연결한 NVIDIA GPU |
| Linux ARM64 컨테이너 | ARM64 Docker 구성 | 컨테이너 내부의 렌더러 |
| macOS의 Docker | ARM64 또는 AMD64 이미지의 Linux 가상 머신 | 컨테이너 렌더러 사용. 호스트 Metal GPU가 Linux 렌더러로 연결되지는 않음 |

Ubuntu 직접 설치 명령은 Bash에서 실행합니다. Docker 명령은 호스트에서, ROS·Gazebo 명령은 선택한 컨테이너 안에서 실행합니다. 가능하면 호스트와 같은 CPU 아키텍처의 이미지를 사용하세요.

## 센서와 그래픽

Headless 서버에는 GUI 창이 없지만 카메라, 깊이 센서와 DVL 처리에도 렌더링 환경이 필요할 수 있습니다. 첫 데이터 수신 확인에는 소프트웨어 렌더링을 사용할 수 있습니다. 영상을 많이 처리하는 실험에는 하드웨어 렌더러를 권장합니다.

멀티빔 소나는 NVIDIA GPU, 호환되는 CUDA 툴킷과 빌드된 소나 라이브러리가 필요합니다. [소나 빌드와 성능](sonar-tuning.md)을 참고하세요. 나머지 패키지의 빌드 성공과 CUDA 사용 조건은 각각 확인해야 합니다.

## 실험용 WGPU 백엔드

WGPU 통합은 [PR #6](https://github.com/IOES-Lab/POSIM/pull/6)에서 별도로 진행 중입니다. 이 문서 검토 시점(2026-10-07)에는 `main`에 포함되지 않았으며, 여기의 설치·Quickstart 명령으로 해당 후보 버전을 설치하거나 검증하는 것은 아닙니다.

macOS의 Metal과 호환 환경의 Vulkan은 각 GPU·런타임 설정과 Gazebo 통합이 필요합니다. Apple Silicon 호스트의 Linux 컨테이너에서 네이티브 Metal을 바로 사용할 수는 없습니다. 셰이더 출력 비교, 소나 전체 실행, CUDA와의 수치 동등성은 각각 확인해야 합니다.

## 실험에 맞는 자원 준비

지형 복잡도, 영상 해상도, 센서 주기, 로봇 수와 충돌 형상에 따라 CPU·메모리·GPU 사용량이 달라집니다. 로봇 한 대와 기본 센서 설정으로 시작하세요. 메모리, GPU 사용률과 Gazebo의 실시간 비율을 측정한 뒤 부하를 늘립니다. 라이브러리에서 GPU 한 대당 고정된 동시 실행 수를 정하지는 않습니다.

소스, 빌드·설치 작업 공간, Docker 이미지, Gazebo Fuel 자산과 ROS bag 기록을 저장할 디스크 공간도 필요합니다. 소스 설치 과정에서는 외부 파도 의존성도 빌드합니다.

## 다음 단계

ROS·Gazebo 패키지 저장소, GitHub와 Gazebo Fuel에 접근할 수 있는지 확인하세요. 일부 예제 자산은 첫 실행 때 내려받습니다. [Ubuntu 설치](install.md) 또는 [Docker 환경](docker.md)을 구성한 뒤 [첫 시뮬레이션](quickstart.md)에서 확인합니다.
