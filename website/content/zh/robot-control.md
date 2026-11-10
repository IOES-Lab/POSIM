# 推进器与摇杆控制

选择与机器人对应的控制接口。先确认传感器数据，再启用一个控制器，避免同时使用多个指令源。

| 载具 | 指令路径 | 指南 |
|---|---|---|
| REXROV | ROS 推力指令 → Gazebo 推进器 | [Python](python-control.md) |
| BlueROV2 | ROS／摇杆 → MAVROS → ArduSub | [QGC 与视频](qgroundcontrol.md) · [ROS／摇杆](rovs.md) |
| WAM-V | 水面载具集成与控制器 | [水面机器人](surface.md) |

## REXROV 推进器

在[准备好的终端](terminals.md)中，让 A 运行 REXROV，B 发送指令。配置桥接了八个 `cmd_thrust` 话题，消息类型为 `std_msgs/msg/Float64`，数值表示以牛顿为单位的推力。让第一个推进器回到中立状态：

{{CODE:robot-control:0}}

根据各推进器的位置和轴向分配推力。控制整机速度需要协调多个推进器。实验结束时，向所有使用过的推进器发送零值。

## BlueROV2 与 ArduSub

启动 [BlueROV2 示例](rovs.md#start-bluerov2)，等待自动驾驶仪连接后检查接口：

{{CODE:robot-control:1}}

`volatile` 持久性设置选择新发布的状态，过滤器在连接建立后等待 `connected: true`。如果一直等待，请检查 MAVROS 和 ArduSub 的启动输出及连接设置。

适配器将 `/joy` 和 `/keyboard/joy` 转换为 MAVROS 指令：

- 发布频率：**20 Hz**。
- 输入超时：**0.3 秒**。

只使用一个指令源，并确认解锁和导航状态。键盘与 WebSocket 配置见 [ROV 与 BlueROV2](rovs.md)。
