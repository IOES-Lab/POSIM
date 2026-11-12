# 引用与许可证

POSIM 由 IOES-Lab 维护。使用或分发时，请保留各组件的作者信息及相关声明。

## 引用仿真基础工作

DAVE 项目论文介绍了水下仿真环境：

Zhang 等，*DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*，IEEE/OES Autonomous Underwater Vehicles Symposium，2022 年。[DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808)。

多波束声呐模型对应论文：

Choi 等，*Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*，Frontiers in Robotics and AI，2021 年。[DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646)。

为便于复现实验，还应记录 POSIM 修订版本、机器人与世界配置，以及相关依赖。完整作者名单和书目信息请以原始出版记录为准。

## 组件来源

| 组件 | 来源与声明 |
| --- | --- |
| 采用 Apache 许可证的 POSIM 自有及继承代码 | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE)和各文件中的声明 |
| DAVE 基础代码 | [Project DAVE](https://github.com/Field-Robotics-Lab/dave)，作者及来源声明 |
| 外部 Wave Sim | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim)，原项目 GPLv3 许可证 |
| 外部 WAM-V/VRX 资源 | 随下载依赖保留的声明 |
| Gazebo Fuel 模型 | 各项资源自己的来源与许可证 |

波浪组件的构建会分别保留上游源码和修改内容。依赖与分发说明见 [extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)。将仓库分开并不能替代各组件的许可证要求。

## 文档来源

[原 ROS 2 项目文档](https://dave-ros2.notion.site/)是模型、环境和传感器说明的参考来源。本网站的命令使用 POSIM 软件包名称，以及安装指南中指定的 ROS/Gazebo 环境。

更多参考资料见 [IOES-Lab POSIM Notion Wiki](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74)和各页面的来源链接。

## 维护与联系

韩国海洋大学（KMOU）的 [IOES Lab.](https://lab.wschoi.com) 负责维护 POSIM。

联系实验室请发送邮件至 [woensug.choi@gmail.com](mailto:woensug.choi@gmail.com)。请在 [GitHub Issues](https://github.com/IOES-Lab/POSIM/issues) 报告软件问题。
