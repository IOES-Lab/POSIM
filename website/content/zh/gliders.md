# Slocum 水下滑翔机

`glider_slocum` 模型包含机体、推进器、水动力学，以及 IMU、导航和压力接口。可用于滑翔机实验，也可作为自定义载具的基础。

## 在海洋世界中启动

在[与安装方式对应的 POSIM 终端](terminals.md)中运行：

{{CODE:gliders:0}}

如需在空世界中观察模型，使用 `world_name:=empty.sdf z:=0.2`。

## 查看观测数据

{{CODE:gliders:1}}

IMU 和 NavSat 的 Gazebo 话题位于 `/model/glider_slocum/` 下。请通过桥接配置确认对应的 ROS 话题和类型。模型还加载了[压力插件](pressure.md)和 Gazebo 里程计发布器。

先启动载具和环境，再接入自己的控制节点，开展下潜或滑翔任务。

## 修改模型

结合桥接配置阅读 [Slocum SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf)。同步调整质量、惯性、碰撞几何和水动力系数，保持它们相互一致。接入估计器或控制器前，请确认坐标系约定。

保留原模型用于比较。[添加机器人](custom-robots.md)介绍如何创建独立模型，[ROS](ros.md)介绍观测与控制接口。

## 查看滑翔机运动

{{MEDIA:gliders}}

图片和视频：POSIM Notion Wiki。DAVE 文档的署名见[引用与许可证](citation.md)。运行时请以本页的命令和话题名称为准。
