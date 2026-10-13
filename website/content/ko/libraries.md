# 지형·경로·차량 제어 라이브러리

POSIM은 해양 환경·로봇 모델·센서·ROS 2 인터페이스를 제공하는 시뮬레이션 엔진입니다. [WWW-POSIM](https://www-posim.vercel.app/)은 엔진과 세 공개 라이브러리를 연결해 지리 기반 월드와 자동 항해를 제공합니다.

| 저장소 | 역할 | 입력과 출력 |
| --- | --- | --- |
| [POSIM-Terrain](https://github.com/IOES-Lab/POSIM-Terrain) | 육지 고도·수심·위성 텍스처 | 지리 영역 → 수치 지형·메시·SDF·자료 출처 기록 |
| [POSIM-Routing](https://github.com/IOES-Lab/POSIM-Routing) | 전역 항로·해안 경로·항구 접근·로컬 충돌 검사 | 출발지·목적지·지형 → 항로·경유점 |
| [POSIM-Control](https://github.com/IOES-Lab/POSIM-Control) | 추진·선행/추종 제어·운동 정책 | 검사한 경유점·실측 상태 → 추진 명령·제어기 상태 |

## POSIM-Terrain

위도·경도·크기로 육지와 해저 지형을 만듭니다. 사용 가능한 고도·수심 자료를 하나의 수치 지형으로 결합해 메시와 충돌 검사에 사용합니다. Sentinel-2 영상을 준비하면 육지 텍스처를 넣을 수 있습니다. 출처 기록에는 자료 범위·해시·가정이 포함됩니다.

- [설치와 지형 생성 명령](https://github.com/IOES-Lab/POSIM-Terrain/blob/main/README.ko.md)
- [뉴욕·부산·도쿄 지형 묶음과 3D 캡처](https://github.com/IOES-Lab/POSIM-Terrain/blob/main/examples/README.ko.md)

## POSIM-Routing

세계 해상 항로를 계획하고 도착 수역까지 연결합니다. 항구 연결에는 OSM 육지 경계를 사용합니다. 해안·로컬 계획기는 POSIM-Terrain의 수치 지형으로 경로를 검사합니다. 전역 항로만으로 수심을 확인할 수는 없습니다.

- [설치·항로 출력·계획기 API](https://github.com/IOES-Lab/POSIM-Routing/blob/main/README.ko.md)
- 지형 의존성: POSIM-Terrain

## POSIM-Control

속도·거친 파도·안정성·복구에 관한 Python 정책을 제공합니다. 네이티브 Gazebo 플러그인은 수상 선행 차량과 추종 차량에 추진력을 가합니다. 선행 차량은 지정된 경유점을 따라가고, 추종 차량은 선행 차량의 실측 위치를 따라갑니다. 호출하는 프로그램이 경로를 보내기 전에 지형을 검사합니다.

- [Python 정책·네이티브 플러그인 빌드·제어 토픽](https://github.com/IOES-Lab/POSIM-Control/blob/main/README.ko.md)
- 네이티브 빌드: Gazebo 개발 패키지를 갖춘 POSIM 실행 환경

## 함께 사용하기

POSIM-Terrain이 지리 지형을 제공합니다. POSIM-Routing은 그 지형에서 경로를 계획하고 검사합니다. POSIM-Control은 승인된 경유점을 따라가도록 제어하고, Gazebo가 물리 운동을 계산합니다. WWW-POSIM은 세션·지형 작업·항해 진행·사용자 화면을 관리합니다.

각 라이브러리는 README의 방법으로 직접 사용할 수 있습니다. WWW-POSIM은 세 라이브러리와 POSIM 엔진의 커밋을 Git 서브모듈로 지정합니다. 웹 작업공간과 설치형 앱에도 지정된 구현을 포함합니다. 앱의 구조는 [WWW-POSIM 구성 요소](https://www-posim.vercel.app/guide/components.ko.html)를 참고하세요.

엔진 사용은 [설치](install.md), [로봇 모델](custom-robots.md), [ROS 2 제어](ros.md)에서 시작하세요.
