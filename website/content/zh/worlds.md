# 世界场景库

场景文件位于 `models/posim_worlds/worlds`。启动时填写文件名，省略 `.world` 扩展名。

海洋演示场景包含 `model://calm_waves`：Wave Sim FFT 波浪使用 3 m/s 风速输入、256 m 周期网格和 768 × 768 m 显示区域，水面从上下两侧均可见。原生水下效果作用于 RGB 传感器图像；浅水教程也将其应用到 Gazebo 世界视图。标准安装脚本和镜像包含 Wave Sim。水下机器人的浮力、海洋 GNSS 和光学边界使用平均水面 z = 0，独立于显示波高；WAM-V 则使用与波浪耦合的 Wave Sim 水动力。无水的 USBL 示例和独立声呐测试场景保留各自的几何结构。

## 启动场景

无论使用 Ubuntu 原生安装、WSL、Docker 还是 RDP 桌面，都在[对应安装方式的 POSIM 终端](terminals.md)中执行。

{{CODE:worlds:0}}

文件名可能与内部世界名称不同。例如，`posim_ocean_waves.world` 定义的内部名称是 `oceans_waves`；Gazebo 的话题和服务路径使用内部名称。填写路径前，先检查 `<world name="…">`。

## 按实验选择场景

| 实验 | 世界场景 |
|---|---|
| 首次运行机器人 | `posim_ocean_waves` |
| 水下相机图像效果 | `camera_tutorial` |
| DVL 测量 | `dvl_world` |
| USBL 询问与应答 | `usbl_tutorial` |
| 海流配置 | `ocean_current_plugin` |
| 近岸与地理坐标场景 | `posim_Santorini` |
| 声纳 | `posim_multibeam_sonar` 及名称含 sonar 的海洋场景 |
| 操作任务 | `posim_bimanual_example`、`posim_plug_and_socket`、`posim_electrical_mating` |

声纳场景需要相应的[声纳运行环境](sonar-tuning.md)。引用 Fuel 资源的模型可能需要在首次运行时下载。

## SDF 文件

打开文件可查看系统、模型、原点、光源和相机配置。

| 世界 SDF |
|---|
{{WORLD_CATALOG}}

## 组合新场景

将适合的 SDF 复制到 `models/posim_worlds/worlds`，为文件和世界分别设置新名称。使用能够解析到已安装资源的 URI。重新编译并加载工作空间，然后用新文件名启动。另见[高度图地形](heightmaps.md)和[物体模型](objects.md)。

## 等待 Fuel 下载完成

`posim_integrated` 和 `posim_ocean_models` 使用远程模型。等待首次下载完成，加载过程中不要反复启动仿真。设置 `verbose:=true` 可查看资源加载信息。

在同一环境的另一个终端中查询已加载的场景：

{{CODE:worlds:1}}

确认返回结果包含预期模型，仿真时钟持续推进，传感器正常输出测量值。如果下载停滞，请查看加载信息，并检查运行 Gazebo 的计算机或容器的网络连接。

## 查看启动参数

{{CODE:worlds:2}}

| 参数 | 用途 | 传感器启动文件的默认值 |
|---|---|---|
| `namespace` | 已安装模型的目录 | 空，需明确指定 |
| `world_name` | 世界场景文件 | `empty.sdf` |
| `paused` | 以暂停状态启动 | `true` |
| `gui` / `headless` | 显示 / 隐藏 Gazebo 界面 | `true` / `false` |
| `use_sim_time` | ROS 节点使用仿真时钟 | `true` |
| `x`, `y`, `z` | 初始位置，m | 各为 `0.0` |
| `roll`, `pitch`, `yaw` | 初始姿态，rad | 各为 `0.0` |
| `debug` / `verbosity_level` | 详细日志 / 日志级别 | `false` / `1` |
| `use_ned_frame` | 发布 `world_ned` 变换（ENU → NED） | `false` |

要让传感器产生数据，请设置 `paused:=false`。机器人和世界启动文件的参数，请分别用 `--show-args` 查询。
