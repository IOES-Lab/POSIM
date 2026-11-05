# 水面机器人与波浪

使用 Wave Sim 波浪和 ArduRover 船舶模式来仿真 WAM-V。POSIM 标准安装程序和 Docker 构建流程会编译外部波浪库。

## 准备 WAM-V

此可选镜像采用 ARM64 桌面布局。在 Apple Silicon Mac 上准备好 [Docker 与 RDP](install-macos.md)，然后进入 POSIM 仓库副本，在 macOS 终端执行：

{{CODE:surface:0}}

通过 RDP 连接 **127.0.0.1:3390**，使用 `docker` 账号。在 Linux 桌面内打开终端：

{{CODE:surface:1}}

镜像包含独立的 ArduSub 和 ArduRover 可执行文件。本例选择 `FRAME_CLASS=2` 的 ArduRover。车辆名称、生成文件和控制器端口固定，因此**每个环境只运行一个实例**。多艇运行需要分开文件、名称、控制器标识和端口，或使用相互隔离的环境。

控制器等待模型生成并收到 IMU 数据。若生成失败或超过 180 秒，或生成后 180 秒内没有 IMU 测量值，启动程序会报告失败阶段并以错误状态退出。重试前先检查 Gazebo 和传感器消息。

## 模型、控制与导航

`extras/surface/wamv.py` 负责配置外部模型。船尾两台螺旋桨由输出 1 和 3 驱动，船从水面开始运行。

外部导航适配器向 MAVROS 发送 **Gazebo 仿真的位姿与速度**。发送任务前，请检查控制器状态、导航数据有效性和推力方向。

要研究接收机噪声、接触水面和重新捕获，请加入[海洋 GNSS](gnss.md)、天线及合适的导航适配器。本例的外部位姿适配器属于另一种导航输入。

## 调整波浪

通过 `/world/<world_name>/waves` 发送 `gz.msgs.Param`。名称来自 SDF 的 `<world name="…">`。先列出当前话题：

{{CODE:surface:2}}

[WAM-V 启动文件](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py)的默认名称是 `posim_surface`，因此话题为 `/world/posim_surface/waves`。在准备好的容器中可以更改名称：

{{CODE:surface:3}}

此时波浪话题为 `/world/harbour/waves`，ROS IMU 仍使用 `/model/wamv/imu`。`world_name` 设置生成世界的名称，不选择地形文件，也不会隔离另一个实例。

本例海底为 z = −10 m 的平面。自定义地形请参考[高度图地形](heightmaps.md)。

| 参数（`double`） | 单位 | 用途 |
|---|---|---|
| `wind_speed` | m/s | 波浪模型的风速 |
| `wind_angle` | 度 | 风向 |
| `steepness` | 无量纲 | 波陡 |

保持视觉与水动力设置一致。[海流](currents.md)单独配置。除了安装库，还必须在世界中加入并配置波浪系统。

## 限制与依赖

此可选 Dockerfile 支持 ARM64 布局。船体、推进和阻力参数需要根据船舶及运行条件标定。退出时，MAVROS 或 Gazebo 可能需要由监控进程终止；再次启动前请确认两者都已退出。

`extras/surface/dependency.json` 固定外部依赖版本。安装程序将库编译到 `/opt/waves`，并把源码保留在 `/opt/asv_wave_sim`。它是构建依赖，不是 Git 子模块。另见[集成指南](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)和[许可证](citation.md)。
