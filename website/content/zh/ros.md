# ROS 终端与话题

终端 A 运行仿真，终端 B 查看数据或发送控制指令。两者必须加载相同的 ROS 发行版和 POSIM 工作空间。

## 理解数据流

<div class="docs-flow"><div><strong>ROS 程序与命令</strong><span>订阅、服务与控制输入</span></div><div><strong>ROS 接口</strong><span>ros_gz 桥接或插件直接发布</span></div><div><strong>Gazebo 与 POSIM 插件</strong><span>物理、机器人状态与测量值</span></div></div>

传感器数据发往 ROS 程序，控制指令发往仿真器。Gazebo 话题只有通过桥接或具备 ROS 接口的插件发布后，才能在 ROS 中使用。启动机器人时会加载其桥接配置。

## 准备 A 和 B

根据安装方式完成[终端准备](terminals.md)。保持[第一次仿真](quickstart.md)在 A 中运行，在 B 中执行以下命令。

以下话题适用于未启用声呐的 BlueROV2 水下示例。启用声呐时，将 `bluerov2` 替换为 `bluerov2_heavy_multibeam_sonar`；水面示例则使用 `wamv`。

## 检查通信关系

`--no-daemon` 直接查询 ROS，不依赖命令行守护进程的缓存。

{{CODE:ros:0}}

`ros2 topic list` 列出 ROS 话题，`gz topic -l` 列出 Gazebo Transport 话题。连接程序前，请查看消息类型、发布者、订阅者和 QoS 配置。

## 观察测量值

{{CODE:ros:1}}

机器人位置可能几乎不变，但应持续收到带有新时间戳的数据。在 **B 中按 Ctrl+C** 只会停止查看，A 中的仿真继续运行。使用 `--once` 可接收单条消息。采用仿真时间的节点需要设置 `use_sim_time:=true` 并接收 `/clock`。

## 连接另一台 ROS 计算机

使用兼容的 ROS 和消息包，保持域设置一致，并确保网络允许 DDS 发现与数据通信。使用 POSIM 专用消息时安装 `posim_interfaces`。如果存在发布者却收不到数据，请先查看[故障排查](troubleshooting.md)。

接下来可学习 [Python 控制](python-control.md)、[推进器与摇杆控制](robot-control.md)或[显示和记录数据](visualization.md)。
