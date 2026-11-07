# 查看和记录传感器数据

保持仿真在终端 A 中运行，在 B 中检查测量值、打开可视化工具或记录数据。

## 接收相机图像

先在[同一环境中准备两个终端](terminals.md)。在 A 中按 Ctrl+C 停止之前的仿真，再启动下面的场景。Gazebo 会显示在安装时配置的桌面中。

{{CODE:visualization:0}}

在 B 中接收一条图像消息：

{{CODE:visualization:1}}

检查宽度、高度是否非零，以及编码和时间戳是否正确。`--no-arr` 隐藏像素数组。要查看图像，可在 RViz 中添加 **Image** 显示项并选择此话题。衰减参数见[水下相机](camera.md)。

## 选择显示类型

在 RViz 中，相机使用 **Image**，点云使用 **PointCloud2**，坐标系和状态使用 **TF／位姿显示项**。

其他独立的 ROS 客户端要使用 POSIM 专用消息时，也需要安装并加载 `posim_interfaces`。话题和消息类型见[相机](camera.md)、[压力](pressure.md)、[DVL](dvl.md)、[USBL](usbl.md)和[声呐](sonar.md)参考页。[海流](currents.md)和[球面坐标](coordinates.md)还提供了用于改变实验输入的 ROS 服务。

## 记录和回放

此相机独立启动方式不会创建 ROS `/clock` 发布者。要同时录制仿真时间，请在与终端 A、B 相同的 ROS 环境中准备终端 C，并保持以下单向桥接运行。如果 `/clock` 已有发布者，请勿重复启动桥接。

{{CODE:visualization:2}}

返回终端 B，在录制前确认已收到时钟消息：

{{CODE:visualization:3}}

{{CODE:visualization:4}}

按 Ctrl+C 结束记录。每次记录使用新的输出目录，再检查各话题的消息数量：

{{CODE:visualization:5}}

录制后，确认 bag 同时包含图像话题和 `/clock`。回放 bag 前，请在终端 C 中按 Ctrl+C 停止桥接。

在相同话题上回放前，请先停止仿真：

{{CODE:visualization:6}}
