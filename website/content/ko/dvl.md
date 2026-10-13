# DVL

DVL은 해저 또는 수괴에 대한 상대 속도를 추정합니다. POSIM은 Gazebo DVL 시스템과 ROS 브리지를 사용하여 관측값을 `posim_interfaces/msg/DVL`로 변환합니다.

## 센서 실행

```bash
ros2 launch posim_demos posim_sensor.launch.py \
  namespace:=nortek_dvl500_300 world_name:=dvl_world z:=-30 \
  paused:=false gui:=false headless:=true
```

같은 환경의 두 번째 터미널에서 Gazebo와 ROS 데이터를 차례로 확인합니다.

```bash
gz topic -e -t /dvl/velocity
ros2 topic echo /dvl/velocity posim_interfaces/msg/DVL --once
```

Gazebo 확인 도구를 종료한 뒤 ROS 명령을 실행합니다. 새 센서를 만들었다면 모델의 토픽 이름에 맞춥니다.

## 포함된 모델

| 모델 | 구성 계열 |
| --- | --- |
| `nortek_dvl500_300` | Nortek DVL500 |
| `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300` | Nortek DVL1000 |
| `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000` | Teledyne Explorer |
| `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL과 [CUDA 소나](sonar-tuning.md) |

센서 모델의 샘플링과 형상은 SDF에서 확인하세요.

## DVL 설정

사용자 센서는 `gz:type="dvl"`로 설정합니다. 빔 각도·기울기, 해저·수괴 추적, 속도 잡음과 기준 좌표를 지정합니다.

- DVL500-300 요청 주기: **8 Hz**
- 관측 범위: **0.3–200 m**

예제는 `<reference_frame>`에서 ENU를 설정된 전방·우현·아래 방향으로 변환합니다. ROS 브리지는 Gazebo의 frame ID를 유지합니다. 추정기가 같은 좌표계를 사용하고 해저 추적과 수괴 추적을 구분하는지 확인하세요.

## 관측값 확인

메시지 정의는 `ros2 interface show posim_interfaces/msg/DVL`로 확인합니다. 정지 상태와 제어된 이동에서 속도, 좌표계와 상태를 비교하세요. 해저 형상, 범위와 빔 교차에 따라 관측값이 달라집니다. 디스플레이가 준비된 환경에서는 모델의 시각화 설정을 켜 빔을 확인할 수 있습니다.

## DVL 빔과 출력 보기

<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png"><img width="1417" height="846" src="{{ASSET_PREFIX}}media/notion/dvl-e7cc9419.png" alt="해저를 향하는 DVL 음향 빔" loading="lazy" decoding="async"></a><figcaption>해저를 향하는 DVL 음향 빔</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif"><img width="754" height="476" src="{{ASSET_PREFIX}}media/notion/dvl-cc9c9419.gif" alt="DVL 속도 관측 출력" loading="lazy" decoding="async"></a><figcaption>DVL 속도 관측 출력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png"><img width="473" height="417" src="{{ASSET_PREFIX}}media/notion/dvl-607c9419.png" alt="Gazebo DVL 토픽 확인 화면" loading="lazy" decoding="async"></a><figcaption>Gazebo DVL 토픽 확인 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png"><img width="528" height="87" src="{{ASSET_PREFIX}}media/notion/dvl-b85c9419.png" alt="ROS DVL 토픽 확인 화면" loading="lazy" decoding="async"></a><figcaption>ROS DVL 토픽 확인 화면</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
