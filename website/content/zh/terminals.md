# 准备终端

**终端**是输入命令、查看运行结果的窗口。本指南使用两个普通终端窗口：**终端 A** 运行仿真，**终端 B** 查看传感器数据或发送控制指令。A、B 只是便于区分用途的名称，并不是不同的软件。

## 在哪里打开这两个窗口

| 安装方式 | 终端 A 和 B |
|---|---|
| [Ubuntu 原生安装](install.md) | 在 Ubuntu 中打开两个终端窗口 |
| [Linux 上的 Docker](install-ubuntu-docker.md) | 打开两个 Linux 终端，从两边进入同一个容器 |
| [Windows 上的 WSL](install-windows.md) | 在 Windows Terminal 中打开同一 WSL 发行版的两个 Ubuntu 标签页 |
| [Windows 上的 WSL + Docker](install-windows-docker.md) | 打开两个 Ubuntu 标签页，从两边进入同一个容器 |
| [macOS 上的 Docker + RDP](install-macos.md) | 连接 RDP 后，在 Linux 桌面内打开两个终端 |

Linux 和 Windows 的 Docker 安装使用名为 `posim` 的容器。在宿主机的两个终端中分别运行：

{{CODE:terminals:0}}

macOS 使用的 RDP 桌面已经位于容器内，ROS 命令应在该桌面的 Linux 终端中执行。PowerShell 和 macOS 终端用于启动容器等宿主机操作。

## 什么是工作空间

**ROS 工作空间**用于组织源代码和编译后的软件包。`src/` 存放源码，`build/` 存放编译中间文件，`install/` 存放供 ROS 加载的软件包。

| 安装方式 | 路径 |
|---|---|
| Ubuntu 或 WSL 原生安装 | `~/posim_ws` |
| AMD64 Docker 镜像 | `/opt/posim_ws` |
| ARM64 RDP 镜像 | `/home/docker/posim_ws` |

路径中的 `~` 表示当前 Linux 用户的主目录。下面的 `POSIM_WS` 变量用于保存工作空间路径，后续示例会沿用它。

## 在 A、B 中分别加载环境

**Ubuntu 或 WSL 原生安装：**

{{CODE:terminals:1}}

**Docker，包括通过 RDP 打开的 Linux 桌面：**

{{CODE:terminals:2}}

`source` 用于加载路径和设置，让当前终端能够找到 ROS 和 POSIM。每次打开新终端后，都需要执行相应命令。

## 启动前检查

{{CODE:terminals:3}}

两个路径都应位于工作空间的 `install/` 目录中。如果找不到软件包，请回到安装指南检查。如果 `ros_gz_sim` 指向 `/opt/ros/lyrical`，说明当前终端尚未加载工作空间中的版本。

接下来运行[第一次仿真](quickstart.md)。持续输出消息的命令仍在运行；请在**运行它的终端中按 Ctrl+C** 停止，等待命令提示符重新出现，再输入下一条命令。
