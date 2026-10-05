# Platform for Ocean Simulation

POSIM은 해양 로봇 실험을 위한 ROS 2·Gazebo 라이브러리입니다. 해양 환경, 로봇 모델, 모의 센서와 제어 인터페이스를 조합하여 같은 조건의 실험을 반복할 수 있습니다.

## 여기서 시작하세요

<div class="docs-architecture"><a href="install.html"><strong>설치</strong><span>Ubuntu에 ROS 2·Gazebo 작업 공간을 구성합니다.</span></a><a href="quickstart.html"><strong>실행</strong><span>해양 월드를 띄우고 첫 센서 데이터를 받습니다.</span></a><a href="custom-robots.html"><strong>확장</strong><span>자신의 로봇, 지형과 제어 코드를 추가합니다.</span></a></div>

Linux에 직접 설치하려면 [Ubuntu 설치](install.md)를, 독립된 실행 환경을 만들려면 [Docker 환경](docker.md)을 따라 진행하세요. 설치 후에는 [첫 시뮬레이션](quickstart.md)으로 이어집니다. 현재 소스의 기준 환경은 **Ubuntu 26.04, ROS 2 Lyrical, Gazebo Jetty**입니다.

## 라이브러리에 무엇이 있나요?

| 구성 요소 | 용도 | 안내 |
| --- | --- | --- |
| 해양 월드 | 수면, 해저 지형과 작업 장면 구성 | [월드 목록](worlds.md) |
| 로봇 | REXROV, BlueROV2, Slocum과 수상 로봇 통합 | [ROV](rovs.md), [글라이더](gliders.md), [수상 로봇](surface.md) |
| 센서 | 수중 카메라, DVL, 수압, USBL, CUDA 멀티빔 소나 | [카메라](camera.md), [DVL](dvl.md), [소나](sonar.md) |
| 환경 플러그인 | 해류와 위도·경도 좌표 서비스 | [해류](currents.md), [좌표](coordinates.md) |
| ROS 인터페이스 | 센서 구독, 로봇 제어와 실험 기록 | [ROS 2와 제어](ros.md) |

## 시뮬레이션의 구성

월드 SDF에는 환경과 월드 시스템을 정의합니다. 모델 SDF에는 링크, 충돌 형상, 관성, 센서와 모델 시스템을 정의합니다. Launch 파일이 Gazebo를 시작하고 선택한 모델을 배치한 뒤 ROS 브리지를 연결합니다. 사용자의 ROS 노드는 관측값을 받아 설정된 인터페이스로 제어 명령을 보냅니다.

POSIM은 시뮬레이션 라이브러리입니다. 계정 관리, 온라인 세션과 WWW-POSIM 웹 플랫폼은 이 라이브러리를 사용하는 별도 응용 프로그램입니다. ROS 작업 공간에서 라이브러리를 직접 사용할 때는 해당 서비스가 필요하지 않습니다.

## 필요한 문서 찾기

예제에는 바로 실행할 수 있는 명령을 모았습니다. 상세 가이드에서는 모델·지형 추가와 빌드 설정을 다룹니다. 플러그인 참고 문서에는 설정값, 단위와 토픽 이름을 정리했습니다. 처음에는 예제를 하나씩 실행하여 Launch와 리소스 구성을 익혀보세요.

이 문서는 IOES-Lab의 POSIM Notion Wiki를 현재 소스 구조에 맞게 정리한 것입니다. 각 페이지 하단에서 참고 자료와 기준 소스를 확인할 수 있습니다. 코드와 문서를 함께 갱신하는 방법은 [기여하기](contributing.md)에 있습니다.
