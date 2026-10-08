# Install on Ubuntu

Build POSIM in a ROS workspace on Ubuntu 26.04. Use [Docker](docker.md) if you prefer an isolated Linux environment.

## 1. Get the source

Run the following in Bash:

```bash
mkdir -p ~/posim_ws/src
git clone https://github.com/IOES-Lab/POSIM.git ~/posim_ws/src/posim
cd ~/posim_ws
```

The checkout directory is `posim`. Keep this name when importing the matching repository manifest below.

## 2. Install the stack

The helper installs ROS 2 Lyrical, Gazebo Jetty, ArduSub, MAVROS and wave libraries. Read `src/posim/extras/ros-lyrical-gz-jetty-install.sh` first.

- Configures system repositories
- Uses `sudo`
- Runs `apt-get full-upgrade -y`

A dedicated Ubuntu environment is recommended.

```bash
POSIM_EXTRAS_DIR="$PWD/src/posim/extras" \
  bash src/posim/extras/ros-lyrical-gz-jetty-install.sh
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
```

Wave Sim is fetched at the pinned upstream revision and built by the helper. It is an external build dependency; a recursive Git clone is not required.

## 3. Resolve workspace dependencies

```bash
vcs import src --shallow --skip-existing \
  --input src/posim/extras/repos/posim.lyrical.repos
rosdep update --rosdistro lyrical
rosdep install --rosdistro lyrical --from-paths src --ignore-src -r -y
```

`--skip-existing` keeps the POSIM checkout already in `src/posim`. Check every reported dependency failure before building.

## 4. Build and source

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 pkg prefix posim_demos
```

The last command prints an installed package prefix. In every new terminal, load the same environment:

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
```

Continue with [First simulation](quickstart.md). NVIDIA/CUDA sonar has additional [build requirements](sonar-tuning.md).

## Update a workspace

Record the current revision, pull the desired POSIM revision, then repeat dependency resolution and the build. Keep custom model work committed before updating.

```bash
git -C ~/posim_ws/src/posim rev-parse HEAD
git -C ~/posim_ws/src/posim pull --ff-only
```
