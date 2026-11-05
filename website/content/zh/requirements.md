# 选择运行环境

根据电脑现有的操作系统选择安装方式。**推荐在Ubuntu 26.04中原生安装**，便于直接使用Gazebo图形界面、ROS工具和NVIDIA/CUDA设备。

## 安装方式

| 操作系统 | 安装方法 | 安装指南 |
| --- | --- | --- |
| Ubuntu 26.04 | 直接编译并运行POSIM | [Ubuntu原生安装](install.md) |
| Ubuntu或其他受Docker支持的Linux发行版，x86-64 | 运行基于Ubuntu的POSIM镜像，在Linux桌面显示窗口 | [Linux与Docker](install-ubuntu-docker.md) |
| Windows 10/11，x86-64 | 在WSL2的Ubuntu 26.04中编译，通过WSLg显示窗口 | [Windows与WSL](install-windows.md) |
| Windows 10/11，x86-64 | 使用Docker Desktop的WSL2后端运行镜像，通过WSLg显示窗口 | [WSL与Docker](install-windows-docker.md) |
| Apple Silicon版macOS | 运行ARM64 Docker镜像，通过RDP连接Linux桌面 | [macOS与Docker、RDP](install-macos.md) |

Docker镜像包含自己的Ubuntu用户空间。Linux宿主机可以使用其他受Docker支持的版本或发行版。本文不涵盖在Intel Mac上模拟运行ARM64镜像的情况。

## 软件与图形环境

所有安装方式均使用**ROS 2 Lyrical**和**Gazebo Sim — Gazebo Jetty**。原生安装将它们配置在Ubuntu中；Docker镜像已经包含这些软件。图形驱动应按照相应指南安装在宿主机上。

Gazebo以及彩色、深度相机都需要正常工作的渲染环境。先从一台机器人及其默认传感器开始。[CUDA多波束声呐](sonar-tuning.md)还需要兼容的NVIDIA GPU、CUDA工具和声呐库。macOS上的Docker不会将Apple Metal作为Linux NVIDIA/CUDA设备提供给容器。

## 资源与下载

资源需求取决于机器人数量、地形细节、图像分辨率和传感器更新频率。提高这些设置前，请观察内存、GPU负载和实时因子。为源代码、编译文件、Docker镜像、Fuel模型和记录数据预留磁盘空间。

安装需要访问ROS/Gazebo软件仓库、GitHub和Docker Hub。部分世界场景首次运行时会下载Fuel模型。确认演示场景正常运行后，继续[准备终端](terminals.md)。
