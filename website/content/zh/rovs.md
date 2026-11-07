# ROV 与 BlueROV2

通过 `namespace` 参数选择已安装模型的目录。ROS 话题由该模型的配置决定。

## 可用模型

| 模型 | 用途 |
|---|---|
| `rexrov` | Gazebo 推进器、水动力学和里程计 |
| `bluerov2` | 集成 ArduSub SITL 与 MAVROS 的 BlueROV2 |
| `bluerov2_heavy` | Heavy 配置 |
| `bluerov2_heavy_multibeam_sonar` | 配备 CUDA 声呐的 Heavy 配置 |

请使用[与安装方式对应的终端](terminals.md)。REXROV 可从[第一次仿真](quickstart.md)开始。

## 启动 BlueROV2 {#start-bluerov2}

{{CODE:rovs:0}}

使用 `namespace:=bluerov2_heavy` 选择 Heavy 型。通过启动输出和 `/mavros/state` 确认自动驾驶仪连接。带声呐的模型需要先[配置 CUDA](sonar-tuning.md)。

## 键盘控制

在桌面的交互式终端中设置 `gui:=true headless:=false use_teleop:=true`。

| 按键 | 操作 |
|---|---|
| `c` / `x` | 解锁／加锁 |
| `w` / `s` | 前进／后退 |
| `a` / `d` | 向左／向右转向 |
| `r` / `f` | 上浮／下潜 |
| `h` / `j` | ALT_HOLD／STABILIZE |
| 空格 | 中立指令 |

## 使用浏览器摇杆

在另一个准备好的终端中启动已安装页面的服务：

{{CODE:rovs:1}}

启用遥操作并启动 BlueROV2：

{{CODE:rovs:2}}

保持 `use_teleop:=true`，因为 WebSocket 桥接包含在遥操作启动流程中。如果浏览器没有自动打开，请在同一个 Linux 桌面中访问该 URL。Docker 镜像包含 Firefox 和 QGC。[QGroundControl](qgroundcontrol.md)介绍其安装、虚拟摇杆和实时视频。

如需从自己的程序订阅数据或发送指令，请继续阅读 [ROS](ros.md)。

## 查看机器人运动

{{MEDIA:rovs}}

图片和视频：POSIM Notion Wiki。DAVE 署名见[引用与许可证](citation.md)。请使用本页给出的命令和话题名称。
