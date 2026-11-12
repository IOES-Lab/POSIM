# Windows：WSL 与 Docker

通过 Docker Desktop 运行 POSIM，再由 WSLg 将 Gazebo 窗口显示在 Windows 桌面上。本页的 Docker 命令均在 **WSL 内的 Ubuntu 终端**中执行。

## 1. 准备 WSL 和显卡驱动

先完成 [Windows 安装指南](install-windows.md)的前两步：在 WSL2 中安装 Ubuntu 26.04，更新 WSL 和 **Windows 侧的显卡驱动**。NVIDIA 显卡也需要这样设置。不要在 WSL 中安装 Linux 显示驱动。

安装[使用 WSL2 后端的 Docker Desktop](https://docs.docker.com/desktop/features/wsl/)，启用 **Settings → Resources → WSL Integration → Ubuntu-26.04**。使用 Docker Desktop 的引擎即可，无需在 Ubuntu 内再安装一个 Docker Engine。

检查服务端和显示套接字：

{{CODE:install-windows-docker:0}}

如果套接字不存在，请先完成 [WSLg 配置](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)。

## 2. 下载 POSIM

{{CODE:install-windows-docker:1}}

镜像版本和源码构建方法见 [Docker 环境](docker.md)。以下配置使用软件渲染。如需使用受支持的 NVIDIA GPU，请先确认 [GPU 访问要求](docker.md#graphics-and-gpu-access)。

## 3. 创建容器

{{CODE:install-windows-docker:2}}

将自己的文件保存在 `/work` 中，该目录对应 `posim-data` 数据卷。使用 `docker stop posim` 停止容器，使用 `docker start posim` 重新启动。

运行后续示例时，打开[两个 Ubuntu 终端](terminals.md)，并将两者连接到同一个容器。Gazebo 会直接显示为 Windows 窗口。

## 4. 启动演示世界

进入容器：

{{CODE:install-windows-docker:3}}

运行以下命令验证安装：

{{CODE:install-windows-docker:4}}

Gazebo 应显示海洋场景。仿真运行期间保持终端打开，按 **Ctrl+C** 停止。接下来可以[启动第一个机器人](quickstart.md)。

请在同一个 POSIM 容器内启动 Linux 版 QGC，不要在 Windows 主机或容器外的 WSL 中启动 QGC。请参阅 [QGC 设置](qgroundcontrol.md)。
