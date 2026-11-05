# 物体与任务场景

POSIM 提供用于操作、检查和交互实验的物体模型。`posim_object_models` 安装模型描述、网格与配置。请在[对应环境的 POSIM 终端](terminals.md)中运行命令，WSL、Docker 和 RDP 环境也适用。

## 本地模型目录

以下目录来自 `models/posim_object_models/description`，可查看 SDF、连杆、关节和资源。示例世界还会引用 Gazebo Fuel 中的物体。

{{OBJECT_CATALOG}}

## 从现成场景开始

{{CODE:objects:0}}

其他任务场景包括 `posim_electrical_mating` 和 `posim_bimanual_example`。它们使用 Fuel 中的防喷器面板、公母连接器和水下花瓶等资源。放置机器人或进行接触、关节控制实验前，先查看 SDF 中的资源 URI 与位置。

## 将物体加入世界

把 URI 换成所选模型的实际资源名称：

{{CODE:objects:1}}

位置单位为米，角度单位为弧度。为每个实例指定独立名称。按用途选择 `static`：固定检查目标与需要通过接触移动的物体，应采用不同的动力学设置。

视觉几何决定外观，碰撞几何决定接触。检查比例、位姿、质量、惯量、关节限位和摩擦。先做幅度较小、过程可控的交互实验，再通过 [ROS 2](ros.md)记录状态。

## 单独启动一个物体

加载环境后，启动 `mossy_cinder_block`：

{{CODE:objects:2}}

此启动文件会创建自己的世界。要与其他示例组合，请用 `<include>` 将模型放进同一个世界。参见[添加机器人](custom-robots.md)和[世界场景库](worlds.md)。

## 在本地使用 Fuel 模型

从 [Fuel 模型库](https://app.gazebosim.org/fuel/models)下载并解压模型。将 `model.sdf`、`model.config`、`meshes/` 和 `materials/` 放在同一目录，例如 `~/posim_models/my_object/`。在准备启动 Gazebo 的终端中注册其上级目录：

{{CODE:objects:3}}

在 Gazebo 中打开 **Resource Spawner**，选择本地模型并放置。之后可用 `<uri>model://my_object</uri>` 在世界 SDF 中复用。Fuel 页面 SDF 片段中的 HTTPS URI 也可使用，但首次运行需要下载。离线实验请保留模型文件及许可证。

## 发布到 Fuel

登录 [Gazebo Fuel](https://app.gazebosim.org/home)并创建模型。填写名称、描述和许可证，上传 `model.sdf`、`model.config` 及引用的网格、材质。发布后重新下载，检查是否仍依赖外部本地路径。需要时可加入模型集合。

插件库名和类名必须在已安装的 Jetty 环境中可用。[添加机器人](custom-robots.md)介绍如何连接浮力、水动力学、推进器和传感器。

{{MEDIA:objects}}

图片来源：POSIM Notion Wiki。DAVE 文档署名见[引用与许可证](citation.md)。实际执行时，请使用本页代码块中的命令和话题名称。
