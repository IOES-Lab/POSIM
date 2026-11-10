# Windows：在 WSL 中安装

在 Windows Subsystem for Linux 2（WSL2）的 Ubuntu 26.04 中安装 POSIM。WSLg 会把 Gazebo 窗口显示在 Windows 桌面上。如果希望直接使用预构建镜像，请选择 [WSL + Docker](install-windows-docker.md)。

## 1. 安装 WSL 和 Ubuntu

以管理员身份打开 PowerShell：

{{CODE:install-windows:0}}

按提示重启 Windows。打开 Ubuntu，设置 Linux 用户名和密码。随后在 PowerShell 中确认该发行版使用 WSL 2：

{{CODE:install-windows:1}}

已有 WSL 或安装遇到问题时，请参考 [Microsoft 安装指南](https://learn.microsoft.com/zh-cn/windows/wsl/install)。

## 2. 准备显卡驱动和 WSLg

安装或更新 Intel、AMD 或 NVIDIA GPU 的 **Windows 显卡驱动**。NVIDIA 用户同样需要这一步。WSLg 借助 Windows 驱动提供图形加速，具体操作见 [GUI 应用指南](https://learn.microsoft.com/zh-cn/windows/wsl/tutorials/gui-apps)。

NVIDIA 用户请按照 [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) 安装 Windows 驱动，**不要在 WSL 内安装 Linux 显示驱动**。只有编译声呐的 CUDA 后端时才需要 CUDA 工具包。

在 PowerShell 中更新 WSL：

{{CODE:install-windows:2}}

如果需要重启，请先保存各个 WSL 终端中的工作，完成后重新打开 Ubuntu。

## 3. 在 Ubuntu 终端安装 POSIM

Linux 命令应在 Windows Terminal 的 **Ubuntu** 配置文件中执行。先确认发行版：

{{CODE:install-windows:3}}

按照 [Ubuntu 原生安装](install.md) 完成获取源码、安装依赖和编译。工作空间放在 Linux 主目录下的 `~/posim_ws`。安装脚本使用 `sudo`，并更新 Ubuntu 中的软件包。

如果能看到图像话题却收不到图像，请查看[故障排查](troubleshooting.md)。后续示例使用的两个终端在[准备终端](terminals.md)中有详细说明。

## 4. 运行演示世界

在 Ubuntu 终端中执行以下命令。Windows 桌面应出现 Gazebo 窗口，显示海洋场景。首次运行可能需要下载 Fuel 模型。开始其他示例前，请按 Ctrl+C 停止当前仿真。

{{CODE:install-windows:4}}
