# 海洋 GNSS

`posim_gz_gnss` 在模型的天线连杆处模拟卫星导航接收机。它将世界位置转换为地理坐标，加入可配置的高斯误差，在同一消息中发布位置、速度、时间戳和状态，支持水下失锁、显式遮挡、重新捕获和延迟。

## 构建并安装接收机

在[按安装方式准备好的终端 A](terminals.md)中运行；macOS 使用 RDP 桌面内的 Linux 终端：

{{CODE:gnss:0}}

模型需要按预期位姿连接到机器人上的实体天线连杆，通常名为 `gnss_link`。世界需配置 [WGS 84 坐标](coordinates.md)。在模型 SDF 中添加：

{{CODE:gnss:1}}

按机器人实际情况修改连杆和话题。[创建机器人](custom-robots.md)介绍了模型准备与启动方法。

[配置文件](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/config/profiles.json)提供相对 `base_link` 的安装位置：WAM-V 的 z = 1.8 m，BlueROV2 为 0.3 m，REXROV 为 0.8 m。WAM-V 配置启用接收机，水下机器人配置默认停用。这些是安装配置数据，仍需将天线连杆和插件加入模型。WAM-V 还需要[外部 Wave Sim 模型](surface.md)。

## 参数

| SDF 参数 | 单位或含义 | 默认值 |
|---|---|---|
| `update_rate` | 每仿真秒采样数；大于 0 且不超过 100 Hz | 5 |
| `latency` | 测量交付延迟，仿真秒；0–10 s | 0.2 |
| `reacquire_seconds` | 持续露出水面的捕获时间，s | 3 |
| `dry_margin` | 捕获所需的水面上方净空，m | 0.05 |
| `horizontal_stddev` | 水平位置标准差，m | 0.7 |
| `vertical_stddev` | 垂直位置标准差，m | 1.2 |
| `velocity_stddev` | 速度标准差，m/s | 0.1 |
| `water_level` | 未设置波浪话题时的平均水面 z，m | 0 |
| `seed` | 接收机随机种子 | 42 |
| `enabled` | 启用接收机 | true |
| `link_name` | 模型内的天线连杆 | `gnss_link` |
| `topic` | Gazebo 测量话题 | `/model/<model-name>/gnss` |
| `wave_topic` | 可选的水面高度话题 | 空 |

数值设置必须有限。延迟、捕获时间、净空和标准差必须非负。

## 局部水面高度

`wave_topic` 为空时使用平均水面。设置该话题后，必须收到新鲜的局部水面采样；数据缺失或无效时进入 `surface unavailable`。

`gz.msgs.Double_V` 按顺序包含：模式版本 `1`、网格尺寸、X/Y 周期尺寸、风速和风向角、波陡、仿真时间，以及按行排列的高度值。

- 网格尺寸为 2–256 的整数，高度值数量为该数的平方。
- X/Y 尺寸必须有限且为正。
- 时间戳必须有限，最多比当前时间早 **0.6 仿真秒**，不能来自未来。

兼容的发布端需单独配置。风的元数据不控制接收状态。

## 读取测量与遮挡状态

启动配置好的模型后，在准备好的终端 B 中读取话题：

{{CODE:gnss:2}}

每条消息包含 15 个双精度数，索引从零开始。定位结果延迟到达时，仍应保留其采集时间戳。

| 索引 | 字段与单位 |
|---|---|
| 0–3 | 模式版本 `1`、重置标识 epoch、采集时间（仿真秒）、状态 |
| 4–6 | 纬度（度）、经度（度）、海拔（m） |
| 7–9 | 向东、向北、向上速度，m/s |
| 10–12 | 水平/垂直位置方差（m²）、速度方差（(m/s)²） |
| 13–14 | 天线世界坐标 z 与局部水面高度，m |

状态：`0 disabled`（停用）、`1 submerged`（水下）、`2 acquiring`（捕获中）、`3 fix`（定位有效）、`4 blocked`（遮挡）、`5 surface unavailable`（水面信息不可用）。

无效测量的坐标与速度为 `NaN`。接触水面或被遮挡会清除尚未交付的定位结果。重新捕获要求连续保持所需净空。重置会清除计时器和延迟样本，并递增 epoch。

以下输入模拟遮挡；改用 `false` 可解除：

{{CODE:gnss:3}}

## 解读与检查

比较静止且露出水面的天线、水下天线、被遮挡天线及恢复过程。检查捕获时间、延迟、重置后的 epoch 和递增的测量时间戳。接收端须拒绝过期或未来的定位结果，并处理无效状态。插件输出 Gazebo 消息；需要 ROS `NavSatFix` 或向自动驾驶仪、导航估计器输入时，须通过适配器转换为目标格式。

该模型基于天线位姿，不计算卫星几何、射频传播、物理多径或水声定位。精度配置是仿真输入，并非厂商精度保证。参见[实现](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_gnss/Gnss.cc)及[软件包指南](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_gnss)。
