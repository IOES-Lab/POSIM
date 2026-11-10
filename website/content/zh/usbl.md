# USBL 定位

超短基线（USBL）定位通过声学信号确定水下设备位置。此示例将一个收发器连接到两个应答器，可以询问全部设备，也可以按 ID 选择一个。

## 启动示例世界

在[按安装方式准备好的终端 A](terminals.md)中运行：

{{CODE:usbl:0}}

世界已包含这三个设备。在准备好的终端 B 中接收位置：

{{CODE:usbl:1}}

两种消息均包含 `transponder_id`，但字段含义不同：

| 输出 | x | y | z |
|---|---|---|---|
| `transponder_location` | 水平方位角，度 | 距离，m | 仰角，度 |
| `transponder_location_cartesian` | 相对 x，m | 相对 y，m | 相对 z，m |

方向根据世界坐标位置差计算。水平方位角沿 +x 为 0°，沿 +y 为 90°；正仰角指向 +z。与机器人本体坐标系数据结合时，请考虑这一坐标轴约定。

## SDF 集成

将收发器放在 `box` 模型内。每个 ID 对应 `transponder_attached_object` 列表中相同位置的模型：

{{CODE:usbl:2}}

将以下应答器放在 `sphere` 中，再向 `sphere2` 添加 `transponder_ID` 为 `2` 的应答器。两端的收发器名称、ID 和 namespace 必须一致：

{{CODE:usbl:3}}

## 参数

| 参数 | 设备 | 含义与默认值 |
|---|---|---|
| `namespace` | 两者 | 必填命名空间，例如 `USBL` |
| `transceiver_device`, `transceiver_ID` | 两者 | 组成路径的收发器名称和 ID，必填 |
| `transponder_device`, `transponder_ID` | 两者 | 名称和 ID，必填；收发器可配置逗号分隔的多个 ID |
| `transponder_attached_object` | 收发器 | 必填模型名，逗号分隔且顺序与 ID 一致 |
| `transceiver_model` | 应答器 | 用于获取位置的收发器模型名，必填 |
| `sound_speed` | 两者 | 10 °C、z = 0 时的参考声速，m/s；`1540`，须有限且为正 |
| `enable_ping_scheduler` | 收发器 | 必填布尔值；`false` 使用手动 ping |
| `ping_frequency` | 收发器 | 按仿真时间计的频率，Hz；默认 `1`，示例 `0.5`；有限且为正 |
| `interrogation_mode` | 收发器 | `common` 或 `individual`；默认 `common` |
| `mu`, `sigma` | 应答器 | 位置噪声均值与标准差，m；默认 `0`、`1`，示例 `0`、`0`；有限且 sigma ≥ 0 |

使用已存在的模型和唯一 ID，ID 列表与模型列表必须等长。ID 是 **0–2147483647** 的十进制整数，不带符号或多余前导零。每个插件都需要所属模型包含 link。[完整 SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world)提供几何与布局。

配置无效时插件保持停用。未知通道和命令目标会被忽略；未知通道不会改变当前选择。初始通道为列表中的第一个应答器。定时 ping 随仿真暂停，每次更新最多发送一次。

## 声传播时间

每次询问有两个传播阶段：请求到达应答器，以及位置回复返回收发器。各阶段延迟等于距离除以对应设备的声速。

- 小于一秒的延迟也随仿真时间推进，随世界暂停；时间回退时会被取消。
- 每个阶段、每个应答器仅保留一个待处理样本；额外请求或回复会被忽略。
- 若希望每次询问都有回复，ping 间隔应长于预计往返时间。

有效声速为 `sound_speed + 0.017 × depth + 4 × (temperature − 10)`。单位为 m/s，深度是 z = 0 以下的米数，温度为 °C。水面上方的深度项为零，默认温度为 10 °C。

在 `/USBL/transceiver_manufacturer_168/temperature` 或 `/USBL/transponder_manufacturer_1/temperature` 发布 `std_msgs/msg/Float64` 可修改温度，对后续传播阶段生效。非有限温度，以及导致声速非正或非有限的修改会被忽略。

此近似参考 [DOSITS 声速说明](https://dosits.org/decision-makers/tutorials/science/sound-speed/)。传播采用直线距离，不计算折射、盐度、多径或声学干扰。返回阶段使用收到的位置，包括已配置的位置噪声。

## 询问全部应答器

{{CODE:usbl:4}}

`common` 模式每次定时 ping 广播一次，可在同一话题返回多个 ID。保留每条回复的 ID，以区分来源。收发器保留当前模式，供稍后加入的应答器接收。

## 选择一个应答器

{{CODE:usbl:5}}

切换通道会进入 `individual` 模式，定时 ping 只发往该 ID。需要询问全部设备时改回 `common`。

## 布局与询问模式

{{MEDIA:usbl}}

图片来源：POSIM Notion Wiki；DAVE 署名见[引用与许可证](citation.md)。执行时使用本页命令和话题名称。
