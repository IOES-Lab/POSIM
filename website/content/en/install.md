# Ubuntu: native installation

Build POSIM in a ROS workspace on Ubuntu 26.04. Use [Linux: Docker installation](install-ubuntu-docker.md) if you prefer an isolated Linux environment.

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
vcs import src --skip-existing \
  --input src/posim/extras/repos/posim.lyrical.repos
rosdep update --rosdistro lyrical
rosdep install --rosdistro lyrical --from-paths src --ignore-src -r -y
```

`--skip-existing` keeps the POSIM checkout already in `src/posim`. Check every reported dependency failure before building.

The manifest also imports pinned `ros_gz` sources, including the bridge lifetime and POSIX launch corrections. An existing `src/ros_gz` is preserved by `--skip-existing`; compare its origin and commit with the manifest before building. Use a fresh workspace when retaining an older checkout. Source this workspace after building so ROS uses its bridge and launch files.

## 4. Build and source

```bash
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
ros2 pkg prefix posim_demos
ros2 pkg prefix ros_gz_bridge
ros2 pkg prefix ros_gz_sim
```

Both `ros_gz` prefixes must point into this workspace installation. A prefix under `/opt/ros/lyrical` means the system package is still selected.

The last command prints an installed package prefix. In every new terminal, load the same environment:

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
```

Continue with [First simulation](quickstart.md). NVIDIA/CUDA sonar has additional [build requirements](sonar-tuning.md).

If the compiler is killed because memory is exhausted, limit compiler jobs as well as package concurrency. `--executor sequential` serializes packages, not compiler processes:

```bash
MAKEFLAGS="-j1" CMAKE_BUILD_PARALLEL_LEVEL=1 colcon build --merge-install --executor sequential --symlink-install
```

## Update a workspace

Record the current revision, pull the desired POSIM revision, then repeat dependency resolution and the build. Keep custom model work committed before updating.

```bash
git -C ~/posim_ws/src/posim rev-parse HEAD
git -C ~/posim_ws/src/posim pull --ff-only
```

## 5. Run the demo world

In the Ubuntu terminal, load the installed environment and open the demo. Gazebo should show the ocean scene; the first run can download Fuel models. Stop it with Ctrl+C before continuing to [Prepare your terminals](terminals.md).

```bash
source /opt/ros/lyrical/setup.bash
source "$HOME/.ros_ardusub_env/env"
source ~/posim_ws/install/setup.bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_ocean_waves headless:=false
```
