# 소나 빌드와 성능

멀티빔 소나를 빌드한 뒤 실행할 때 WGPU·CUDA·CPU 계산 방식을 선택합니다. WGPU는 Linux에서 Vulkan, macOS에서 직접 실행할 때 Metal을 사용합니다. 모든 계산 방식에서 광선 센서의 Gazebo 렌더링이 필요합니다.

## 도구 준비

[POSIM 작업 공간 설치 안내](install.md)에 따라 ROS 2 Lyrical·Gazebo Jetty·ROS–Gazebo 브리지를 준비합니다. `~/posim_ws`와 같은 작업 공간의 최상위 폴더에서 빌드하세요.

- CPU 계산을 선택할 경우에도 WGPU 라이브러리를 빌드하기 위해 Rust와 Cargo가 필요합니다.
- Linux의 WGPU에는 Vulkan 로더와 GPU에 맞는 드라이버가 필요합니다. `vulkaninfo --summary`로 사용 가능한 어댑터를 확인합니다.
- CUDA에는 호환 NVIDIA GPU·드라이버·CUDA 컴파일러와 런타임·cuFFT·cuBLAS가 필요합니다. CMake가 툴킷을 찾으면 CUDA 지원을 포함합니다.
- 렌더링에는 Ogre2와 디스플레이·가상 디스플레이 또는 구성된 headless 렌더러가 필요합니다.

```bash
rustc --version
cargo --version
```

Rust가 없으면 [공식 Rust 설치 안내](https://www.rust-lang.org/tools/install)를 따르세요. CUDA를 사용할 때는 다음 항목도 확인합니다.

```bash
nvidia-smi
nvcc --version
```

GPU 접근과 컴파일러 설치는 각각 확인합니다. NVIDIA GPU를 사용하는 Linux 컨테이너에는 호스트의 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)과 컨테이너 내부의 드라이버·런타임 라이브러리도 필요합니다.

## 빌드하고 환경 불러오기

새 Ubuntu 작업 공간에서는 WGPU 라이브러리를 먼저 빌드하고 환경을 불러온 뒤 나머지 패키지를 빌드합니다.

```bash
source /opt/ros/lyrical/setup.bash
colcon build --merge-install --executor sequential --symlink-install \
  --packages-select wgpu_vendor
source install/setup.bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
```

macOS에서 직접 빌드할 때도 해당 작업 공간의 ROS 환경을 불러온 뒤 같은 두 빌드 명령을 사용합니다. 컴파일러나 GPU 도구를 바꾸면 작업 공간을 다시 빌드하세요.

## CUDA 아키텍처 선택

소나의 CMake 변수 `CUDA_ARCHITECTURE` 기본값은 `native`입니다. CUDA 지원을 빌드할 때 현재 GPU의 아키텍처를 선택합니다. 다른 GPU에 배포할 경우 해당 GPU와 툴킷이 지원하는 SM 값을 지정하세요.

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --packages-select multibeam_sonar \
  --cmake-args -DCUDA_ARCHITECTURE=YOUR_GPU_SM
source install/setup.bash
```

`YOUR_GPU_SM`을 아키텍처 숫자로 바꿉니다. 프로젝트는 이 값을 CMake의 `CMAKE_CUDA_ARCHITECTURES`로 전달합니다. 값을 선택하기 전에 툴킷의 지원 목록을 확인하세요.

## 설치 타깃 확인

```bash
ros2 pkg prefix wgpu_vendor
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

Linux에서는 설치된 라이브러리를 확인합니다.

```bash
test -f "$(ros2 pkg prefix wgpu_vendor)/lib/libsonar_wgpu.a"
test -f "$(ros2 pkg prefix multibeam_sonar)/lib/multibeam_sonar/libmultibeam_sonar.so"
test -f "$(ros2 pkg prefix multibeam_sonar_system)/lib/multibeam_sonar_system/libmultibeam_sonar_system.so"
```

각 명령의 종료 상태가 0이어야 합니다. [소나 예제](sonar.md)를 실행하여 로그의 계산 방식을 확인하고 영상·원시 데이터를 수신하세요. 패키지 경로나 라이브러리 파일의 존재만으로 GPU 접근까지 확인할 수는 없습니다.

## WGPU 어댑터 선택

Gazebo를 실행하는 터미널에서 다음 환경 변수를 설정합니다.

| 변수 | 동작 |
| --- | --- |
| `WGPU_BACKEND` | GPU API 선택. Linux는 `vulkan`, macOS에서 직접 실행할 때는 `metal` 등 |
| `WGPU_ADAPTER_NAME` | 대소문자 구분 없이 값이 이름에 포함된 첫 번째 어댑터 선택 |
| `WGPU_POWER_PREF` | `high`·`low`·`none` 전력 선호 설정. 기본값은 `high` |

예를 들어 이름에 `RTX`가 포함된 Vulkan 어댑터를 선택합니다.

```bash
WGPU_BACKEND=vulkan WGPU_ADAPTER_NAME=RTX WGPU_POWER_PREF=high \
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

컴퓨터에서 사용 가능한 어댑터 이름을 지정하세요. 일치하는 이름이 없으면 로그에 사용 가능한 이름을 출력합니다. 선택된 WGPU 어댑터는 같은 Gazebo 프로세스의 소나 센서들이 공유합니다. 어댑터를 바꾸려면 새 프로세스를 시작하세요.

## Apple Silicon

macOS에서 직접 실행하려면 Gazebo Jetty·Ogre2·ROS–Gazebo 브리지가 포함된 ROS 2 작업 공간의 환경을 불러옵니다. 위 안내에 따라 Rust·Cargo와 소나 패키지를 빌드하세요. WGPU 계산은 Metal을 통해 Apple GPU를 사용합니다.

```bash
WGPU_BACKEND=metal \
ros2 launch posim_multibeam_sonar_demo multibeam_sonar_demo.launch.py \
  compute_backend:=wgpu
```

Metal은 음향 계산을 담당하며, Gazebo의 렌더링 환경도 필요합니다. CPU에서 음향을 계산하려면 `compute_backend:=cpu`를 사용하세요. CUDA에는 호환 NVIDIA 환경이 필요합니다.

Apple Silicon 호스트의 Linux ARM64 컨테이너는 [Docker 설치와 가상 디스플레이 렌더링](docker.md)을 따르세요. 컨테이너 안에서 `POSIM_SONAR_COMPUTE_BACKEND=cpu`를 지정하여 [서버 모드 소나 명령](sonar.md)을 실행합니다. Linux 컨테이너에서 WGPU를 사용하려면 Vulkan 어댑터가 필요합니다. 컨테이너의 계산 API는 macOS에서 직접 실행할 때의 Metal과 별도로 구성합니다.

## 시작 오류 해결

| 증상 | 조치 |
| --- | --- |
| `wgpu_vendorConfig.cmake`를 찾지 못함 | `wgpu_vendor`를 빌드하고 `install/setup.bash`를 불러온 뒤 나머지 패키지 빌드 |
| WGPU에서 사용 가능한 어댑터가 없다고 표시 | GPU 드라이버·Linux의 `vulkaninfo --summary`·어댑터 선택 변수 확인. GPU 계산을 사용할 수 없으면 `auto`나 `cpu` 선택 |
| CUDA를 직접 선택했을 때 실패 | CUDA 빌드 지원·`nvidia-smi`·런타임 라이브러리·컨테이너 GPU 접근 확인 |
| 알 수 없는 계산 방식이라고 표시 | `auto`·`wgpu`·`cuda`·`cpu` 중 선택 |
| 같은 이름의 월드가 이미 실행 중이라고 표시 | 기존 POSIM Launch를 Ctrl+C로 종료한 뒤 다시 실행 |
| 계산 방식은 초기화되지만 영상이 오지 않음 | 월드의 일시 정지를 해제하고 렌더러와 [소나 토픽](sonar.md) 확인 |

## 한 번에 하나씩 조절

| 설정 | 확인할 영향 |
| --- | --- |
| 수평 빔·수직 음선 수 | 각도 샘플링과 계산량 |
| 거리·`maxDistance` | 관측 범위와 거리 처리 |
| `raySkips` | 샘플링과 계산량의 균형 |
| 센서 `update_rate` | 설정한 센서 갱신 빈도 |
| `writeLog`, `debugFlag` | 디스크·콘솔 출력 부하 |
| 센서 이득(gain) | 영상 표현. 물리적 반사 강도와 구분 |

같은 장면·자세에서 프레임 처리 시간, 수신 토픽 주기, Gazebo 실시간 비율과 CPU·GPU 사용량을 기록합니다. 물리 계산·렌더링·음향 계산·발행·기록 비용을 구분하세요. 렌더러를 충분히 실행한 뒤 측정합니다. 설정된 30 Hz는 요청 주기이며, 실제 수신 처리율은 장면·센서 설정·하드웨어에 따라 달라집니다.

계산 방식이나 센서 설정을 비교할 때는 원시 강도·거리 데이터를 사용하고 gain·표시 설정을 일정하게 유지하세요. 연구 출처와 저자 표시는 [인용 안내](citation.md)에 있습니다.

## GPU 계산 흐름

<figure><a href="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png"><img width="1049" height="554" src="{{ASSET_PREFIX}}media/notion/sonar-optimization-a26c9419.png" alt="소나 계산의 GPU 처리 단계" loading="lazy" decoding="async"></a><figcaption>소나 계산의 GPU 처리 단계</figcaption></figure>

소나 계산은 후방 산란·빔 보정·거리 압축으로 구성됩니다. 프로파일링할 때 계산 시간과 메모리 전송을 구분하세요. 그림 출처: POSIM Notion Wiki.
