# Install POSIM from source

These instructions target **Ubuntu 26.04, ROS 2 Lyrical, and Gazebo Jetty**.
For macOS, use the [ARM64 Docker image build](docker.md); this page does not
describe a native macOS installation.

## 1. Check out the source

```bash
mkdir -p ~/posim_ws/src
git clone --branch main https://github.com/IOES-Lab/POSIM.git ~/posim_ws/src/dave
cd ~/posim_ws
git -C src/dave rev-parse HEAD
```

Keep the checkout directory named `dave`: the repository manifest uses this key
so that dependency import can skip the source you already checked out.

## 2. Install the platform dependencies

Review `src/dave/extras/ros-lyrical-gz-jetty-install.sh`, then run it on the target
Ubuntu machine:

```bash
DAVE_EXTRAS_DIR="$PWD/src/dave/extras" \
  bash src/dave/extras/ros-lyrical-gz-jetty-install.sh
```

This helper performs an apt system upgrade, installs ROS/Gazebo and the
ArduSub/MAVROS dependencies, and adds environment setup to the user's shell
configuration. The explicit `DAVE_EXTRAS_DIR` selects the helper files from the
same checkout. It is a retained compatibility variable.

## 3. Import companion repositories and build

In a Bash terminal:

```bash
cd ~/posim_ws
source /opt/ros/lyrical/setup.bash
vcs import src --shallow --skip-existing \
  --input src/dave/extras/repos/posim.lyrical.repos
rosdep update --rosdistro lyrical
rosdep install --rosdistro lyrical --from-paths src --ignore-src -r -y
colcon build --merge-install --executor sequential --symlink-install
source install/setup.bash
```

The manifest tracks the companion `dockwater` and `rocker` repositories on
their `main` branches. For a repeatable experiment, save the resolved revisions:

```bash
vcs export src --exact > resolved.repos
```

CUDA sonar targets are conditional on a CUDA toolkit. Installing the rest of the
workspace without CUDA does not enable those sonar targets. WGPU is not part of
the initial POSIM import.

## 4. Run a first scene

```bash
ros2 launch dave_demos dave_world.launch.py world_name:=dave_ocean_waves
```

For a server-only world:

```bash
ros2 launch dave_demos dave_world.launch.py \
  world_name:=dave_ocean_waves headless:=true
```

For a server-only REXROV run with interactive controls disabled:

```bash
ros2 launch dave_demos dave_robot.launch.py \
  namespace:=rexrov world_name:=dave_ocean_waves z:=-5 paused:=false \
  gui:=false use_teleop:=false use_web_joystick:=false
```

Some sensors still require a rendering context during server-only execution.
First use may also need network access to Gazebo Fuel. See the
[demo guide](../examples/dave_demos/README.md) for other launch entries.
