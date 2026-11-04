# 기능과 예제

공개 시뮬레이션 기능의 구현 위치와 사용 안내를 찾을 수 있습니다. POSIM은 Gazebo 시스템, 로봇 모델과 ROS 인터페이스를 제공합니다. 월드 SDF와 Launch 설정에서 시뮬레이션에 사용할 시스템을 선택합니다.

## 환경과 렌더링

| 기능 | 공개 구현 | 사용 안내 |
| --- | --- | --- |
| 수중 흐림, RGB 흡수와 햇빛 감쇠 | `gazebo/posim_gz_rendering` | [장면 효과·기본값·실시간 제어](underwater-rendering.md) |
| 해저 햇빛 무늬, 부유 입자·마린 스노 | `gazebo/posim_gz_rendering` | [수중 효과 활성화와 확인](underwater-rendering.md) |
| 해저 텍스처, 측정 격자와 수심 등고선 | `gazebo/posim_gz_rendering`, 지형에서 제공하는 맵 | [간격과 재질 설정](underwater-rendering.md) |
| 스펙트럼 파도와 파도를 반영한 WAM-V 유체역학 | 외부 Wave Sim, `extras/surface` 연동 | [수상 로봇과 파도](surface.md) |
| 균일 해류, 수심별 해류와 조석 입력 | `posim_gz_world_plugins`, `posim_gz_model_plugins`, `posim_ros_gz_plugins` | [해류 입력과 ROS 서비스](currents.md) |
| 지리 원점과 좌표 변환 | `posim_ros_gz_plugins` | [위도·경도 좌표 서비스](coordinates.md) |
| 해저·물체·작업 월드 | `models/posim_worlds`, `posim_object_models` | [월드](worlds.md), [물체](objects.md), [높이맵](heightmaps.md) |

수중 광학 효과는 PBR 표면에 적용되며 수치 깊이를 유지합니다. 해수면의 사용자 재질은 자체 렌더링을 사용합니다. 마린 스노는 절차적 시각 근사이며 기본값은 꺼짐입니다. 해수면 기준과 카메라 동작은 [수중 장면 안내](underwater-rendering.md)를 참고하세요.

## 센서·로봇·제어

| 기능 | 공개 구현 | 사용 안내 |
| --- | --- | --- |
| 해양 GNSS, 침수 시 수신 상실, 재수신과 지연 | `gazebo/posim_gz_gnss` | [수신기 설정과 측정 형식](gnss.md) |
| RGBD 기반 수중 영상 감쇠 | `posim_gz_sensor_plugins/UnderwaterCamera` | [카메라 예제와 설정](camera.md) |
| DVL 측정과 ROS 변환 | Gazebo DVL, `posim_ros_gz_plugins` | [빔 설정과 출력](dvl.md) |
| 정수압과 압력 기반 깊이 | `posim_gz_sensor_plugins` | [수압 설정과 단위](pressure.md) |
| USBL 음향 요청·응답과 위치 추정 | `posim_gz_sensor_plugins` | [송수신기·응답기 예제](usbl.md) |
| CUDA 멀티빔 소나 | `gazebo/posim_gz_multibeam_sonar` | [소나 모델](sonar.md), [빌드·조절](sonar-tuning.md) |
| REXROV·BlueROV2 계열·Slocum 글라이더 | `models/posim_robot_models` | [ROV](rovs.md), [글라이더](gliders.md), [사용자 로봇](custom-robots.md) |
| ArduRover WAM-V 연동과 ROS 제어 | `extras/surface`, 모델 Launch 설정, `posim_interfaces` | [수상 차량](surface.md), [ROS 2](ros.md) |

사용 가능한 센서는 설치된 플러그인과 모델 설정에 따라 달라집니다. GNSS 프로필은 장착 정보를 제공하며, 호출 코드가 링크와 수신기를 연결해야 합니다. CUDA 소나에는 지원 GPU·개발 도구가 필요합니다. 토픽 이름의 설정만으로 측정 수신을 확인할 수는 없습니다.

## 소스 디렉토리

아래 경로는 POSIM 소스 저장소를 기준으로 합니다.

| 위치 | 제공하는 내용 |
|---|---|
| `models/posim_worlds` | 월드 파일과 환경 리소스 |
| `models/posim_robot_models` | 기체 역학, 센서와 로봇별 ROS 설정 |
| `models/posim_sensor_models` | 독립 센서 모델과 예제 |
| `gazebo/` | 해양 환경·센서 플러그인 |
| `posim_interfaces` | POSIM 메시지·서비스 정의 |
| `examples/posim_demos` | 월드·모델·ROS를 연결하는 Launch |

[첫 시뮬레이션](quickstart.md)에서 이 구성 요소를 함께 실행합니다. 모델이나 환경을 바꾸려면 [로봇 추가](custom-robots.md)와 [월드 만들기](custom-worlds.md)를 사용하세요.

## macOS Ogre 수명 관리 오버레이

[Ogre 오버레이 도구](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md)는 명시된 macOS arm64·Ogre-Next 2.3.3 환경에서 사용할 독립 Ogre 코어를 빌드합니다. 이 구성을 사용할 때는 해당 문서의 SDK 조건과 로드된 라이브러리 확인 절차를 따르세요.

외부 코드의 라이선스 조건은 각 [구성 요소](citation.md)에 유지됩니다.
