# Platform for Ocean Simulation

POSIM은 ROS 2와 Gazebo용 오픈소스 해양 로봇 시뮬레이션 라이브러리입니다. 작업 공간에서 해양 환경, 수상·수중 로봇, 모의 센서와 제어 실험을 직접 실행할 수 있습니다. POSIM은 [Project DAVE](https://field-robotics-lab.github.io/dave.doc/)를 계승하며, DAVE는 [UUV Simulator](https://uuvsimulator.github.io/)를 기반으로 합니다.

<figure class="docs-abstract"><div class="docs-abstract-grid"><a href="rovs.html" aria-label="ROV · BlueROV2"><img width="577" height="797" src="{{ASSET_PREFIX}}media/overview/bluerov2.png" alt="ROV · BlueROV2" decoding="async"><span>ROV · BlueROV2</span></a><a href="surface.html" aria-label="수상 로봇 · WAM-V"><img width="1280" height="720" src="{{ASSET_PREFIX}}media/overview/wamv.jpg" alt="수상 로봇 · WAM-V" decoding="async"><span>수상 로봇 · WAM-V</span></a><a href="objects.html" aria-label="해저 · 작업 장면"><img width="2200" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-6eec9419.png" alt="해저 · 작업 장면" decoding="async"><span>해저 · 작업 장면</span></a><a href="sonar.html" aria-label="소나 · 영상과 점군"><img width="1738" height="1066" src="{{ASSET_PREFIX}}media/notion/sonar-f4ac9419.png" alt="소나 · 영상과 점군" decoding="async"><span>소나 · 영상과 점군</span></a><a href="dvl.html" aria-label="DVL · 속도 관측"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="DVL · 속도 관측" decoding="async"><span>DVL · 속도 관측</span></a><a href="camera.html" aria-label="카메라 · 수중 시각"><img width="2202" height="1650" src="{{ASSET_PREFIX}}media/notion/camera-f14c9419.png" alt="카메라 · 수중 시각" decoding="async"><span>카메라 · 수중 시각</span></a></div><figcaption>해양 환경 → 로봇과 동역학 → 센서 관측 → ROS 2 제어. POSIM Notion Wiki의 센서 그림과 WWW-POSIM에서 실행한 POSIM 로봇 장면입니다. 각 그림을 누르면 사용 안내로 이동합니다.</figcaption></figure>

## 여기서 시작하세요

<div class="docs-architecture"><a href="requirements.html"><strong>설치</strong><span>Ubuntu, macOS 또는 Windows 환경을 선택합니다.</span></a><a href="quickstart.html"><strong>실행</strong><span>해양 월드를 띄우고 센서 데이터를 받습니다.</span></a><a href="custom-robots.html"><strong>확장</strong><span>로봇, 지형과 제어 코드를 추가합니다.</span></a></div>

설치 안내를 선택하세요: [Ubuntu](install.md)(권장), [Docker와 RDP를 사용하는 macOS](install-macos.md), [WSL 2를 사용하는 Windows](install-windows.md). 일반적인 [Docker 안내](docker.md)도 있습니다. 환경을 준비한 뒤 [첫 시뮬레이션](quickstart.md)을 실행하세요.

- 운영체제: **Ubuntu 26.04**
- ROS: **ROS 2 Lyrical**
- 시뮬레이터: **Gazebo Jetty**

## 라이브러리 구성

[공개 기능 목록](features.md)에서 환경 효과·센서·로봇 연동·관련 라이브러리의 소스와 사용 안내를 찾을 수 있습니다.

| 구성 요소 | 용도 | 안내 |
| --- | --- | --- |
| 해양 월드 | 수면·해저·작업 장면 | [월드 목록](worlds.md) |
| 로봇 | REXROV, BlueROV2, Slocum, 수상 로봇 | [ROV](rovs.md), [글라이더](gliders.md), [수상 로봇](surface.md) |
| 수중 렌더링 | 흐림·흡수·햇빛·해저 햇빛 무늬·마린 스노·해저 오버레이 | [장면 효과와 제어](underwater-rendering.md) |
| 센서 | 카메라·해양 GNSS·DVL·수압·USBL·CUDA 소나 | [카메라](camera.md), [GNSS](gnss.md), [DVL](dvl.md), [소나](sonar.md) |
| 환경 플러그인 | 해류·위도·경도 좌표 | [해류](currents.md), [좌표](coordinates.md) |
| ROS 인터페이스 | 센서 구독·제어·실험 기록 | [ROS 2와 제어](ros.md) |

## 시뮬레이션의 구성

1. 월드 SDF에 환경과 월드 시스템을 정의합니다.
2. 모델 SDF에 링크·충돌·관성·센서를 정의합니다.
3. Launch 파일로 Gazebo와 모델, ROS 브리지를 실행합니다.
4. ROS 노드에서 센서를 구독하고 제어 명령을 보냅니다.

## 필요한 문서 찾기

| 다음에 할 작업 | 문서 |
|---|---|
| 시뮬레이션 하나를 실행하고 종료하기 | [첫 시뮬레이션](quickstart.md) |
| ROS 연결 이해하기 | [ROS 터미널과 토픽](ros.md) |
| 제어 프로그램 작성하기 | [Python으로 제어하기](python-control.md), 이어서 [추진기와 Autopilot](robot-control.md) |
| 측정값을 표시하거나 저장하기 | [센서 데이터 보기와 기록](visualization.md) |
| 실험 조건 바꾸기 | [월드 만들기](custom-worlds.md), [로봇 추가](custom-robots.md), [센서 추가와 설정](custom-sensors.md) |
| 플러그인의 정확한 설정 찾기 | **플러그인 참고** 그룹: 빠른 시작, SDF, 파라미터와 동작 설명 |

각 페이지 하단에서 관련 소스와 참고 자료를 확인할 수 있습니다.

## 관련 프로젝트

[구성 요소와 라이브러리](libraries.md)에서 POSIM 패키지와 관련 지형·경로·차량 제어 프로젝트를 설명합니다. 각 관련 라이브러리에는 별도의 사용 안내가 있습니다.

[WWW-POSIM](https://www-posim.vercel.app/)은 POSIM으로 만든 웹·설치형 앱입니다. 자신의 ROS 작업 공간에서 POSIM을 사용하려면 이 문서의 설치와 시뮬레이션 안내를 따라가세요.
