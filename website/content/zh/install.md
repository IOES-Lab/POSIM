# Ubuntu：原生安装

在Ubuntu 26.04中编译POSIM。打开Ubuntu的终端应用，按顺序执行各个代码块，等待当前步骤完成后再继续。如果希望使用预先构建的镜像，请阅读[Linux与Docker](install-ubuntu-docker.md)。

## 1. 获取源代码

{{CODE:install:0}}

请保留目录名`posim`，后续导入仓库清单时会使用这个名称。

## 2. 安装软件栈

先阅读`src/posim/extras/ros-lyrical-gz-jetty-install.sh`。脚本会配置软件仓库、使用`sudo`并执行`apt-get full-upgrade -y`，因此推荐使用专用的Ubuntu环境。

{{CODE:install:1}}

脚本安装ROS 2 Lyrical、Gazebo Jetty、ArduSub、MAVROS和波浪库。Wave Sim作为外部依赖，按固定版本获取并编译，不需要递归克隆POSIM。

## 3. 准备工作空间依赖

{{CODE:install:2}}

`--skip-existing`会保留已有的POSIM源代码。编译前请处理所有依赖错误。

清单还会导入固定版本的`ros_gz`，其中包含桥接生命周期和POSIX启动修复。已有的`src/ros_gz`不会被替换：请比较其来源和提交版本是否符合清单，或使用新的工作空间。

## 4. 编译并加载环境

{{CODE:install:3}}

两个`ros_gz`路径都应指向当前工作空间的`install/`目录。如果显示`/opt/ros/lyrical`，说明仍在使用系统包。每次打开新终端，都要按照[准备终端](terminals.md)加载ROS和工作空间。

### 编译时内存不足

`--executor sequential`只让软件包依次编译，不限制单个包内部的编译进程数。可以同时限制两者：

{{CODE:install:5}}

### 更新工作空间

先将自己的修改提交到Git。记录当前版本，获取更新，然后重新安装依赖并编译：

{{CODE:install:6}}

接下来运行[首次仿真](quickstart.md)。声呐请参阅 [CPU/WGPU/CUDA 编译与配置](sonar-tuning.md)。

## 5. 运行演示场景

最后检查Gazebo能否打开海洋场景。首次运行可能需要下载Fuel模型。使用**Ctrl+C**结束仿真，再继续[准备终端](terminals.md)。

{{CODE:install:7}}
