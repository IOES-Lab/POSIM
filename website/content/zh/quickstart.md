# 第一次仿真

在海洋场景中启动一台 REXROV，接收实时数据，然后结束仿真。首次运行请保留示例中的机器人和世界设置。

## 准备终端

完成[安装](requirements.md)，再打开[两个已加载 ROS 和 POSIM 环境的终端](terminals.md)。终端 A 运行仿真，终端 B 查看数据。使用 Docker 时，Gazebo 会显示在已配置的 Linux 桌面、WSLg 或 RDP 中。

## 1. 启动机器人和世界

在**终端 A** 中运行：

{{CODE:quickstart:0}}

REXROV 被放置在 z = 0 下方 5 米处，并启动相应的 ROS 桥接。此命令也会启动世界，因此只需运行这一套仿真，并保持 A 打开。

## 2. 读取位置

在**终端 B** 中运行：

{{CODE:quickstart:1}}

位置数据应持续到达。机器人逐渐稳定时，数值变化很小也可能是正常现象。在 **B 中按 Ctrl+C** 停止查看。

## 3. 检查仿真时间

{{CODE:quickstart:2}}

时间应持续前进。按 Ctrl+C 停止查看。如果时间不动或没有消息，请参考[故障排查](troubleshooting.md)。

## 4. 结束仿真

在 **A 中按 Ctrl+C**，等待各进程退出。关闭话题查看程序不会停止仿真。运行其他示例前，请先结束当前仿真。

## 接下来

- [查看和记录传感器数据](visualization.md)：显示图像并保存观测数据。
- [使用 Python 控制](python-control.md)：读取状态并驱动一个推进器。
- [ROV 与 BlueROV2](rovs.md)：使用 ArduSub、键盘和摇杆。
- [世界库](worlds.md)：选择其他环境。
