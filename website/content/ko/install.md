# Ubuntu 설치

Ubuntu 26.04의 ROS 작업 공간에서 POSIM을 빌드합니다. 독립된 Linux 환경을 사용하려면 [Docker 환경](docker.md)을 선택하세요.

## 1. 소스 내려받기

Bash 터미널에서 실행합니다.

```bash
mkdir -p ~/posim_ws/src
git clone https://github.com/IOES-Lab/POSIM.git ~/posim_ws/src/posim
cd ~/posim_ws
```

소스 디렉터리 이름은 `posim`입니다. 아래 저장소 목록을 가져올 때도 이 이름을 유지합니다.

## 2. 기본 환경 설치

설치 도우미는 시스템 패키지 저장소를 설정하고 ROS 2 Lyrical, Gazebo Jetty, ArduSub, MAVROS와 외부 파도 라이브러리를 설치합니다. `sudo`를 사용하며 `apt-get full-upgrade -y`로 시스템 전체 패키지를 갱신하므로 실행 전에 `src/posim/extras/ros-lyrical-gz-jetty-install.sh` 내용을 확인하세요. 전용 Ubuntu 환경을 사용하거나 기존 시스템에 미칠 영향을 검토한 뒤 진행합니다.

```bash
POSIM_EXTRAS_DIR="$PWD/src/posim/extras" \
  bash src/posim/extras/ros-lyrical-gz-jetty-install.sh
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
```

Wave Sim은 도우미가 지정된 업스트림 커밋을 내려받아 빌드합니다. 외부 빌드 의존성이므로 Git의 재귀 클론 옵션은 필요하지 않습니다.

## 3. 작업 공간 의존성 설치

```bash
vcs import src --shallow --skip-existing \
  --input src/posim/extras/repos/posim.lyrical.repos
rosdep update --rosdistro lyrical
rosdep install --rosdistro lyrical --from-paths src --ignore-src -r -y
```

`--skip-existing`은 이미 받은 `src/posim`을 유지합니다. 의존성 오류가 있다면 빌드를 시작하기 전에 해당 패키지를 확인합니다.

## 4. 빌드하고 환경 불러오기

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 pkg prefix posim_demos
```

마지막 명령은 설치된 패키지 경로를 확인합니다. 시뮬레이션 실행과 센서 데이터 수신까지 확인하는 명령은 아닙니다. 새 터미널에서도 같은 환경을 불러옵니다.

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
```

이어서 [첫 시뮬레이션](quickstart.md)을 실행하세요. NVIDIA/CUDA 소나는 [추가 빌드 설정](sonar-tuning.md)이 필요합니다.

## 작업 공간 갱신

현재 커밋을 기록하고 필요한 POSIM 커밋을 받은 뒤 의존성 설치와 빌드를 반복합니다. 직접 수정한 로봇 모델은 갱신 전에 커밋하여 보관하세요.

```bash
git -C ~/posim_ws/src/posim rev-parse HEAD
git -C ~/posim_ws/src/posim pull --ff-only
```

이전 패키지 이름에서 이전할 때는 새 작업 공간을 사용합니다. 이전 설치 파일과 환경 훅이 새 `posim_*` 패키지를 가리는 일을 피할 수 있습니다.
