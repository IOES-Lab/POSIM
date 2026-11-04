# 문제 해결

Launch 로그의 첫 오류부터 살펴보세요. 환경 경로, 모델 리소스, 시뮬레이션 시간, 플러그인, 데이터 수신을 차례로 확인하면 원인을 좁힐 수 있습니다. 전체 로그도 함께 보관하세요.

## 패키지를 찾지 못할 때

```bash
printf 'ROS_DISTRO=%s\n' "$ROS_DISTRO"
ros2 pkg prefix posim_demos
```

`lyrical`과 선택한 작업 공간 경로가 나와야 합니다. [설치](install.md) 또는 [Docker](docker.md)에 따라 환경을 불러오세요. 호스트와 컨테이너의 경로를 확인합니다.

## 모델·메시가 없을 때

로그의 첫 누락 URI를 확인합니다. 해당 패키지 리소스가 설치되었는지, 맞는 작업 공간을 불러왔는지 확인하세요. Fuel 참조는 Gazebo 실행 환경에서 네트워크 접근이 필요합니다. 첫 다운로드가 진행 중이라면 상태를 확인하고 기다린 뒤 다음 Launch를 시작합니다.

## 영상이 없거나 렌더링 초기화가 실패할 때

영상 센서는 화면 없이 실행해도 렌더러가 필요합니다. 디스플레이·offscreen 설정을 확인하세요. [Docker 환경](docker.md)의 Xvfb 설정이나 [CUDA 라이브러리](sonar-tuning.md)를 확인합니다.

[수중 장면 효과](underwater-rendering.md)는 `/posim/appearance/<session_nonce>/state` 서비스, 렌더러·셰이더 경로와 활성 카메라 수를 확인합니다. 부유 입자의 기본값은 꺼짐이므로 직접 켜세요. 렌더링 후 적용 리비전과 준비 상태를 확인합니다. 카메라 감쇠 예제만 실행하면 장면 효과 시스템은 불러오지 않습니다.

명시된 **macOS arm64 / Ogre-Next 2.3.3** 구성에는 하나의 프로세스에서 별도 Ogre 코어를 빌드·선택하는 [선택형 수명 관리 오버레이 도구](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md)가 있습니다. 정확한 SDK·소스 검사를 따르고 실제 라이브러리 로드 경로를 확인하세요. 표준 설치에서 자동으로 켜지지 않으며, 다른 Ogre 버전이나 플랫폼에는 각각 맞는 구성이 필요합니다.

## GNSS 위치가 유효하지 않을 때

[GNSS 유효 상태](gnss.md), 안테나 월드 높이, 해당 위치의 수면 높이와 취득 시각을 읽습니다. 월드의 위도·경도 좌표, 차단되지 않은 수면 위 안테나와 재수신·지연에 필요한 시뮬레이션 시간 진행을 확인합니다. 파도 토픽을 지정했다면 호환되는 최신 수면 높이 표본을 확인하세요. 수신기를 설치하는 것만으로 모델 연결이나 ROS·autopilot 어댑터가 구성되지는 않습니다.

## 메시지가 오지 않을 때

시간이 진행되는지 먼저 확인한 뒤 토픽과 자료형을 찾습니다.

```bash
gz topic -l
ros2 topic list -t
ros2 topic info /underwater_camera/simulated_image --verbose
```

맞는 통신 도구를 사용하고 같은 환경에서 확인하며 QoS·도메인 설정을 점검합니다. 브리지와 플러그인 오류도 읽어보세요. 센서 Launch의 `namespace`에는 실제 모델 이름을 지정해야 합니다. [USBL](usbl.md)은 튜토리얼 월드에 포함되어 있으므로 월드 Launch를 사용합니다.

## 영상 토픽은 보이지만 수신이 멈출 때

Gazebo에서 영상을 만들고 ROS에서 발행자도 찾지만 영상이 거의 오지 않으면 DDS 데이터 경로를 확인하세요. Cyclone DDS의 큰 UDP 데이터그램은 IP 단편화 한계에 걸릴 수 있습니다. MTU가 1500바이트인 네트워크에서는 데이터그램 크기를 줄이는 설정이 도움이 될 수 있습니다. [Cyclone DDS 단편화 설정](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation)을 참고하세요.

`rmw_cyclonedds_cpp`가 설치된 환경에서 다음 설정 파일을 한 번 만듭니다. 이미 Cyclone DDS 설정 파일을 사용한다면 탐색·인터페이스 설정을 유지하도록 기존 파일의 `General` 항목에 두 크기 설정을 합치세요.

```bash
mkdir -p "$HOME/.config/posim"
cat > "$HOME/.config/posim/cyclonedds-images.xml" <<'XML'
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain Id="any">
    <General>
      <MaxMessageSize>1400B</MaxMessageSize>
      <FragmentSize>1280B</FragmentSize>
    </General>
  </Domain>
</CycloneDDS>
XML
```

시뮬레이터와 영상 확인 도구를 종료합니다. POSIM 환경을 불러온 각 터미널에서 다음 설정을 적용한 뒤 다시 실행하세요. 기존 파일에 설정을 합쳤다면 그 파일의 경로를 사용합니다.

```bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI="file://$HOME/.config/posim/cyclonedds-images.xml"
```

이 도메인에서 ROS CLI 데몬이 실행 중이라면 한 번 종료하여 이후 CLI 명령에 새 설정이 적용되게 합니다.

```bash
ros2 daemon stop
```

[카메라 영상·수신 주기 확인](camera.md#예제-실행)을 반복합니다. 수신 주기와 함께 취득 타임스탬프도 확인하세요. 작은 패킷은 처리 부담을 늘리며, MTU가 더 작은 네트워크에는 그에 맞춰 크기를 더 줄여야 합니다.

## 로봇이 움직이지 않을 때

제어기 상태, Arm·항법 유효성, 명령 수신과 선택 모드를 확인합니다. 관절 이름과 추진기 축을 확인하고 명령 입력은 하나씩 사용하세요. 중립 입력, 일시 정지된 월드와 Arm되지 않은 autopilot에서도 로봇은 멈춰 있습니다.

## 종료가 실패할 때

데이터 확인 도구를 종료한 뒤 Launch 터미널에서 Ctrl+C를 한 번 누르고 자식 프로세스가 끝날 때까지 기다립니다. Abort, segmentation fault와 강제 종료 로그를 보관하세요. [문제 보고](https://github.com/IOES-Lab/POSIM/issues)에는 명령, 커밋, 운영체제·아키텍처와 렌더러를 함께 기록합니다.

### Discard를 선택한 뒤 RViz가 비정상 종료될 때

Ubuntu 26.04의 RViz 15.2.6은 센서 화면이 정상적으로 표시되어도 창을 닫고 **Discard**를 선택할 때 간헐적으로 비정상 종료될 수 있습니다. 다시 실행하기 전에 Launch 터미널에서 Ctrl+C를 눌러 남은 프로세스를 종료하세요. 문제를 보고할 때 RViz 버전과 창을 닫은 방법도 적어주세요.

## macOS에서 깊이 카메라 재생성

Ogre-Next 2.3.3을 사용하는 네이티브 arm64 SDK에는 깊이 카메라의 제거·재생성 과정에서 재질 수명을 보장하는 [별도 Ogre 오버레이](https://github.com/IOES-Lab/POSIM/tree/main/extras/ogre)를 사용할 수 있습니다. 안내의 빌드 도구를 실행한 뒤, `run_overlay.py`에 Gazebo 명령을 전달해 실행하세요. 오버레이를 빌드하는 것만으로 실행 중인 프로세스에 적용되지는 않습니다. 호환 SDK를 사용하고 검증 기록을 보관하세요. 이 도구는 시스템 라이브러리를 교체하거나 Linux 컨테이너를 설정하지 않습니다.

## 재현 가능한 보고 작성

소스 SHA 또는 이미지 digest, 의존성 버전, 명령·인수, 첫 오류, 기대 동작, 관련 토픽 자료형과 짧은 재현 예제를 포함합니다. 토픽이 목록에만 나타나는지, 데이터도 수신되는지 함께 적어주세요.
