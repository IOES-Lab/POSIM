# 多波束声呐

POSIM 使用基于声线的点散射模型，生成强度—距离数据和声呐图像。运行前请先配置 [NVIDIA/CUDA 环境](sonar-tuning.md)。

## 运行示例 {#run-a-sonar-example}

在支持 NVIDIA/CUDA 的安装环境中准备[终端 A 和 B](terminals.md)。在 A 中启动仿真，在 B 中检查输出。

{{CODE:sonar:0}}

`posim_multibeam_sonar_demo` 也提供独立示例：

{{CODE:sonar:1}}

每次只运行其中一个示例。需要在机器人上安装声呐时，选择 `bluerov2_heavy_multibeam_sonar`，并配合适合声呐的世界场景。

## 查看输出

在终端 B 中运行：

{{CODE:sonar:2}}

- 图像发布端采用 `reliable` QoS。RViz 的 Image 显示项也应使用这一可靠性设置。
- 为 `/sensor/multibeam_sonar/point_cloud` 添加 PointCloud2 显示项。
- 原始声学数据采用 `marine_acoustic_msgs/msg/ProjectedSonarImage`。
- BlueView 启动配置将 Gazebo 时钟桥接到 ROS `/clock`。使用 `use_sim_time` 的节点需要仿真保持运行，并且时钟持续推进。

## 配置传感器 {#sensor-configuration}

将以下系统直接放在 `<world>` 中：

{{CODE:sonar:3}}

将 P900 传感器放在模型的 `<link>` 中：

{{CODE:sonar:4}}

此示例设置了 512 个水平波束，覆盖约 130°；300 条垂直声线，覆盖约 12°；距离范围为 0.1–10 m。频率为 900 kHz，带宽为 29.9 kHz，声速为 1500 m/s。30 Hz 是设定的更新频率，实际接收频率取决于计算和渲染开销。完整配置见[传感器模型](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf)。

## `<spec>` 参数

| 参数 | 含义 | 代码默认值 | P900 示例 |
| --- | --- | --- | --- |
| `verticalFOV` | 声学波束的垂直宽度，度 | 10 | 12 |
| `sonarFreq` | 声学频率，Hz | 900000 | 900000 |
| `bandwidth` | 信号带宽，Hz | 29500000 | 29900 |
| `soundSpeed` | 声速，m/s | 1500 | 1500 |
| `sourceLevel` | 声源级，dB | 220 | 220 |
| `maxDistance` | 处理距离，m；应与声线距离范围一致 | 60 | 10 |
| `raySkips` | 垂直采样步长，正整数 | 10 | 10 |
| `sensorGain` | 读取并输出，但不参与图像生成 | 0.02 | 0.02 |
| `blazingSonarImage` | 根据实际时钟生成随机种子；`false` 时固定为 1234 | false | true |
| `pointCloudTopicName` | Gazebo 点云话题后缀 | `point_cloud` | 相同 |
| `sonarImageRawTopicName` | ROS 原始声学数据话题后缀 | `sonar_image_raw` | 相同 |
| `sonarImageTopicName` | ROS 图像话题后缀 | `sonar_image` | 相同 |
| `frameName` | 光学坐标系 ID | `forward_sonar_optical_link` | 相同 |
| `writeLog` | 将 CSV 写入每次运行的独立目录 | false | false |
| `writeFrameInterval` | 两次写入之间的帧数，正整数 | 10 | 5 |
| `debugFlag` | 控制台诊断输出 | false | false |
| `plotScaler` | 读取并输出，但不参与图像生成 | 10 | 未设置 |

### 话题与坐标系

各后缀会追加到传感器的 `<topic>` 后面，本例为 `/sensor/multibeam_sonar`。Gazebo 点云类型为 `gz.msgs.PointCloudPacked`，需要使用随附的 ROS 桥接配置转换为 `sensor_msgs/msg/PointCloud2`。将 `pointCloudTopicName` 与 `sensor_config.py` 中的路径对应，并在 `frameName` 中指定声呐的光学坐标系。

### 声线的几何范围

`<ray>` 的默认值为 256 个水平波束、3 条垂直声线、水平角 ±π/4、垂直角 ±π/8，以及 0.1–5 m 的距离范围。角度默认使用弧度；设置 `degrees="true"` 后使用度。`<gz:multibeam_sonar>` 中的可选项 `<reference_frame>` 默认为位置与旋转均为零的位姿。

请保留完整的 `<ray>` 和 `<spec>` 块：默认值适用于块内字段。声线的距离范围应与 `maxDistance` 一致。参数解析和数据发布方式见[实现代码](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc)。

### 保持实验可比

测量处理性能时应关闭记录功能，除非记录本身也是实验的一部分。按照[声呐配置与调优](sonar-tuning.md)设置实验，并保存声学参数及 `blazingSonarImage` 的取值，以便比较多次运行。

## 添加新的声呐传感器

1. 将 `models/posim_sensor_models/description/blueview_p900/` 复制为新的传感器目录。
2. 准备对应的 `config/<传感器标识>/sensor_config.py`，统一模型、连杆、坐标系名称和桥接话题。
3. 调整采样数、角度和距离范围。在世界中加载上面的两个系统；其他配置可参考[声呐世界 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world)。
4. 准备 CUDA 和渲染环境，重新编译工作空间并加载环境，然后用[启动参数](quickstart.md)选择新传感器。

## 记录和绘制原始数据

设置 `writeLog=true` 后，每个传感器会在仿真器的工作目录下创建独立的 `SonarRawData_<run>_<suffix>/`，并输出其路径。目录内包含带序号的 CSV 和 `SonarRawData_beam_angles.csv`，已有记录会保留。写入间隔小于 1 时会调整为 1；无法创建目录时则关闭记录并报告错误。

进入输出的记录目录，打开第一帧：

{{CODE:sonar:6}}

保存为 PNG：

{{CODE:sonar:7}}

[绘图工具](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py)也可以在没有 CUDA 的环境中安装。它生成一幅声呐图像及五个等间隔选取的波束剖面；波束少于五个时全部绘制。

| 选项 | 作用 |
| --- | --- |
| `--range-max` | 最大显示距离，m |
| `--beam-step` | 每隔 N 个波束取一个 |
| `--dynamic-range` | 低于峰值的显示范围，dB；默认为 60 |

颜色表示相对于当前帧峰值的回波强度；比较不同运行时应使用 CSV 数值。第一列是距离，其余列是各波束的复数回波。请将对应的波束角度文件与数据一同保存，并注意磁盘占用。

## 场景与观测示例

{{MEDIA:sonar}}

图片和视频来自 POSIM Notion Wiki。DAVE 与声呐模型的出处见[引用与许可证](citation.md)。执行时请使用本页代码块中的命令和话题名。
