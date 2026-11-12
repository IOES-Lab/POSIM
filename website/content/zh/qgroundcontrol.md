# 使用 QGroundControl 控制

将 QGroundControl（QGC）连接到模拟的 BlueROV2，通过虚拟摇杆控制并查看前置相机画面。先完成[安装](requirements.md)，再准备[终端 A 和 B](terminals.md)。

## 1. 在 POSIM 所在的 Linux 环境中启动 QGC

**Windows/WSL：**本教程使用 Linux 版 QGC。请勿启动安装在 Windows 上的 QGC。应在运行 POSIM 的同一个 Ubuntu 发行版中启动 QGC；使用 Docker 时，应在同一个 POSIM 容器中启动。WSLg 会将 Linux QGC 的窗口显示在 Windows 桌面上，但程序仍在 Linux 内运行。

本指南中的 `127.0.0.1` 指向 QGC 所在环境的本地地址。该地址并不会将 Windows 版 QGC 配置为接收 WSL 或 Docker 中的遥测和视频。

| 安装方式 | QGC 的运行位置 |
|---|---|
| Ubuntu 原生安装 | 在 Ubuntu 中安装并运行 Linux 版应用。 |
| Windows WSL 原生安装 | 在安装 POSIM 的同一个 Ubuntu 发行版中安装并运行 Linux 版。 |
| Linux 或 Windows Docker | 在 POSIM 容器内运行镜像自带的应用。 |
| macOS Docker 与 RDP | 在 RDP 桌面的终端中运行自带的应用。 |

原生安装请按照 [QGC 官方安装指南](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html)，选择与 CPU 架构对应的 Linux 版本。在 Docker 中，从终端 B 启动：

{{CODE:qgroundcontrol:0}}

末尾的 `&` 让 QGC 保持运行，同时允许继续输入命令。如果从另一个终端启动应用，也要保持该终端打开。首次启动出现偏好设置时，请选择 **Metric System**，使用公制单位。

QGC、MAVROS 和 Gazebo 位于同一个 Linux 环境中。遥测使用 **UDP 14550**，视频使用 **UDP 5600**，因此此配置无需额外发布 Docker 端口。macOS 通过 RDP 访问这个环境。

## 2. 设置虚拟摇杆

打开 **Application Settings → Fly View → Virtual Joystick**：

- 开启 **Virtual Joystick**。
- 开启 **Auto-Center Throttle**，使摇杆松开后控制指令回到中位。
- 关闭 **Left-Handed Mode**，保持与本例相同的摇杆布局。

## 3. 设置视频接收

打开 **Application Settings → Video**。先选择视频源，再按下表填写 UDP 地址和其余设置。

| 设置项 | 值 |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`，自动使用相机画面比例 |
| Disable Video Stream When Disarmed | 关闭 |
| Low Latency Mode | 开启 |
| Auto-reconnect on stream loss | 开启 |

## 4. 启动机器人和视频流

停止之前的仿真，在已准备好的终端 A 中运行：

{{CODE:qgroundcontrol:1}}

Gazebo 打开海底场景，QGC 自动连接 ArduSub。等待初始化完成后，在终端 B 中开启视频发送：

{{CODE:qgroundcontrol:2}}

返回 **Fly View**，点击小视频预览，将相机画面切换到主视图。

{{MEDIA:qgroundcontrol}}

相机按仿真时间以 **5 Hz** 的频率生成 **640 × 480** 图像。软件渲染可能降低实际显示帧率。BlueROV2 具有正浮力，即使未解锁，也可能上浮，使目标移出画面。

`qgc_video:=true` 添加 H.264 发送功能。机器人 RGB 视频包含世界的[水下场景效果](underwater-rendering.md)。独立的[相机滤镜](camera.md)使用自己的 ROS 输出。GNSS 在天线入水后失去定位，出水后重新捕获；遥测、视频和手动控制连接仍保持。水下使用 Manual 模式。镜头低于天线，因此 GNSS 有效时也可能显示水下画面。

## 5. 解锁、操纵和停止

1. 等待状态变为 **Ready**，选择 **Manual** 模式。
2. 打开载具状态菜单，长按 **Arm**，直至解锁操作完成。
3. 短暂向前、向后推动右侧摇杆。
4. 松开摇杆，将指令恢复到中位。水流和惯性仍可能让机器人继续移动。
5. 完成后，在状态菜单中长按 **Disarm** 加锁。

保持键盘控制和网页摇杆关闭，避免多个控制源同时发送指令。在终端 B 中停止视频发送：

{{CODE:qgroundcontrol:3}}

然后在终端 A 中按 **Ctrl+C**，并关闭 QGC。

MAVROS 在最长 60 秒未收到载具心跳后报告连接中断。此设置用于适应较慢的软件渲染，因此真正的心跳中断也可能需要最长 60 秒才会反映在 `/mavros/state` 中。

## 排查连接和画面问题

- **没有载具：** 确认 QGC 与 POSIM 位于同一 Linux 环境中，且没有其他 QGC 实例占用 UDP 14550。
- **一直显示 Waiting for video：** 检查 H.264 视频源、UDP 地址和 `qgc_video:=true`，从终端 B 开启发送后等待几帧。
- **使用其他端口：** 启动时指定 `qgc_video_port:=5602`，并将 QGC 地址设为 `127.0.0.1:5602`。
- **没有 `qgc_video` 参数：** 更新 POSIM，或[拉取镜像后重新创建容器](docker.md#update-an-existing-container)。
- **固件或串口提示：** 仿真使用配套的 ArduSub 和网络连接，无需刷写固件或设置串口。

更多选项见 [QGC 官方视频设置](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html)。ROS 控制接口见[推进器与摇杆控制](robot-control.md)。
