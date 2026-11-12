# 第一次仿真

先用 BlueROV2 观察浅水检查场景，再用 WAM-V 观察浮标航道。每个示例都会打开 Gazebo 和预先配置的 RViz。一次只运行一个仿真。

## 开始之前

按照[运行环境](requirements.md)指南完成安装，在[终端 A 和 B](terminals.md)中加载相同的 ROS/POSIM 工作空间。A 运行仿真，B 查看数据并启动视频传输。Docker 用户使用安装时配置的 Linux 桌面、WSLg 或 RDP。两个示例都包含在标准 POSIM 安装中，无需单独构建水面仿真镜像。

## BlueROV2 水下检查

在**终端 A**运行：

{{CODE:quickstart:0}}

BlueROV2 从平均水面以下约 3.1 m 处开始，海底深度为 4 m，前方有箱体、管道、立柱和球体。场景启用了小幅波浪、水下雾化、吸收、焦散和海洋雪。示例调整了压载，使未解锁的机器人留在物体附近。Gazebo 在场景旁显示实时相机画面。

要使用多波束声呐，在上述命令后添加 `sonar:=true`。它会选择配备声呐的 BlueROV2 Heavy，在 Gazebo 中增加声呐图像面板，并在 RViz 中显示声呐图像与点云。声呐计算量较大，**可能降低帧率和控制响应速度**。`auto` 根据主机与传感器配置，依次尝试 CUDA、WGPU、CPU。渲染保持通常的默认设置。后端要求见[声呐配置与调优](sonar-tuning.md)。

以下截图来自启用声呐的示例。RViz 自动加载机器人模型、相机和传感器显示配置。1 m 网格是机器人坐标系中的距离参考，不是测得的海底。默认选择 **Move Camera**：拖动旋转视角，中键平移，滚轮缩放。显示用的螺旋桨保持静止角度，仿真动力学不变。

{{MEDIA:quickstart:0}}

### 检查实时数据

在**终端 B**运行：

{{CODE:quickstart:1}}

启用声呐时，将话题中的 `bluerov2` 替换为 `bluerov2_heavy_multibeam_sonar`。即使机器人几乎静止，位置样本也应持续到达。按 **Ctrl+C** 停止查看，再检查仿真时间是否前进：

{{CODE:quickstart:2}}

按 Ctrl+C 退出查看程序。若没有消息或时钟暂停，请参阅[故障排除](troubleshooting.md)。

### 用 QGC 查看视频并控制

按照 [QGC 设置](qgroundcontrol.md)的第 1–3 步，在相同 Linux 环境中打开 QGroundControl 并启用虚拟摇杆。视频源选择 **UDP h.264 Video Stream**，地址为 **127.0.0.1:5600**，关闭 **Disable Video Stream When Disarmed**。继续使用正在运行的教程场景，在终端 B 启动视频传输：

{{CODE:quickstart:3}}

点击 QGC 的小视频面板可将其放大。海洋 GNSS 天线位于水下时，显示 **No GPS Lock** 属于正常现象；遥测、视频和 Manual 控制仍然可用。等待 **Ready**，选择 **Manual**，解锁后短暂推动摇杆，松开回中，再上锁。详见 [QGC 操作步骤](qgroundcontrol.md)。不要同时启动其他键盘或摇杆控制器。

{{MEDIA:quickstart:1}}

## WAM-V 水面感知

先结束水下仿真，再在**终端 A**运行：

{{CODE:quickstart:4}}

WAM-V 在小幅波浪中面向四个固定浮标。RViz 打开相机画面，并在同一视图显示船体和激光雷达点云，预设 1 m 参考网格与 Move Camera。激光雷达进行水平扫描，点云表示传感器高度处的浮标轮廓，不是完整的三维重建。

在 Gazebo 的 **Lidar** 面板点击刷新并选择 `/tutorial/lidar`。关闭 **Show Non Hitting Rays**，即可只显示命中物体的射线。相机面板仍位于场景旁。

{{MEDIA:quickstart:2}}

相机使用与水下示例相同的 `/tutorial/camera` 话题、视频启动命令和 QGC 视频设置。WAM-V 使用 ArduRover 船舶模式，以仿真位置和速度作为外部导航输入，详见[水面机器人与波浪](surface.md)。该输入独立于 BlueROV2 的海洋 GNSS 接收器。

在 QGC Manual 模式下，用左摇杆的上下方向控制 WAM-V 油门。解锁后短暂推动摇杆，松开回中，再上锁。

{{MEDIA:quickstart:3}}

在**终端 B**读取一次激光雷达扫描：

{{CODE:quickstart:5}}

点云话题为 `/tutorial/lidar/points`，船体里程计话题为 `/model/wamv/odometry`。

## 结束仿真

将机器人上锁，关闭 RViz 和 QGC，再在**终端 A 按 Ctrl+C**，等待启动的进程退出。RViz 若询问是否保存视图修改，选择放弃以保留提供的预设。若无法正常退出，请参阅[查看器故障排除](troubleshooting.md)。关闭话题查看程序不会停止仿真。运行其他示例前，请先结束当前会话。

## 继续学习

- [显示与记录传感器数据](visualization.md)：查看图像并保存测量值。
- [用 Python 控制](python-control.md)：启动独立的 REXROV 示例，测试一个推进器。
- [ROV 与 BlueROV2](rovs.md)：使用 ArduSub、键盘或本地摇杆。
- [世界场景库](worlds.md)：选择环境并查看启动参数。
