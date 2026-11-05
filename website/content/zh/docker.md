# Docker 环境

POSIM 镜像包含 Ubuntu、ROS 2 Lyrical、Gazebo Jetty、POSIM、ArduSub、MAVROS 和 QGroundControl。请先在[选择运行环境](requirements.md)中找到对应的安装指南，再配置显示和驱动程序。

## 下载已发布的镜像

镜像发布在 [Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags)。Docker 会根据 CPU 自动选择 AMD64 或 ARM64 版本。

{{CODE:docker:0}}

| 标签 | 用途 |
|---|---|
| `latest` | 主分支已发布的构建，包含 AMD64 和 ARM64 版本 |
| `sha-<12-character-commit>` | 指定源码提交；请复制 Docker Hub 上实际存在的标签 |
| SHA 标签后缀 `-amd64` 或 `-arm64` | 明确指定架构 |
| 已发布的版本号 | 对应项目发布标签的版本 |

镜像标签记录了完整的源码提交号。若要使用完全相同的镜像复现实验，还应保存镜像摘要。

{{CODE:docker:1}}

请使用与源码版本对应的文档。下载新镜像不会自动更新已有容器。

## 从源码构建

在宿主机上执行：

{{CODE:docker:2}}

构建 AMD64 镜像：

{{CODE:docker:3}}

构建 ARM64 镜像：

{{CODE:docker:4}}

在对应的安装指南中，用本地标签替换 `ioeslab/posim:latest`。两种镜像都会编译当前源码和固定版本的波浪库。ARM64 镜像还包含 XFCE 桌面和 RDP 服务。

## 按操作系统启动

- [Linux 与 Docker](install-ubuntu-docker.md)：使用 Linux 桌面的显示连接。
- [Windows、WSL 与 Docker](install-windows-docker.md)：通过 WSLg 显示 Linux 窗口。
- [macOS、Docker 与 RDP](install-macos.md)：连接容器内的 Linux 桌面。

这些指南都会创建名为 `posim` 的容器。[准备终端](terminals.md)说明了在哪里输入命令以及如何加载工作空间。

## 保存自己的文件

AMD64 镜像的工作空间位于 `/opt/posim_ws`，ARM64 镜像则位于 `/home/docker/posim_ws`。可通过 `POSIM_WS` 或 `POSIM_UNDERLAY` 查看路径。

将自己的 SDF 文件和录制数据保存在 `/work`，它挂载的是 `posim-data` 数据卷。容器停止后，其他文件也会保留；但删除容器时，这些文件会一起被删除。替换容器前，请将源码修改提交到 Git 或复制到外部。数据卷只有在明确删除时才会消失。

## 停止、恢复和更新 {#update-an-existing-container}

先在仿真终端中按 **Ctrl+C**，然后在宿主机上执行：

{{CODE:docker:5}}

更新时，先保存工作、停止容器并下载所需镜像，再删除已停止的容器。重新执行安装指南中的创建命令，并继续使用 `posim-data`，即可保留 `/work` 中的数据。

## 图形与 GPU {#graphics-and-gpu-access}

Linux 需要配置宿主机驱动和 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)。Windows 请参照 [Docker Desktop 的 WSL2 GPU 配置](https://docs.docker.com/desktop/features/gpu/)。CUDA 声呐还需要镜像内的相关库，详见[声呐配置](sonar-tuning.md)。初次显示验证使用软件渲染，速度可能低于 GPU 渲染。
