# 水下场景效果

`posim_gz_rendering` 为 Ogre2 PBR 表面添加水下雾化、RGB 吸收、随深度衰减的阳光、焦散和悬浮颗粒。颗粒用于近似表现海洋雪，默认**关闭**。

Gazebo 场景、RGB 相机和 RGBD 彩色图像共用这些效果。数值深度和碰撞几何保持不变。独立的深度图像滤镜见[水下相机](camera.md)。

## 构建并集成系统

准备与服务器匹配的 Gazebo Jetty/Ogre2 开发环境。在[准备好的终端 A](terminals.md)中从 POSIM 源码构建；macOS 使用 RDP 桌面内的 Linux 终端：

{{CODE:underwater-rendering:0}}

CMake 需要 Gazebo 开发包，以及 pkg-config 的 `OGRE-Next` 和 `OGRE-Next-Hlms` 条目。命令已加入 Ubuntu 中 ROS 软件包 `gz_ogre_next_vendor` 的路径。库、头文件和着色器应来自同一次构建。

将插件加入 `<world>`，并把 `/absolute/underwater` 换成上述安装前缀。每个 Gazebo 进程只加载一个实例，各会话使用不同的 `session_nonce`：

{{CODE:underwater-rendering:1}}

Linux 使用 `.so`，macOS 的 Docker/RDP 内也一样。直接为 macOS 构建的库使用 `.dylib`。生成传感器图像需要场景中有活动相机。

若同时使用 `UnderwaterCamera`，在服务器环境中将 `POSIM_UNDERWATER_PLUGIN` 设为已加载系统的路径，使已经着色的像素直接通过，避免二次衰减。仅在场景系统已安装并加载时设置它。

## 效果与默认值

| 设置 | 效果 | 默认值 |
|---|---|---|
| 水体光学 | 所有光学效果的总开关 | 开 |
| 水下雾化 | 根据水下光程混合颜色 | 开 |
| RGB 吸收 | 随波长变化的颜色损失 | 开 |
| 阳光衰减 | 随深度减弱的方向光 | 开 |
| 焦散 | 水下表面上移动的阳光纹理 | 开 |
| 悬浮颗粒／海洋雪 | 稀疏的程序生成亮点 | 关 |
| 海底纹理 | 海底 PBR 贴图细节 | 开 |
| 海底网格 | 间距 10 m，可调范围 1–500 m | 关 |
| 等深线 | 间距 5 m，可调范围 1–50 m | 关 |

关闭水体光学仍保留纹理、网格和等深线的选择。海底叠加层识别名为 `seabed` 或 `shallow` 的 visual，也支持 Gazebo 完整作用域名称。纹理需包含在地形资源中，见[高度图地形](heightmaps.md)。

<a id="turn-on-marine-snow"></a>
## 开启海洋雪

启动世界后，在准备好的终端 B 中读取状态：

{{CODE:underwater-rendering:2}}

前 13 个值按以下顺序排列，索引从零开始：

| 索引 | 数值 |
|---|---|
| 0–1 | 模式版本 `1`、修订号 |
| 2–5 | 水体光学、海底纹理、网格、等深线 |
| 6–10 | 雾化、吸收、阳光衰减、焦散、颗粒 |
| 11–12 | 网格和等深线间距，m |

系统刚启动、修订号为 `0` 时，以下请求开启颗粒，同时保留其他默认值：

{{CODE:underwater-rendering:3}}

后续修改应保留所需设置，将修订号加一。布尔字段接受 `0` 或 `1`，也允许重发完全相同的请求。

渲染后再次读取 `/state`，索引 13–15 分别追加 `ready`、相机数量和 `renderer_failed`。检查请求修订号、`ready = 1` 和 `renderer_failed = 0`；`/set` 响应只确认请求已接收。

若希望启动时就开启颗粒，可用以下插件标签代替第一次请求：

{{CODE:underwater-rendering:4}}

WWW-POSIM 使用此公开渲染器，并增加自己的外观面板、会话管理和偏好设置保存。

## 模型范围与比较

水下光程采用 **z = 0 m** 的平均水面，不跟随每个波峰。焦散和颗粒是视觉近似，不模拟校准后的颗粒浓度或光学传感器噪声。

局部灯光保留直接照明。朝向天空的射线应用水下衰减；无光照材质和自定义海面着色器保留自身渲染，因此波面下侧由海面材质处理。

比较水上/水下视角、RGB/RGBD 彩色图像和各个开关。固定位置姿态与光照，确认深度逐字节一致、效果随仿真时间暂停，以及渲染故障可被报告。[公开软件包](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering)提供 RGB/RGBD 检查示例、着色器源码和构建说明。
