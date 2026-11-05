# 水压

压力插件根据模型深度计算静水压力，发布 ROS `FluidPressure` 消息，也可以输出由该压力推算的深度。

## 查看 REXROV 水压

在[按安装方式准备好的终端 A](terminals.md)中启动：

{{CODE:pressure:0}}

在准备好的终端 B 中读取：

{{CODE:pressure:1}}

压力单位为 **Pa**，方差为 **Pa²**，深度为 **m**。

## 集成插件

将以下内容直接放在待测 `<model>` 下。`namespace` 用于区分输出话题：

{{CODE:pressure:2}}

输出路径为 `/model/<namespace>/<topic>`，深度话题再加 `_depth`。插件测量的是其所属模型，namespace 可以与模型名称不同。

## 参数

| 参数 | 含义与默认值 |
|---|---|
| `namespace` | 必填，必须构成有效的 ROS 话题前缀 |
| `topic` | 输出名称；`sea_pressure` |
| `standard_pressure` | 有限、非负的水面参考压力，kPa；`101.325` |
| `kPa_per_meter` | 有限、正数的静水压力增长率，kPa/m；`9.80638` |
| `estimate_depth_on` | 发布计算深度；`true` |
| `update_rate` | 有限的发布频率，Hz；`0`。非正值表示每次物理时间推进时发布 |
| `saturation` | 接受此值，默认为 `3000`；当前实现不使用它限制压力 |

深度采用世界坐标中的 `max(0, -z)`，嵌套模型包含父模型变换。参考面为世界 z = 0，与波面高度无关。计算使用 kPa，发布前转换为 Pa。

## 时间与有效性

ROS 压力、估计深度和 Gazebo 压力使用同一仿真时间戳。暂停或重复时间戳不会产生新测量；时间继续推进或回退后恢复发布。

配置无效时插件保持停用。位姿缺失、数值非有限或计算溢出时，不输出压力样本。

发布的方差固定为 **9,000,000 Pa²**，并未向压力加入随机噪声。将它用于估计器时请考虑这一点。计算细节见[插件源码](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_sensor_plugins/src/sea_pressure_sensor.cc)。

## 比较两个深度

保持压力增长率不变，在两个已知 z 位置测量。压力差应等于每米增长率乘以深度变化。推算深度使用同一计算，因此不是独立的深度测量。估计器的单位和参考压力应与此一致。

## 压力与深度输出

{{MEDIA:pressure}}

图片来源：POSIM Notion Wiki；DAVE 署名见[引用与许可证](citation.md)。执行时使用本页代码块中的命令和话题名称。
