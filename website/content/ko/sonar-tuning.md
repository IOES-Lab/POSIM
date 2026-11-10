# 소나 설정과 실험

계산 백엔드를 선택한 뒤 관측 범위, 세부 표현과 갱신율을 비교합니다. 빠른 시작, SDF와 설정표는 [멀티빔 소나 참고 문서](sonar.md)에 있습니다.

## 실행 환경 선택

[준비한 POSIM 터미널](terminals.md)을 사용하세요. Ubuntu 네이티브, WSL, Docker 중 POSIM을 설치한 같은 환경 안에서 수정·빌드·실행합니다. `POSIM_WS`는 그 환경의 작업 공간 경로입니다. 어떤 계산 백엔드를 선택해도 Gazebo의 렌더링 센서는 필요합니다.

| 실행 환경 | 계산 방식과 준비 사항 |
| --- | --- |
| Linux 네이티브 | CPU, Vulkan 장치를 사용할 수 있는 WGPU, 호환 NVIDIA GPU·툴킷을 사용하는 CUDA |
| WSL 2 또는 Linux 컨테이너 | CPU 사용 가능. GPU 계산에는 해당 환경 **안에서** 장치와 드라이버에 접근할 수 있어야 합니다. |
| macOS Docker·RDP | CPU로 시작하세요. Linux 컨테이너는 Mac의 Metal 장치를 제공하지 않습니다. |
| macOS 네이티브 | 별도로 준비한 네이티브 ROS·Gazebo 개발 환경에서 WGPU의 Metal 사용 가능. [macOS 설치 안내](install-macos.md)는 Docker·RDP 방식입니다. |

`auto`는 CUDA → WGPU → CPU 순으로 선택합니다. 비교 실험에서는 `cpu`, `wgpu`, `cuda`를 직접 지정하세요. 지정한 백엔드를 사용할 수 없으면 센서 초기화가 중단됩니다. [선택값 표](sonar.md#compute-backend)를 참고하세요.

## 소나 패키지 빌드 {#build-the-sonar-packages}

[소스 설치](install.md)와 의존성 준비를 마친 환경에서 진행합니다. `wgpu_vendor`는 CPU 사용 시에도 Rust 라이브러리를 빌드합니다. Rust와 Cargo가 없다면 [공식 설치 안내](https://www.rust-lang.org/tools/install)를 따르세요.

{{CODE:sonar-tuning:0}}

환경을 불러온 작업 공간에서 vendor 라이브러리, 센서, 월드 시스템과 예제를 빌드합니다.

{{CODE:sonar-tuning:1}}

CUDA는 선택 사항입니다. 설치된 CUDA 툴킷을 제외하고 CPU·WGPU용으로 빌드하려면 `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`을 덧붙입니다. Rust 의존성 버전은 `Cargo.lock`으로 고정되어 있으며, 첫 다운로드에는 인터넷 연결이 필요합니다.

{{CODE:sonar-tuning:2}}

네 패키지를 확인한 뒤 [소나 예제](sonar.md)를 실행해 영상·원시 데이터를 받으세요. 패키지가 목록에 있다고 센서 로드까지 확인된 것은 아닙니다. 컨테이너는 그 안에서 확인하고, 패키지가 없으면 소스에서 빌드합니다. 이미지 태그만으로 포함 여부를 판단하지 마세요.

## WGPU 장치 선택

Linux에서는 Gazebo를 실행할 환경 안에서 `vulkaninfo --summary`를 확인합니다. 시스템에 맞는 Vulkan 드라이버와 도구가 필요합니다. llvmpipe 같은 소프트웨어 장치는 Vulkan을 실행할 수 있지만 하드웨어 가속은 제공하지 않습니다. WGPU 시작 출력에서 선택한 장치를 확인하세요.

{{CODE:sonar-tuning:3}}

| 환경 변수 | 의미 |
| --- | --- |
| `WGPU_BACKEND` | Linux는 `vulkan`, macOS 네이티브 프로세스는 `metal`로 API를 지정합니다. 생략하면 플랫폼 기본값을 사용합니다. |
| `WGPU_ADAPTER_NAME` | 이름에 지정한 문자열이 포함된 첫 장치를 선택합니다. 대소문자는 구분하지 않습니다. 드라이버·시작 출력에 표시된 이름을 사용하세요. |
| `WGPU_POWER_PREF` | `high`, `low`, `none`. 장치 이름을 지정하지 않으면 기본값은 `high`입니다. |

실행 전에 변수를 설정합니다. 장치 초기화는 프로세스 안에서 공유되므로 변경 후에는 시뮬레이션을 종료하고 다시 실행하세요. 그래픽 API는 [wgpu 프로젝트](https://github.com/gfx-rs/wgpu)를 참고하세요. 초기화나 자원 검사에 실패하면 출력된 장치와 제한값을 확인하고, `cpu`를 직접 선택하거나 센서 크기를 줄입니다.

## CUDA를 사용할 때

NVIDIA 드라이버, CUDA 컴파일러·런타임, cuFFT와 cuBLAS가 필요합니다. 실제 실행 환경에서 확인하세요.

{{CODE:sonar-tuning:4}}

Linux 컨테이너는 호스트에 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)을 설정합니다. Windows는 [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)과 필요한 경우 [Docker Desktop GPU 설정](https://docs.docker.com/desktop/features/gpu/)을 따릅니다. 디스플레이 드라이버는 Windows에 설치하고 WSL 안에 Linux용 드라이버를 설치하지 않습니다. 컨테이너 안에도 CUDA 라이브러리가 필요합니다.

GPU와 툴킷이 지원하는 아키텍처를 선택하세요. 아래 SM 75 예제의 `75`를 자신의 GPU 값으로 바꿉니다. `OFF`는 CPU·WGPU 전용 빌드에서 껐던 CUDA 탐색을 다시 켭니다.

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code`로 지원 타깃을 확인합니다. 아키텍처를 생략하면 컴파일러 기본값을 사용합니다. 여러 툴킷 중 하나를 고를 때는 `-DCUDAToolkit_ROOT=/path/to/cuda`를 지정하고 해당 `bin`을 `PATH`에 넣습니다. 예제를 `compute_backend:=cuda`로 실행해 CUDA 사용을 확인하세요.

## 기준 결과 측정

장면, 표적 거리와 센서 자세를 고정합니다. 같은 환경을 불러온 터미널 B에서 영상 수신 빈도를 측정합니다.

{{CODE:sonar-tuning:6}}

Ctrl+C로 종료한 뒤 시뮬레이션 속도를 확인합니다.

{{CODE:sonar-tuning:7}}

소나 예제의 월드 엔티티 이름은 `default`입니다. 수신 빈도와 함께 `real_time_factor`를 기록하세요. 시뮬레이션 시간 기준으로 요청한 30 Hz가 실제 시간에서 초당 30장의 수신을 보장하지는 않습니다. 확인 도구는 Ctrl+C로 종료합니다.

## 샘플링과 거리 조절

[소나 모델](sonar.md)을 복사해 설정을 하나씩 바꿉니다. 작업 공간을 빌드하고 환경을 불러온 뒤 같은 자세에서 시뮬레이션을 다시 실행합니다.

| 실험 | 첫 비교 | 확인할 결과 |
| --- | --- | --- |
| 각도별 세부 표현 | 시야각을 유지하고 수평 빔 512개와 256개 비교 | 가까운 표적의 분리와 수신 빈도 |
| 수직 샘플링 | 수직 음선 300개와 150개 비교 | 표적의 연속성과 계산 시간 |
| 관측 범위 | 음선 범위와 `maxDistance`를 함께 변경 | 가장 먼 표적과 거리 축의 크기 |
| 갱신율 | 30 Hz와 10 Hz 요청 비교 | 수신 빈도와 시뮬레이터 실시간 비율 |
| 음선 건너뛰기 | `raySkips` 10과 5 비교 | 수직 샘플링의 세부 표현과 계산 시간 |

거리 간격은 `soundSpeed / (2 × bandwidth)`이고, 셀 수는 `ceil(maxDistance / 간격) + 1`입니다. 1500 m/s, 29.9 kHz, 15 m에서는 599개입니다. 마지막 좌표는 `maxDistance`를 조금 넘을 수 있지만 설정 범위 밖의 반사값은 제외됩니다. 실제 거리 배열과 빔 수는 원시 메시지나 CSV에서 읽으세요.

WGPU는 2의 거듭제곱이 아닌 경우에도 요청한 변환 길이를 유지하고 FFT 단계를 자동 선택합니다. 거리와 대역폭을 늘리면 메모리·계산량도 증가합니다. 기본 격자 모드에서는 CPU가 반사값을 가장 가까운 거리 셀에 배치하고 GPU는 주파수 영역에서 처리합니다. 백엔드를 비교할 때는 진폭과 거리 응답도 각각 측정하세요.

## 반복 가능한 비교 기록

렌더링이 안정된 뒤 측정합니다. 처리율 측정 중에는 `writeLog`와 `debugFlag`를 끄고, [소나 CSV](sonar.md)나 [ROS 데이터 기록](visualization.md)은 따로 수행하세요. `blazingSonarImage=false`는 난수 시드를 1234로 고정합니다.

SDF, POSIM 커밋, 백엔드·장치, 드라이버·툴킷 버전, 프레임 시간, 수신 빈도, 실시간 비율과 메모리 사용량을 보관합니다. 화면과 함께 원시 강도·거리 데이터도 비교하세요. 연구 모델 출처는 [인용 안내](citation.md)에 있습니다.

## GPU 계산 흐름

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="소나 계산의 GPU 처리 단계" loading="lazy" decoding="async"></a><figcaption>소나 계산의 GPU 처리 단계</figcaption></figure>

GPU 처리는 음선 합산, 창 함수·보정, FFT 단계로 구성됩니다. 계산 시간과 메모리 전송 시간을 구분해서 측정하세요. 그림 출처: POSIM Notion Wiki.
