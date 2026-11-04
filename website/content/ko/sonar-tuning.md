# 소나 설정과 실험

소나 실험에 필요한 관측 범위, 세부 표현과 갱신율을 선택합니다. 빠른 시작, SDF 구성과 설정값의 정의는 [멀티빔 소나 참고 문서](sonar.md)에 있습니다. 이 가이드는 설정별 결과를 비교하고 CUDA 실행 환경을 준비하는 방법을 설명합니다.

## 기준 결과 측정

[BlueView P900 예제](sonar.md)를 실행하고 장면, 표적 거리와 센서 자세를 고정합니다. 처리율을 측정하기 전에 영상과 원시 관측값이 도착하는지 확인하세요. CUDA 라이브러리가 없다면 먼저 아래 도구 준비 과정을 진행합니다.

같은 환경을 불러온 다른 터미널에서 영상 수신 주기를 측정합니다.

```bash
ros2 topic hz /sensor/multibeam_sonar/sonar_image
```

안정된 표본을 모은 뒤 Ctrl+C로 종료합니다. 시뮬레이션 속도도 따로 확인합니다.

```bash
gz topic -e -t /world/default/stats
```

소나 예제의 월드 엔티티 이름은 `default`입니다. 수신 주기와 함께 `real_time_factor`를 기록하세요. 시뮬레이션 시간 기준으로 요청한 30 Hz가 실제 시간에서 초당 30장의 수신을 보장하지는 않습니다. 확인 도구는 Ctrl+C로 종료합니다.

## 비교 실험 선택

[소나 모델](sonar.md)을 복사해 설정을 하나씩 바꾸고 작업 공간을 빌드·source한 뒤 같은 자세에서 시뮬레이션을 다시 실행합니다.

| 실험 | 첫 비교 | 확인할 결과 |
| --- | --- | --- |
| 각도별 세부 표현 | 시야각을 유지하고 수평 빔 512개와 256개 비교 | 가까운 표적의 분리와 수신 주기 |
| 수직 샘플링 | 수직 ray 300개와 150개 비교 | 표적의 연속성과 계산 시간 |
| 관측 범위 | ray 범위와 `maxDistance`를 함께 변경 | 가장 먼 표적과 거리 축의 크기 |
| 갱신율 | 30 Hz와 10 Hz 요청 비교 | 수신 주기와 시뮬레이터 실시간 비율 |
| Ray 건너뛰기 | `raySkips` 10과 5 비교 | 수직 샘플링의 세부 표현과 계산 시간 |

표의 설정으로 비교 실험을 수행해 효과를 측정합니다. 처리율을 측정할 때는 `writeLog`와 `debugFlag`를 끄고, 원시 관측값이나 처리 시간을 조사할 때 따로 켜세요. 실행마다 SDF, POSIM 커밋, GPU·툴킷 버전과 관측값을 보관합니다. 토픽 기록은 [ROS 2 안내](ros.md)를 참고하세요.

## 도구 확인

[멀티빔 소나 플러그인](sonar.md)에는 NVIDIA 드라이버·CUDA 컴파일러와 런타임·cuFFT·cuBLAS·Gazebo 렌더링이 필요합니다.

소나를 실행할 컴퓨터 또는 컨테이너 안에서 확인합니다.

```bash
nvidia-smi
nvcc --version
```

GPU 접근과 컴파일러 설치는 각각 확인합니다. 컨테이너에서는 호스트의 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)과 이미지 안의 라이브러리를 모두 준비해야 합니다.

## CUDA 아키텍처 선택

CMake의 `CMAKE_CUDA_ARCHITECTURES`에 GPU와 툴킷이 지원하는 SM 버전을 지정합니다. 다음 예제는 SM 75용으로 빌드합니다. `75`를 자신의 GPU 아키텍처로 바꾼 뒤 환경을 불러온 작업 공간에서 실행하세요.

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --cmake-args -DCMAKE_CUDA_ARCHITECTURES=75
source install/setup.bash
```

`nvcc --list-gpu-code`로 툴킷의 지원 타깃을 확인합니다. 아키텍처 설정을 생략하면 CMake는 컴파일러의 기본값을 사용합니다. CUDA가 여러 개 설치되어 있으면 `-DCUDAToolkit_ROOT=/path/to/cuda`로 사용할 툴킷을 선택하세요. 셸에서 `nvcc`를 실행할 때는 해당 툴킷의 `bin` 디렉터리를 `PATH`에 포함합니다.

## 설치 타깃 확인

```bash
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

CUDA 타깃이 빌드되었는지 라이브러리 파일을 확인합니다. 패키지는 CUDA 타깃을 생략해도 등록될 수 있습니다.

```bash
test -f "$(ros2 pkg prefix multibeam_sonar)/lib/multibeam_sonar/libmultibeam_sonar.so"
test -f "$(ros2 pkg prefix multibeam_sonar_system)/lib/multibeam_sonar_system/libmultibeam_sonar_system.so"
```

각 명령의 종료 상태가 0인지 확인하세요. 빌드·실행 로그에서 타깃 생략과 로드 오류를 확인하고, [멀티빔 소나](sonar.md)에서 영상·원시 데이터를 받습니다.

## 한 번에 하나씩 조절

| 설정 | 확인할 영향 |
| --- | --- |
| 수평 빔·수직 음선 수 | 각도 샘플링과 계산량 |
| 거리·`maxDistance` | 관측 범위와 거리 처리 |
| `raySkips` | 샘플링과 계산량의 균형 |
| 센서 `update_rate` | 설정한 센서 갱신 빈도 |
| `writeLog`, `debugFlag` | 디스크·콘솔 출력 부하 |
| `blazingSonarImage` | 변화하는 난수 시드와 고정 시드 1234의 반복성 비교 |

같은 장면·자세에서 프레임 처리 시간, 수신 토픽 주기, Gazebo 실시간 비율, CPU·GPU와 메모리 사용량을 기록합니다. 물리 계산, 렌더링, CUDA 계산, 발행과 기록 비용을 구분하세요. 측정 전에 렌더러를 충분히 실행합니다.

계산 커널을 변경할 때는 기준 데이터를 보관합니다. 화면뿐 아니라 강도·거리 결과를 비교하세요. 관련 연구와 출처는 [인용 안내](citation.md)에 있습니다.

## GPU 계산 흐름

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="소나 계산의 GPU 처리 단계" loading="lazy" decoding="async"></a><figcaption>소나 계산의 GPU 처리 단계</figcaption></figure>

소나 계산은 산란점 합산, 창 함수·보정과 FFT 단계로 구성됩니다. 프로파일링할 때 각 단계의 실행 시간과 GPU 메모리 전송을 구분하세요. 그림 출처: POSIM Notion Wiki.
