# 지형·경로·차량 제어 라이브러리

POSIM은 해양 환경·로봇 모델·센서·ROS 2 인터페이스를 제공하는 오픈소스 해양 로봇 시뮬레이션 라이브러리입니다. 아래 관련 라이브러리로 실험에 지리 기반 지형, 경로 계획과 차량 제어를 더할 수 있습니다.

| 저장소 | 역할 | 입력과 출력 |
| --- | --- | --- |
| [POSIM-Terrain](https://github.com/IOES-Lab/POSIM-Terrain) | 육지 고도·수심·위성 텍스처 | 지리 영역 → 수치 지형·메시·SDF·자료 출처 기록 |
| [POSIM-Routing](https://github.com/IOES-Lab/POSIM-Routing) | 전역 항로·해안 경로·항구 접근·로컬 충돌 검사 | 출발지·목적지·지형 → 항로·경유점 |
| [POSIM-Control](https://github.com/IOES-Lab/POSIM-Control) | 추진·선행/추종 제어·운동 정책 | 검사한 경유점·실측 상태 → 추진 명령·제어기 상태 |

## POSIM-Terrain

위도·경도·크기로 육지와 해저 지형을 만듭니다. 사용 가능한 고도·수심 자료를 하나의 수치 지형으로 결합해 메시와 충돌 검사에 사용합니다. Sentinel-2 영상을 준비하면 육지 텍스처를 넣을 수 있습니다. 출처 기록에는 자료 범위·해시·가정이 포함됩니다.

이 공개 라이브러리는 GEBCO 수심 데이터를 지원하며, `gebco` 자료 선택 정책으로 GEBCO만 사용해 지형을 생성할 수 있습니다. POSIM은 생성된 지형을 월드에 불러올 수 있습니다. 핵심 엔진과 [평평한 해저면을 사용하는 WAM-V 예제](surface.md)는 GEBCO 데이터 없이 실행됩니다.

- [설치와 지형 생성 명령](https://github.com/IOES-Lab/POSIM-Terrain/blob/main/README.ko.md)
- [뉴욕·부산·도쿄 지형 묶음과 3D 캡처](https://github.com/IOES-Lab/POSIM-Terrain/blob/main/examples/README.ko.md)

## POSIM-Routing

세계 각 항구를 잇는 항로를 계획하고 각 항구의 도착 수역까지 연결합니다. 항구 연결에는 OSM 육지 경계를 사용합니다. 해안·로컬 계획기는 POSIM-Terrain의 수치 지형으로 경로를 검사합니다. 전역 항로만으로 수심을 확인할 수는 없습니다.

- [설치·항로 출력·계획기 API](https://github.com/IOES-Lab/POSIM-Routing/blob/main/README.ko.md)
- 지형 의존성: POSIM-Terrain

## POSIM-Control

속도·거친 파도·안정성·복구에 관한 Python 정책을 제공합니다. 네이티브 Gazebo 플러그인은 수상 선행 차량과 추종 차량에 추진력을 가합니다. 선행 차량은 지정된 경유점을 따라가고, 추종 차량은 선행 차량의 실측 위치를 따라갑니다. 호출하는 프로그램이 경로를 보내기 전에 지형을 검사합니다.

- [Python 정책·네이티브 플러그인 빌드·제어 토픽](https://github.com/IOES-Lab/POSIM-Control/blob/main/README.ko.md)
- 네이티브 빌드: Gazebo 개발 패키지를 갖춘 POSIM 실행 환경

## 함께 사용하기

POSIM-Terrain이 지리 지형을 제공합니다. POSIM-Routing은 그 지형에서 경로를 계획하고 검사합니다. POSIM-Control은 승인된 경유점을 따라가도록 제어하고, Gazebo가 물리 운동을 계산합니다.

각 라이브러리의 README를 따라 설치하고 입력과 출력을 연결하세요. [WWW-POSIM](https://www-posim.vercel.app/)은 이 라이브러리들과 POSIM을 웹 작업 공간과 설치형 앱에서 함께 사용하는 앱의 한 예입니다.

POSIM 실험은 [설치](install.md), [로봇 모델](custom-robots.md), [ROS 2 제어](ros.md)에서 시작하세요.
