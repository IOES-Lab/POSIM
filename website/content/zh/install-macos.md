# macOS：Docker 与 RDP

在搭载 Apple Silicon 的 Mac 上运行 POSIM 的 ARM64 镜像，再通过 RDP 连接其 Linux 桌面。Gazebo 和示例所需的终端都在这个桌面中运行。

## 1. 准备 Docker 和 RDP 客户端

安装并启动 [Docker Desktop for Mac](https://docs.docker.com/desktop/setup/install/mac-install/)。同时安装 [macOS 版 Windows App](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps)，用于连接远程桌面。

在 macOS 的“终端”中检查：

{{CODE:install-macos:0}}

Apple Silicon 应显示 `arm64`。第二条命令应显示 Docker 服务端版本。

## 2. 下载镜像并创建容器

Docker 会自动选择 ARM64 镜像：

{{CODE:install-macos:1}}

RDP 端口只对本机开放。将自己的模型和录制数据保存在 `/work`，即可通过 `posim-data` 数据卷保留它们。镜像版本和源码构建方法见 [Docker 环境](docker.md)。

## 3. 连接 Linux 桌面

在 Windows App 中添加地址为 **127.0.0.1:3389** 的电脑。在 xrdp 登录界面选择 **Xorg**，使用用户名 **docker**、密码 **docker** 登录。

在 Linux 桌面中打开终端，修改这个本地开发账户的密码：

{{CODE:install-macos:2}}

POSIM 命令在这个 Linux 桌面中运行。macOS 终端用于管理 Docker。[准备终端](terminals.md)还说明了工作空间所在的位置。

## 4. 停止和重新连接

关闭 RDP 窗口不会停止仿真。请先在运行仿真的终端中按 **Ctrl+C**，然后在 macOS 中执行：

{{CODE:install-macos:3}}

启动容器后，用 Windows App 重新连接。Linux 使用软件渲染，复杂场景可能较慢。声呐请在[编译并检查相关包](sonar-tuning.md#build-the-sonar-packages)后从 CPU 开始。Metal 可用于原生 macOS 进程，不提供给此 Linux 容器。也可先运行[相机和压力传感器示例](features.md)进行检查。

## 5. 运行演示世界

在 **RDP 桌面内的终端**中完成最后的安装验证：

{{CODE:install-macos:4}}

Gazebo 应显示海洋世界。仿真运行期间请保持终端打开。按 **Ctrl+C** 停止后，继续[第一次机器人仿真](quickstart.md)。

{{MEDIA:install-macos}}
