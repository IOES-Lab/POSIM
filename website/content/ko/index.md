# Platform for Ocean Simulation

POSIM은 해양 로봇 실험을 위한 ROS 2·Gazebo 라이브러리입니다. 해양 환경, 로봇 모델, 모의 센서와 제어 인터페이스로 반복 가능한 실험을 구성합니다.

<figure class="docs-abstract"><div class="docs-abstract-grid"><a href="rovs.html" aria-label="ROV · BlueROV2"><img width="577" height="797" src="{{ASSET_PREFIX}}media/overview/bluerov2.png" alt="ROV · BlueROV2" decoding="async"><span>ROV · BlueROV2</span></a><a href="surface.html" aria-label="수상 로봇 · WAM-V"><img width="1280" height="720" src="{{ASSET_PREFIX}}media/overview/wamv.jpg" alt="수상 로봇 · WAM-V" decoding="async"><span>수상 로봇 · WAM-V</span></a><a href="objects.html" aria-label="해저 · 작업 장면"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="해저 · 작업 장면" decoding="async"><span>해저 · 작업 장면</span></a><a href="sonar.html" aria-label="소나 · 영상과 점군"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="소나 · 영상과 점군" decoding="async"><span>소나 · 영상과 점군</span></a><a href="dvl.html" aria-label="DVL · 속도 관측"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL · 속도 관측" decoding="async"><span>DVL · 속도 관측</span></a><a href="camera.html" aria-label="카메라 · 수중 시각"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="카메라 · 수중 시각" decoding="async"><span>카메라 · 수중 시각</span></a></div><figcaption>해양 환경 → 로봇과 동역학 → 센서 관측 → ROS 2 제어. POSIM Notion Wiki의 센서 그림과 WWW-POSIM에서 실행한 POSIM 로봇 장면입니다. 각 그림을 누르면 사용 안내로 이동합니다.</figcaption></figure>

## WWW-POSIM 체험하기

[WWW-POSIM](https://www-posim.vercel.app/)은 POSIM 엔진을 사용하는 해양 로봇 시뮬레이터입니다. 웹 작업공간이나 설치형 앱에서 사용할 수 있습니다.

- 위도·경도로 해저와 해안 지형 생성
- 수상·수중 로봇 운항과 센서 확인
- ArduPilot 경유점 임무 또는 ROS 2 코드 시험
- 공개 LIVE 세계 항해 관람

## 여기서 시작하세요

<div class="docs-architecture"><a href="install.html"><strong>설치</strong><span>Ubuntu에 ROS 2·Gazebo 작업 공간을 구성합니다.</span></a><a href="quickstart.html"><strong>실행</strong><span>해양 월드를 띄우고 센서 데이터를 받습니다.</span></a><a href="custom-robots.html"><strong>확장</strong><span>로봇, 지형과 제어 코드를 추가합니다.</span></a></div>

[Ubuntu 설치](install.md) 또는 [Docker 환경](docker.md)을 준비한 뒤 [첫 시뮬레이션](quickstart.md)을 실행하세요.

- 운영체제: **Ubuntu 26.04**
- ROS: **ROS 2 Lyrical**
- 시뮬레이터: **Gazebo Jetty**

## 라이브러리 구성

| 구성 요소 | 용도 | 안내 |
| --- | --- | --- |
| 해양 월드 | 수면·해저·작업 장면 | [월드 목록](worlds.md) |
| 로봇 | REXROV, BlueROV2, Slocum, 수상 로봇 | [ROV](rovs.md), [글라이더](gliders.md), [수상 로봇](surface.md) |
| 센서 | 카메라·DVL·수압·USBL·CUDA 소나 | [카메라](camera.md), [DVL](dvl.md), [소나](sonar.md) |
| 환경 플러그인 | 해류·위도·경도 좌표 | [해류](currents.md), [좌표](coordinates.md) |
| ROS 인터페이스 | 센서 구독·제어·실험 기록 | [ROS 2와 제어](ros.md) |

## 시뮬레이션의 구성

1. 월드 SDF에 환경과 월드 시스템을 정의합니다.
2. 모델 SDF에 링크·충돌·관성·센서를 정의합니다.
3. Launch 파일로 Gazebo와 모델, ROS 브리지를 실행합니다.
4. ROS 노드에서 센서를 구독하고 제어 명령을 보냅니다.

POSIM은 ROS 작업 공간에서 직접 사용합니다. WWW-POSIM은 지리 좌표 기반 월드 생성, 작업 화면과 온라인 세션을 제공합니다.

WWW-POSIM은 지형 생성·경로 계획·차량 제어에 [POSIM-Terrain·POSIM-Routing·POSIM-Control](libraries.md)을 사용합니다. 각 공개 라이브러리에 별도 사용 안내가 있습니다.

## 필요한 문서 찾기

- **예제:** 실행 명령과 데이터 확인
- **상세 가이드:** 모델·지형 추가와 빌드 설정
- **플러그인 참고:** 설정값·단위·토픽
- **[기여하기](contributing.md):** 소스·문서 수정과 확인 방법

각 페이지 하단에서 관련 소스와 참고 자료를 확인할 수 있습니다.
