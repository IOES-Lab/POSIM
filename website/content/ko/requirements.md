# 시스템 요구 사항

운영체제·ROS·Gazebo 버전을 맞춰 사용하세요.

- **Ubuntu 26.04**
- **ROS 2 Lyrical**
- **Gazebo Jetty**

## 실행 환경 선택

| 환경 | 설치 방법 | 렌더링 |
| --- | --- | --- |
| Ubuntu 직접 설치 | [소스 설치](install.md) | 호스트 그래픽 드라이버, GUI를 위한 디스플레이 |
| Linux AMD64 컨테이너 | AMD64 Docker 구성 | 소프트웨어 렌더링 또는 별도로 연결한 NVIDIA GPU |
| Linux ARM64 컨테이너 | ARM64 Docker 구성 | 컨테이너 내부의 렌더러 |
| macOS의 Docker | ARM64 또는 AMD64 이미지의 Linux 가상 머신 | 컨테이너 렌더러 사용. 호스트 Metal GPU가 Linux 렌더러로 연결되지는 않음 |

Ubuntu 직접 설치 명령은 Bash에서 실행합니다. Docker 명령은 호스트에서, ROS·Gazebo 명령은 선택한 컨테이너 안에서 실행합니다. 가능하면 호스트와 같은 CPU 아키텍처의 이미지를 사용하세요.

## 센서와 그래픽

카메라·깊이 센서·DVL에는 렌더링 환경이 필요합니다. 화면 없이 실행할 때도 렌더러를 준비하세요. 첫 데이터 확인에는 소프트웨어 렌더링을 사용할 수 있습니다.

멀티빔 소나에는 NVIDIA GPU·CUDA 툴킷·소나 라이브러리가 필요합니다. [소나 빌드와 성능](sonar-tuning.md)을 참고하세요.

## 실험에 맞는 자원 준비

로봇 한 대와 기본 센서로 시작하고 부하를 측정한 뒤 규모를 늘리세요.

- CPU·메모리·GPU 사용량
- Gazebo 실시간 비율
- 지형·충돌 형상 복잡도
- 카메라 해상도와 센서 주기
- 동시 로봇 수

소스, 빌드·설치 작업 공간, Docker 이미지, Gazebo Fuel 자산과 ROS bag 기록을 저장할 디스크 공간도 필요합니다. 소스 설치 과정에서는 외부 파도 의존성도 빌드합니다.

## 다음 단계

ROS·Gazebo 패키지 저장소, GitHub와 Gazebo Fuel에 접근할 수 있는지 확인하세요. 일부 예제 자산은 첫 실행 때 내려받습니다. [Ubuntu 설치](install.md) 또는 [Docker 환경](docker.md)을 구성한 뒤 [첫 시뮬레이션](quickstart.md)에서 확인합니다.
