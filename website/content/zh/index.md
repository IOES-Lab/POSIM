# 概述

POSIM是一套面向ROS 2和Gazebo的开源海洋机器人仿真库。你可以直接运行海洋环境、水面与水下机器人、仿真传感器和控制实验。它源自[Project DAVE](https://field-robotics-lab.github.io/dave.doc/)，后者以[UUV Simulator](https://uuvsimulator.github.io/)为基础。

{{OVERVIEW}}

## 从这里开始

先[选择运行环境](requirements.md)，再按照对应指南安装：[Ubuntu原生环境](install.md)、[Linux上的Docker](install-ubuntu-docker.md)、[Windows上的WSL](install-windows.md)、[WSL中的Docker](install-windows-docker.md)，或[macOS上的Docker与RDP](install-macos.md)。推荐使用Ubuntu原生安装。

安装完成后，[准备终端](terminals.md)，然后运行[第一次仿真](quickstart.md)。

- ROS：**ROS 2 Lyrical** — LTS，支持结束：[2031年5月](https://docs.ros.org/en/lyrical/Releases/Release-Lyrical-Luth.html)
- Gazebo Sim仿真器：**Gazebo Jetty** — LTS，支持结束：[2031年5月](https://gazebosim.org/docs/latest/releases/#release-list)

## 可以仿真什么

[功能与示例](features.md)汇总了各项功能的实现位置和使用指南。

| 任务 | 指南 |
| --- | --- |
| 准备海洋环境与海底 | [世界场景](worlds.md)、[海流](currents.md)、[水下效果](underwater-rendering.md) |
| 选择机器人 | [ROV](rovs.md)、[Slocum](gliders.md)、[水面机器人](surface.md) |
| 查看传感器 | [相机](camera.md)、[GNSS](gnss.md)、[水压](pressure.md)、[DVL](dvl.md)、[USBL](usbl.md)、[声呐](sonar.md) |
| 控制与记录实验 | [ROS](ros.md)、[Python](python-control.md)、[推进器与操纵杆](robot-control.md)、[传感器数据](visualization.md) |

## 仿真的组成

1. 在世界场景的SDF中定义环境和系统。
2. 在模型的SDF中描述连杆、碰撞形状、惯性和传感器。
3. 使用launch文件启动Gazebo、模型和ROS桥接。
4. ROS节点接收测量数据，并发送控制指令。

要设计自己的实验，请继续阅读[创建世界场景](custom-worlds.md)、[添加机器人](custom-robots.md)和[配置传感器](custom-sensors.md)。每个插件的参考页都提供快速上手步骤、SDF片段和参数说明。

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/)将POSIM与用于生成地理地形的POSIM-Terrain、用于路径规划的POSIM-Routing，以及用于机器人运动控制的POSIM-Control相结合。在此基础上，它加入了GUI及Web、桌面应用，让用户通过界面准备场景、操作机器人并查看仿真数据。

下面的视频介绍主要功能。安装和操作方法请参阅[WWW-POSIM文档](https://www-posim.vercel.app/guide/en.html)。

{{INTRO_VIDEO}}

[在YouTube观看介绍视频](https://www.youtube.com/watch?v=_cQtl5JzN2E)。
