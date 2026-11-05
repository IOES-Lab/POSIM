# QGroundControl로 제어하기

QGroundControl(QGC)에서 시뮬레이션 중인 BlueROV2에 연결하고, 가상 조이스틱으로 조종하며 전방 카메라 영상을 확인합니다. 먼저 [첫 시뮬레이션](quickstart.md)을 실행해 보고 [터미널 A와 B를 준비하세요](terminals.md).

## 1. POSIM과 같은 Linux 환경에서 QGC 열기

| POSIM 설치 방식 | QGC를 실행할 위치 |
|---|---|
| Ubuntu 네이티브 | 공식 Linux 앱을 설치하고 Ubuntu에서 실행합니다. |
| Windows WSL 네이티브 | POSIM을 설치한 Ubuntu 배포판 안에 Linux 앱을 설치하고 실행합니다. |
| Linux·Windows Docker | POSIM 컨테이너에 포함된 앱을 컨테이너 안에서 실행합니다. |
| macOS Docker·RDP | RDP 데스크톱 안의 터미널에서 컨테이너에 포함된 앱을 실행합니다. |

네이티브 설치에서는 공식 [QGC 다운로드·설치 안내](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html)를 따르세요. CPU 아키텍처에 맞는 Linux 앱을 받습니다. WSL에서도 Ubuntu 안에서 실행합니다.

Docker에서는 터미널 B에 다음을 입력합니다.

```bash
qgroundcontrol &
```

끝의 `&`는 QGC를 열어 둔 채 터미널에서 다음 명령을 입력할 수 있게 합니다. 네이티브 앱을 별도 터미널에서 실행했다면 그 터미널도 열어 두세요. 처음 실행할 때 환경 설정이 나오면 단위를 **Metric System**으로 선택해 이 안내와 맞춥니다.

이 안내에서는 QGC, MAVROS, Gazebo를 같은 Linux 환경에서 실행합니다. 상태 정보는 **UDP 14550**, 영상은 **UDP 5600**을 사용하므로 별도의 Docker 포트 연결은 필요하지 않습니다. macOS에서는 RDP 접속만 컨테이너 외부를 거칩니다.

## 2. 가상 조이스틱 설정하기

**Application Settings → Fly View → Virtual Joystick**을 엽니다.

- **Virtual Joystick**을 켭니다.
- **Auto-Center Throttle**을 켜서 스틱을 놓으면 명령이 중앙으로 돌아오도록 합니다.
- 아래 설명과 같은 배치를 사용하려면 **Left-Handed Mode**는 끕니다.

<figure>
  <img src="{{ASSET_PREFIX}}media/guides/qgc-virtual-joystick.png" alt="가상 조이스틱과 스로틀 자동 중앙 복귀를 켠 QGC 설정 화면" loading="lazy" width="1280" height="900">
  <figcaption>BlueROV2를 조종하기 전에 가상 조이스틱과 스로틀 자동 중앙 복귀를 켭니다.</figcaption>
</figure>

## 3. 영상 수신 설정하기

**Application Settings → Video**에서 다음 값을 설정합니다.

| 설정 | 값 |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`: 카메라 영상 비율 자동 적용 |
| Disable Video Stream When Disarmed | 끔 |
| Auto-reconnect | 켬 |

<figure>
  <img src="{{ASSET_PREFIX}}media/guides/qgc-video-settings.png" alt="UDP 5600 포트로 H.264 영상을 받도록 설정한 QGC" loading="lazy" width="1280" height="900">
  <figcaption>시뮬레이터와 같은 Linux 환경에서 카메라 영상을 수신합니다.</figcaption>
</figure>

## 4. BlueROV2와 카메라 송신 시작하기

이전에 실행한 POSIM 시뮬레이션을 종료합니다. 준비한 터미널 A에서 실행하세요.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=bluerov2 world_name:=camera_tutorial \
  x:=3 z:=-94 paused:=false gui:=true headless:=false \
  use_teleop:=false use_web_joystick:=false \
  open_qgc:=false open_virtual_joystick:=false qgc_video:=true
```

Gazebo에 해저 물체와 로봇이 나타납니다. QGC는 ArduSub에 자동으로 연결됩니다. 기체 초기화가 끝날 때까지 기다리세요.

준비한 터미널 B에서 카메라 송신을 시작합니다.

```bash
gz topic -t /model/bluerov2/camera/enable_streaming \
  -m gz.msgs.Boolean -p 'data: true' --duration 1
```

**Fly View**로 돌아온 뒤 작은 영상 창을 클릭하면 카메라가 주 화면으로 표시됩니다.

<figure>
  <img src="{{ASSET_PREFIX}}media/guides/qgc-camera-control.png" alt="가상 조이스틱과 함께 표시되는 BlueROV2 전방 카메라 실시간 영상" loading="lazy" width="1280" height="900">
  <figcaption>카메라 예제 월드의 물체가 전방 영상에 보입니다. 영상 위의 가상 조이스틱으로 계속 조종할 수 있습니다.</figcaption>
</figure>

기본 카메라는 **시뮬레이션 시간 기준 5 Hz, 640 × 480** 영상을 만듭니다. 소프트웨어 렌더링으로 시뮬레이션이 느려지면 화면에 도착하는 영상도 느려집니다. BlueROV2에는 양성 부력이 있으므로 시동을 걸지 않고 기다리는 동안 떠올라 물체가 화면 밖으로 벗어날 수 있습니다.

`qgc_video:=true`를 지정하면 이번 실행에 H.264 송신기를 추가합니다. 지정하지 않으면 QGC 영상 송신기를 만들지 않습니다. 이 영상은 로봇의 일반 RGB 카메라 출력입니다. 별도의 [수중 카메라 필터](camera.md)는 자체 ROS 토픽으로 처리 결과를 보냅니다.

## 5. 시동을 걸고 조종한 뒤 정지하기

1. 상태가 **Ready**가 될 때까지 기다리고 **Manual** 모드를 선택합니다.
2. 기체 상태 메뉴에서 **Arm**을 길게 눌러 시동을 겁니다.
3. 오른쪽 스틱을 앞뒤로 짧게 움직이며 로봇과 카메라 영상의 변화를 확인합니다.
4. 스틱을 놓으면 명령이 중립으로 돌아옵니다. 해류나 관성에 의한 움직임은 남을 수 있습니다.
5. 마치면 상태 메뉴에서 **Disarm**을 길게 눌러 시동을 해제합니다.

QGC로 조종하는 동안 키보드·웹 조이스틱 제어기는 꺼 두세요. 여러 제어기가 동시에 명령을 보내는 것을 방지합니다.

영상 송신을 멈추려면 터미널 B에서 실행합니다.

```bash
gz topic -t /model/bluerov2/camera/enable_streaming \
  -m gz.msgs.Boolean -p 'data: false' --duration 1
```

이어서 터미널 A에서 **Ctrl+C**로 시뮬레이션을 종료하고 QGC도 닫습니다.

## 연결이나 영상이 보이지 않을 때

- **기체가 연결되지 않음:** QGC와 POSIM이 같은 Linux 환경에 있는지, 다른 QGC 창이 UDP 14550을 사용 중인지 확인합니다.
- **영상 대기 상태:** H.264 입력 방식, UDP URL, `qgc_video:=true` 옵션을 확인합니다. 터미널 B에서 송신을 시작하고 몇 프레임이 도착할 때까지 기다립니다.
- **다른 영상 포트 사용:** 실행 명령에 `qgc_video_port:=5602`를 추가하고 QGC에는 `127.0.0.1:5602`를 입력합니다. 송신·수신 포트가 같아야 합니다.
- **이전 설치에 `qgc_video` 옵션이 없음:** POSIM을 업데이트하거나 [최신 이미지를 받고 컨테이너를 다시 만드세요](docker.md#update-an-existing-container).
- **펌웨어·직렬 포트 안내:** 이 시뮬레이션은 POSIM에 설정된 ArduSub 빌드와 네트워크 연결을 사용합니다. 펌웨어 업로드나 직렬 포트 설정은 필요하지 않습니다.

녹화와 수신 옵션은 공식 [QGC 영상 설정 안내](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html)를 참고하세요. ROS 명령과 기체 제어 인터페이스는 [추진기와 조이스틱 제어](robot-control.md)에서 이어집니다.
