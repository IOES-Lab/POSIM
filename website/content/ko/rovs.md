# ROV와 BlueROV2

`namespace` 인수로 패키지에 포함된 로봇 모델을 선택합니다. 이 인수는 ROS 토픽 이름만 바꾸는 옵션이 아니라 모델 디렉터리를 선택하는 값입니다.

## 모델 종류

| 모델 | 용도 |
| --- | --- |
| `rexrov` | Gazebo 추진기, 유체역학과 Odometry를 사용하는 ROV |
| `bluerov2` | ArduSub SITL·MAVROS가 연결된 BlueROV2 |
| `bluerov2_heavy` | Heavy 구성 |
| `bluerov2_heavy_multibeam_sonar` | 소나를 포함한 Heavy 구성 |

설치 방식에 맞게 [준비한 POSIM 터미널](terminals.md)을 사용합니다. REXROV는 [아래 예제](#start-rexrov)로 시작하세요.

## BlueROV2 실행

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=bluerov2 world_name:=posim_ocean_waves z:=-0.5 paused:=false \
  gui:=true headless:=false use_teleop:=false use_web_joystick:=false \
  open_qgc:=false open_virtual_joystick:=false
```

Heavy 모델은 `namespace:=bluerov2_heavy`로 선택합니다. Launch 출력과 `/mavros/state`에서 autopilot 연결을 확인하세요. 소나가 포함된 모델에는 [설정](sonar-tuning.md)이 필요합니다.

## 키보드 제어

데스크톱의 대화형 터미널에서 `gui:=true headless:=false use_teleop:=true`로 실행합니다. BlueROV 키는 다음과 같습니다.

| 키 | 동작 |
| --- | --- |
| `c` / `x` | Arm / disarm |
| `w` / `s` | 전진 / 후진 |
| `a` / `d` | 좌회전 / 우회전 |
| `r` / `f` | 상승 / 하강 |
| `h` / `j` | ALT_HOLD / STABILIZE |
| Space | 중립 명령 |

## 로컬 브라우저 조이스틱

환경을 불러온 별도 터미널에서 설치된 조이스틱 페이지를 제공합니다.

```bash
python3 -m http.server 8080 --bind 127.0.0.1 \
  --directory "$(ros2 pkg prefix --share posim_robot_models)/web"
```

Teleoperation을 켜고 BlueROV2를 시작합니다.

```bash
ros2 launch posim_demos posim_robot.launch.py \
  namespace:=bluerov2 world_name:=posim_ocean_waves z:=-0.5 paused:=false \
  gui:=true headless:=false use_teleop:=true use_web_joystick:=true \
  joystick_ws_host:=127.0.0.1 joystick_ws_port:=8765 \
  open_virtual_joystick:=true open_qgc:=false \
  virtual_joystick_url:='http://127.0.0.1:8080/virtual_joystick.html?host=127.0.0.1&port=8765'
```

WebSocket 브리지가 teleoperation Launch 안에 있으므로 `use_teleop:=true`를 유지합니다. 브라우저가 자동으로 열리지 않으면 같은 Linux 데스크톱에서 위 URL을 여세요. QGC 설치, 가상 조이스틱과 영상 수신은 [QGroundControl로 제어하기](qgroundcontrol.md)를 따릅니다. POSIM Docker 이미지에는 QGC가 포함되어 있습니다.

프로그램에서 센서를 구독하거나 명령을 보내려면 [ROS 2와 제어](ros.md)로 이어가세요.

## 로봇 동작 살펴보기

<figure><a href="{{ASSET_PREFIX}}media/notion/rovs-670c9419.gif"><img width="782" height="494" src="{{ASSET_PREFIX}}media/notion/rovs-670c9419.gif" alt="REXROV의 수중 운항과 자세 변화" loading="lazy" decoding="async"></a><figcaption>REXROV의 수중 운항과 자세 변화</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.

## REXROV 시작 {#start-rexrov}

[준비한 터미널 A](terminals.md)에서 Python·추진기 제어 가이드용 예제를 별도로 실행합니다. 해양 월드와 ROS 브리지를 시작하고 REXROV를 z = −5 m에 배치합니다. 터미널 B에서 해당 가이드를 따라가는 동안 이 세션을 유지하세요.

{{CODE:rovs:3}}
