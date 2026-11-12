# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![POSIM Docker 이미지 빌드·배포](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM은 [한국해양대학교(KMOU)의 IOES Lab.](https://lab.wschoi.com)에서 관리하는 오픈소스 해양 로보틱스 시뮬레이션 라이브러리입니다.
해양 환경, 수중·수상 로봇, 센서 모델과 ROS 2 제어 인터페이스를 Gazebo Sim에서 함께 사용할 수 있습니다.

## 문서와 시작하기

**[한국어 문서 읽기](https://ioes-lab.github.io/POSIM/ko/)**

[변경 이력](CHANGELOG.md)

POSIM은 ROS 2 Lyrical과 Gazebo Sim Jetty를 사용합니다. 자신의 운영 체제에 맞는 설치 방법을 선택하세요.

- **Ubuntu 26.04:** 가장 권장하는 [네이티브 설치](https://ioes-lab.github.io/POSIM/ko/install.html). 다른 Linux 환경에서는 [Docker 설치](https://ioes-lab.github.io/POSIM/ko/install-ubuntu-docker.html)도 선택할 수 있습니다.
- **Windows:** [WSL 설치](https://ioes-lab.github.io/POSIM/ko/install-windows.html) 또는 [WSL + Docker](https://ioes-lab.github.io/POSIM/ko/install-windows-docker.html).
- **macOS:** [Docker + RDP](https://ioes-lab.github.io/POSIM/ko/install-macos.html).

[실행 환경 선택](https://ioes-lab.github.io/POSIM/ko/requirements.html)에서 방법을 비교한 뒤,
[터미널을 준비](https://ioes-lab.github.io/POSIM/ko/terminals.html)하고
[첫 시뮬레이션](https://ioes-lab.github.io/POSIM/ko/quickstart.html)을 실행하세요.

[기능과 예제](https://ioes-lab.github.io/POSIM/ko/features.html)에서 로봇, 월드, 센서와 제어 예제의 안내를 찾을 수 있습니다.
[수중 장면 효과와 부유 입자](https://ioes-lab.github.io/POSIM/ko/underwater-rendering.html),
[해양 GNSS](https://ioes-lab.github.io/POSIM/ko/gnss.html)도 포함합니다.

## Docker 이미지

현재 이미지를 다운로드합니다.

```bash
docker pull ioeslab/posim:latest
```

Docker가 호스트에 맞는 AMD64 또는 ARM64 이미지를 선택합니다. 같은 버전으로 실험을 재현하려면
[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags)에서 커밋 ID 앞 12자리를 사용하는
`sha-<12-character-commit>` 태그를 선택하세요.
컨테이너를 만들기 전에 [실행 환경 선택](https://ioes-lab.github.io/POSIM/ko/requirements.html)을 따라
Linux, Windows WSL 또는 macOS의 화면 연결을 준비합니다.

## 관련 애플리케이션

[WWW-POSIM](https://www-posim.vercel.app/)은 POSIM에 지리 지형, 경로 계획, 로봇 제어 라이브러리를 결합하고
GUI와 웹·설치형 앱을 더한 애플리케이션입니다. 앱의 사용 방법은 WWW-POSIM 문서에서 안내합니다.

## 기여하기

[IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM)에서 오류를 신고하거나 변경을 제안할 수 있습니다.
개발 시 확인할 항목과 문서 갱신 방법은 [기여 안내](https://ioes-lab.github.io/POSIM/ko/contributing.html)를 참고하세요.

## 라이선스와 출처

POSIM에는 [Project DAVE](https://github.com/Field-Robotics-Lab/dave)의 코드가 포함되며, 원저자와 저작권 표시를 유지합니다.
POSIM에서 작성한 코드는 [Apache License 2.0](LICENSE)을 사용합니다. 외부 코드와 리소스에는 각각의 원래 조건이 적용됩니다.
외부 Wave Sim은 GPL 조건에 따라 원본 저장소에서 빌드합니다.
자세한 내용은 [Wave Sim 라이선스·배포 안내](extras/surface/README.md#licensing)를 참고하세요.
