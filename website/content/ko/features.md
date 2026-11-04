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

## 관련 공개 라이브러리와 도구

| 구성 요소 | 기능 | 시작 위치 |
| --- | --- | --- |
| POSIM-Terrain | 지리 지형·텍스처와 지형 유틸리티 | [라이브러리 안내](libraries.md), [공개 저장소](https://github.com/IOES-Lab/POSIM-Terrain) |
| POSIM-Routing | 전 지구·연안 경로, 항구 접근과 지역 충돌 검사 | [라이브러리 안내](libraries.md), [공개 저장소](https://github.com/IOES-Lab/POSIM-Routing) |
| POSIM-Control | 추진 정책과 네이티브 선행·추종 차량 제어기 | [라이브러리 안내](libraries.md), [공개 저장소](https://github.com/IOES-Lab/POSIM-Control) |
| macOS Ogre 수명 관리 오버레이 | 명시된 arm64 Ogre 2.3.3 구성에서 선택적으로 사용하는 빌드·실행 도구 | [지원 구성과 확인 방법](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md) |

WWW-POSIM은 선택한 공개 구성 요소 리비전에 앱 서비스와 자체 렌더링·제어 연동을 조합합니다. POSIM 체크아웃에서 직접 사용할 기능은 이 목록의 구현과 안내를 기준으로 확인하세요. 바이너리 패키지와 별도로 설치한 라이브러리는 각각 선택된 리비전이 있으므로 소스 기능을 사용하기 전에 확인합니다. 외부 코드의 라이선스 조건은 각 [구성 요소](citation.md)에 유지됩니다.
