# 故障排查

先查看启动日志中的第一个错误，再依次检查环境、模型资源、仿真时间、插件和数据接收。保留该错误及其前后与复现有关的日志即可。

## 找不到软件包

{{CODE:troubleshooting:0}}

结果应显示 `lyrical` 和所选工作空间的路径。按照[安装方式](requirements.md)准备[终端环境](terminals.md)，并区分主机路径与容器内路径。

## 缺少模型或网格

找到第一个无法解析的 URI，检查软件包资源是否已安装、是否加载了正确的环境。Gazebo 访问 Fuel 资源时需要联网。首次下载可能较慢，请先确认下载进度，再决定是否重新启动仿真。

## 没有图像或渲染初始化失败

检查[安装指南](requirements.md)中的显卡驱动和显示连接。Gazebo 与图像传感器需要正常的渲染器。声呐还需检查[所选后端和已安装的包](sonar-tuning.md)。

对于[水下场景效果](underwater-rendering.md)，检查：

- `/posim/appearance/<session_nonce>/state` 服务、渲染器和着色器路径，以及活动相机数量。
- 渲染后已应用的修订号和就绪状态。
- 是否显式启用了悬浮颗粒；默认情况下它们处于关闭状态。仅运行相机衰减示例不会加载场景效果系统。

## GNSS 无法获得有效定位

查看 [GNSS 有效性状态](gnss.md)、天线的世界坐标高度、局部水面高度和采集时间戳。确认世界地理坐标设置正确，天线位于水面以上且未被遮挡，并留出重新捕获和延迟所需的仿真时间。

如果配置了波浪话题，应确认收到兼容且及时的局部水面高度数据。安装接收机本身不会将它添加到模型，也不会提供 ROS 或自动驾驶仪适配器。

## 收不到消息

先确认仿真时间在推进，再查看话题及类型：

{{CODE:troubleshooting:1}}

使用正确的通信系统，并在同一套已加载环境中检查数据。核对 QoS、域设置、桥接和插件错误。启动传感器时，`namespace` 必须对应已有的描述文件。[USBL](usbl.md) 已内置于示例世界，应通过世界启动命令运行。

## 图像话题可见，但帧数据没有到达 {#image-topics-appear-but-frames-stall}

如果 Gazebo 已生成图像，ROS 也能发现发布者，但只能收到少量帧或完全收不到，应检查 DDS 数据传输。Cyclone DDS 的大 UDP 数据报可能受到 IP 分片限制。对于 MTU 为 1500 字节的网络，减小数据报可能改善传输。参见 [Cyclone DDS 分片配置](https://cyclonedds.io/docs/cyclonedds/latest/config/data-path-config.html#fragmentation)。

### 准备配置文件

在已安装 `rmw_cyclonedds_cpp` 的环境中创建一次以下配置。如果已有 Cyclone DDS 配置，请将这两个大小参数合并到其 `General` 节点，保留原有的发现机制和网络接口设置。

{{CODE:troubleshooting:2}}

### 使用相同配置重新启动

停止仿真器和查看工具。在每个终端中加载 POSIM 环境，然后选择配置文件，再重新启动程序。如果修改的是已有配置，请使用它的路径。

{{CODE:troubleshooting:3}}

如果同一域中已运行 ROS CLI 的后台进程，先将它停止一次，使后续命令采用新配置：

{{CODE:troubleshooting:4}}

重新进行[图像与接收频率检查](camera.md#run-the-example)，同时检查采集时间戳。小数据包会增加额外开销；MTU 更小的网络需要进一步减小这些参数。

## 机器人不移动

检查控制器状态、解锁状态、导航数据有效性、指令接收和所选模式。核对关节名与推进器轴向，并只保留一个活动指令来源。输入回中、世界暂停或自动驾驶仪处于上锁状态，都可能使机器人保持静止。

## 仿真无法结束

先停止查看工具，再在启动仿真的终端按一次 Ctrl+C，等待子进程结束。保留与异常退出、段错误或强制终止有关的日志。[报告问题](https://github.com/IOES-Lab/POSIM/issues)时，请提供命令、修订版本、操作系统、架构和渲染器。

### 选择 Discard 后 RViz 崩溃

POSIM 的 Ubuntu/WSL 安装程序和 AMD64/ARM64 Docker 镜像包含适用于 RViz 15.2.6 的[独立 RViz 覆盖层](https://github.com/IOES-Lab/POSIM/tree/main/extras/rviz)。它在确认关闭后停止更新定时器，并在退出过程中释放视图拥有的 ROS 资源；选择 **Cancel** 时仍会继续更新。系统 RViz 软件包保持不变。加载 POSIM 环境后，`ros2 pkg prefix rviz_common` 应输出 `/opt/posim/rviz-close/install`。较早安装的环境可按链接中的步骤构建并启用覆盖层。

在未启用覆盖层的 Ubuntu 26.04 环境中，RViz 15.2.6 即使能正常显示传感器数据，也可能在关闭窗口并选择 **Discard** 时偶发崩溃。覆盖层修正了特定的定时器和资源清理问题，并不涵盖所有渲染器或退出路径。若问题仍然出现，请在报告中注明 RViz 版本、已加载的库路径及关闭窗口时的操作。

如果 RViz 在窗口关闭后仍继续运行，可能会占用过多内存，使用软件渲染时尤其需要注意。重新运行前，在启动仿真的终端按 Ctrl+C，停止剩余进程。如果 RViz 未能正常退出，ROS launch 会强制终止它。运行声呐时若不需要查看器，可在 `multibeam_sonar_demo.launch.py` 命令中添加 `rviz:=false`。ROS 图像输出仍然可用。

## 在 macOS 原生 SDK 中重新创建深度相机

对于 **macOS arm64 / Ogre-Next 2.3.3**，[独立 Ogre 环境](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md)提供了深度相机销毁和重建时所需的材质生命周期修正。

按照指定步骤核对 SDK 和源码版本，使用构建工具编译，再将 Gazebo 命令传给 `run_overlay.py`。仅完成编译不会自动选择该库。检查实际加载的库路径，并保留验证结果。标准安装器不会启用此环境；它不会替换系统库，也不会配置 Linux 容器。其他版本或平台需要各自兼容的配置。

## 提供可复现的报告

请附上源码 SHA 或镜像摘要、依赖版本、命令和参数、第一个错误、预期行为、相关话题类型，以及最小复现示例。说明话题只是能被发现，还是确实能收到数据。

## MAVROS 2.16.0 因 SIGSEGV 退出

POSIM 安装脚本和官方 Docker 构建默认包含 MAVROS 2.16.0 的
[Router 生命周期修复](https://github.com/IOES-Lab/POSIM/tree/main/extras/mavros)。
加载 POSIM 环境后，运行 `ros2 pkg prefix mavros` 应输出
`/opt/posim/mavros-router/install`。已有安装可按链接中的单独编译步骤应用修复。
如果仍然崩溃，请在问题报告中附上 MAVROS 日志和实际加载的库文件路径。

## POSIM 在 WSL 中运行，但 QGC 在 Windows 中启动 {#qgc-on-windows-host}

关闭 Windows 版 QGC，然后在运行 POSIM 的同一个 Ubuntu 发行版或 POSIM 容器内启动 Linux 版 QGC。WSLg 窗口虽然显示在 Windows 桌面上，程序仍是 Linux 进程。[QGC 设置](qgroundcontrol.md)让遥测和视频在同一环境内部传输。
