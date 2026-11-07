# 功能与示例

POSIM 提供 Gazebo 系统、机器人模型和 ROS 接口。世界 SDF 与启动配置决定每次仿真使用哪些系统。

## 环境与渲染

| 功能 | 实现 | 使用指南 |
|---|---|---|
| 水下雾化、RGB 吸收和阳光衰减 | `gazebo/posim_gz_rendering` | [水下渲染](underwater-rendering.md) |
| 海底焦散和悬浮颗粒 | `gazebo/posim_gz_rendering` | [启用视觉效果](underwater-rendering.md) |
| 海底纹理、测量网格和等深线 | `gazebo/posim_gz_rendering`，地形提供贴图 | [材质与间距](underwater-rendering.md) |
| 谱波与考虑波浪影响的 WAM-V 水动力学 | 外部 Wave Sim；`extras/surface` 集成 | [水面机器人](surface.md) |
| 均匀海流、随深度变化的海流和潮流 | `posim_gz_world_plugins`、`posim_gz_model_plugins`、`posim_ros_gz_plugins` | [海流](currents.md) |
| 地理原点与坐标转换 | `posim_ros_gz_plugins` | [球面坐标](coordinates.md) |
| 海底、物体和任务场景 | `models/posim_worlds`、`posim_object_models` | [世界场景](worlds.md)、[物体](objects.md)、[地形](heightmaps.md) |

水下光学效果作用于 PBR 表面，并保留数值深度。海面采用独立的材质渲染。海洋雪是程序化生成的视觉近似，默认关闭。[水下渲染指南](underwater-rendering.md)介绍了海平面假设和相机行为。

## 传感器、机器人与控制

| 功能 | 实现 | 使用指南 |
|---|---|---|
| 海洋 GNSS、浸水失锁、重新捕获和延迟 | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| 基于 RGBD 的水下图像衰减 | `posim_gz_sensor_plugins/UnderwaterCamera` | [相机](camera.md) |
| DVL 测量与 ROS 转换 | Gazebo DVL、`posim_ros_gz_plugins` | [DVL](dvl.md) |
| 静水压力与推算深度 | `posim_gz_sensor_plugins` | [压力](pressure.md) |
| USBL 声学询问与定位 | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| 多波束声纳 | `gazebo/posim_gz_multibeam_sonar` | [声纳](sonar.md)、[配置与性能](sonar-tuning.md) |
| REXROV、BlueROV2 各型号和 Slocum 滑翔机 | `models/posim_robot_models` | [ROV](rovs.md)、[滑翔机](gliders.md)、[添加机器人](custom-robots.md) |
| WAM-V 的 ArduRover 集成与 ROS 控制 | `extras/surface`、启动配置、`posim_interfaces` | [水面艇](surface.md)、[ROS 2](ros.md) |

可用传感器取决于已安装的插件和模型配置。GNSS 配置描述安装位置，集成时还需加入连杆和接收机。声呐可按环境选择 CPU、WGPU 或 CUDA。 各传感器参考页的快速入门可用于确认仿真中的实时测量输出。

## 源码目录

以下路径相对于 POSIM 仓库。

| 目录 | 内容 |
|---|---|
| `models/posim_worlds` | 世界文件及环境资源 |
| `models/posim_robot_models` | 机器人机械结构、传感器及各机器人的 ROS 配置 |
| `models/posim_sensor_models` | 独立传感器描述与示例 |
| `gazebo/` | 海洋环境和传感器插件 |
| `posim_interfaces` | POSIM 消息与服务定义 |
| `examples/posim_demos` | 连接世界、模型与 ROS 的启动文件 |

[首次仿真](quickstart.md)演示如何组合运行这些部分。需要修改时，参见[添加机器人](custom-robots.md)和[创建世界](custom-worlds.md)。

## macOS 的 Ogre 替换库

[Ogre 构建工具](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md)为 macOS arm64 与 Ogre-Next 2.3.3 的组合构建独立 Ogre 核心。请遵循该指南支持的 SDK 配置，并检查实际加载的库。

第三方组件的授权条件见[许可证](citation.md)。
