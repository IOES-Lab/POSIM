# 소나 빌드와 성능

현재 멀티빔 소나는 CUDA를 사용합니다. NVIDIA 드라이버, CUDA 컴파일러·런타임, cuFFT, cuBLAS와 Gazebo 렌더링을 준비한 뒤 해당 타깃을 빌드합니다.

## 도구 확인

소나를 실행할 컴퓨터 또는 컨테이너 안에서 확인합니다.

```bash
nvidia-smi
nvcc --version
```

GPU 접근과 컴파일러 설치는 각각 확인합니다. 컨테이너에서는 호스트의 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)과 이미지 안의 라이브러리를 모두 준비해야 합니다.

## CUDA 아키텍처 선택

소나 CMake에는 기본값이 `60`인 `CUDA_ARCHITECTURE` 변수가 있습니다. GPU와 CUDA 툴킷에서 지원하는 SM 버전을 선택한 뒤 작업 공간에서 다시 빌드합니다.

```bash
colcon build --merge-install --executor sequential --symlink-install \
  --cmake-args -DCUDA_ARCHITECTURE=YOUR_GPU_SM
source install/setup.bash
```

`YOUR_GPU_SM`을 해당 아키텍처 숫자로 바꿉니다. 툴킷이 오래된 타깃 지원을 종료했을 수 있으므로 지원 목록을 확인하세요. 이 프로젝트의 단수형 변수와 CMake의 `CMAKE_CUDA_ARCHITECTURES`는 구분합니다.

## 설치 타깃 확인

```bash
ros2 pkg prefix multibeam_sonar
ros2 pkg prefix multibeam_sonar_system
ros2 pkg prefix posim_multibeam_sonar_demo
```

빌드 출력에서 CUDA 타깃 생략이나 공유 라이브러리 누락을 확인합니다. [멀티빔 소나](sonar.md)를 실행하여 플러그인이 로드된 뒤 실제 영상·원시 데이터가 도착하는지도 확인하세요.

## 한 번에 하나씩 조절

| 설정 | 확인할 영향 |
| --- | --- |
| 수평 빔·수직 ray 수 | 각도 샘플링과 계산량 |
| 거리·`maxDistance` | 관측 범위와 거리 처리 |
| `raySkips` | 샘플링과 계산량의 균형 |
| 센서 `update_rate` | 요청하는 센서 처리율 |
| `writeLog`, `debugFlag` | 디스크·콘솔 출력 부하 |
| 센서 gain | 영상 표현. 물리적 반사 강도와 구분 |

같은 장면·자세에서 프레임 처리 시간, 수신 토픽 주기, Gazebo 실시간 비율, CPU·GPU와 메모리 사용량을 기록합니다. 물리 계산, 렌더링, CUDA 계산, 발행과 기록 비용을 구분하세요. 측정 전에 렌더러를 충분히 실행합니다.

계산 커널을 변경할 때는 기준 데이터를 보관합니다. 화면뿐 아니라 강도·거리 결과를 비교하세요. 관련 연구와 출처는 [인용 안내](citation.md)에 있습니다.
