# 添加机器人

从结构相近的 POSIM 机器人开始。SDF 描述机械结构与系统，ROS 配置连接测量输出和控制指令。

## 工作环境

使用 Ubuntu 原生安装、WSL 或 Docker 中[准备好的 Linux 终端](terminals.md)。macOS 用户在 RDP 桌面的终端中操作。编辑和编译都在该环境内完成；`POSIM_WS` 是其工作空间路径。

## 1. 复制参考模型

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) 使用 Gazebo 推进器。[BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) 集成 ArduSub/MAVROS。[Slocum](gliders.md)展示另一种机器人结构。

在 `$POSIM_WS` 中完整复制 REXROV，保留配套网格、桥接配置和话题名称：

{{CODE:custom-robots:0}}

将以下程序保存为 `copy_robot.py`，执行 `python3 copy_robot.py`。若 `my_robot` 已存在，程序会停止。

{{CODE:custom-robots:1}}

生成的目录结构包括：

{{CODE:custom-robots:2}}

`my_robot` 是你选择的标识符。启动文件会查找 `description/<namespace>/model.sdf` 及其对应配置。

## 2. 定义机械结构与资源

首次启动先保留复制来的机械设置。修改船体时，应一起检查：

| SDF 路径 | 单位 | 检查内容 |
|---|---|---|
| `model/@name`、连杆名和关节名 | 标识符 | 与插件和 ROS 配置中的名称一致 |
| `link/inertial/mass` | kg | 每个运动连杆的质量为正 |
| `link/inertial/pose` | m, rad | 质心和惯性坐标系方向 |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | 绕惯性坐标系各轴的转动惯量 |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | 非对角项；完整惯性张量应符合物理约束 |
| `link/visual`, `link/collision` | m, rad | 网格比例与几何对齐 |
| `joint/axis`、推进器设置 | 依插件而定 | 执行器方向与推力分配 |

STL 只提供几何形状。质量和动力学应在 SDF 中定义，并核对网格单位与比例。沿用已有示例的安装资源 URI。如果视觉模型细节使接触计算过于昂贵，可使用更简单的碰撞形状。

## 3. 连接系统与 ROS

根据机器人调整水动力学、浮力、推进器轴和传感器位姿。系数应符合目标机器人的物理特性。

参考[海流](currents.md)、[添加传感器](custom-sensors.md)及其中链接的插件参考页。需要经纬度时，通过[球面坐标](coordinates.md)设置世界原点。

修改 `config/<namespace>/robot_config.py`，同步更新实体名、关节名、话题、坐标系和端口。功能包已安装 `description`、`meshes`、`config` 及资源搜索配置。请修改源码，不要编辑 `install/` 中的生成文件。

## 4. 编译并测试

在加载了环境的工作空间中执行：

{{CODE:custom-robots:3}}

用小幅指令逐轴检查，同时尝试从源码目录之外启动：

- 已安装资源的路径。
- 质量、惯量和平衡位姿。
- 碰撞几何与接触响应。
- 坐标系和消息内容。

在另一个准备好的终端中，读取重命名后的模型状态：

{{CODE:custom-robots:4}}

应能收到位置和速度。先让复制模型正常运行，再改机械结构，最后添加传感器或调整推进系统。用[传感器数据](visualization.md)比较实验；资源缺失时参考[故障排查](troubleshooting.md)。
