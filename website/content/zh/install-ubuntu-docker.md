# Linux：使用 Docker 安装

如果桌面系统使用其他 Ubuntu 版本或其他 Linux 发行版，可以直接使用已发布的 POSIM 镜像。本指南面向 **x86-64 桌面上的 AMD64 镜像**，通过 X11 或 XWayland 显示 Gazebo。Ubuntu 26.04 用户建议选择[原生安装](install.md)。

## 1. 准备 Docker 和显示环境

按发行版安装 [Docker Engine](https://docs.docker.com/engine/install/)。Ubuntu 用户可使用[专门的安装说明](https://docs.docker.com/engine/install/ubuntu/)。如果当前账户无法运行 `docker`，请完成[安装后的配置](https://docs.docker.com/engine/install/linux-postinstall/)。

在当前桌面会话中打开终端并安装 `xauth`。Ubuntu 命令如下：

{{CODE:install-ubuntu-docker:0}}

检查显示地址和 Docker 服务端版本：

{{CODE:install-ubuntu-docker:1}}

## 2. 下载 POSIM

{{CODE:install-ubuntu-docker:2}}

[Docker 环境](docker.md)介绍了版本选择和源码构建。首次运行采用软件渲染，速度可能低于使用 GPU 的原生安装。

## 3. 授权容器访问显示服务

将当前会话的 X11 认证信息复制到仅当前账户可读的文件：

{{CODE:install-ubuntu-docker:3}}

最后一条命令检查文件是否包含数据。如果失败，请确认终端属于当前图形会话，且 XWayland 可用。完成认证配置后再继续。

## 4. 创建容器

{{CODE:install-ubuntu-docker:4}}

将自己的模型和录制数据保存在 `/work` 的 `posim-data` 数据卷中。使用 `docker stop posim` 停止容器，使用 `docker start posim` 恢复。如果重新登录 Linux 后无法显示窗口，请更新认证文件。

运行示例前，先[准备两个终端](terminals.md)，并让它们连接同一个容器。启用硬件加速或 CUDA 声呐之前，请阅读 [GPU 配置](docker.md#graphics-and-gpu-access)。

## 5. 运行演示世界

进入容器：

{{CODE:install-ubuntu-docker:5}}

在容器内完成最后的安装验证：

{{CODE:install-ubuntu-docker:6}}

Gazebo 应打开海洋世界。运行期间请保持终端打开，按 **Ctrl+C** 停止。然后继续[第一次仿真](quickstart.md)。
