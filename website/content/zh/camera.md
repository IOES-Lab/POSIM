# 水下相机

`UnderwaterCamera` 利用深度信息，让 RGB 颜色随距离衰减。可按目标水体调整衰减系数和背景色。整个场景共用的视觉效果见[水下场景渲染](underwater-rendering.md)。

<a id="run-the-example"></a>
## 运行示例

此示例使用基于深度的相机滤镜。场景渲染器有单独的[构建和配置步骤](underwater-rendering.md)。在[按安装方式准备好的终端 A](terminals.md)中运行：

{{CODE:camera:0}}

在准备好的终端 B 中查看：

{{CODE:camera:1}}

在 RViz 中添加 Image 显示项。如果话题可见但图像停止到达，请检查[大图像消息的传输设置](troubleshooting.md#image-topics-appear-but-frames-stall)。

## 配置插件

将以下 RGBD 传感器放在 `<link>` 内。世界还需加载指定 `<render_engine>ogre2</render_engine>` 的 `gz-sim-sensors-system`，参见[相机世界](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/camera_tutorial.world)。插件读取 Gazebo 彩色和深度图像，直接发布 ROS 话题 `/underwater_camera/simulated_image`。

{{CODE:camera:2}}

| SDF 参数 | 含义 | 默认值 |
|---|---|---|
| `attenuationR` | 红色通道每米衰减系数，有限且 ≥ 0 | `1/30` |
| `attenuationG` | 绿色通道每米衰减系数，有限且 ≥ 0 | `1/30` |
| `attenuationB` | 蓝色通道每米衰减系数，有限且 ≥ 0 | `1/30` |
| `backgroundR` | 背景红色分量，0–255 整数 | `0` |
| `backgroundG` | 背景绿色分量，0–255 整数 | `0` |
| `backgroundB` | 背景蓝色分量，0–255 整数 | `0` |

## 话题与相机几何参数

`<topic>` 必须生成有效的 ROS 话题名。省略时使用 Gazebo 路径 `/world/<world>/model/<model>/link/<link>/sensor/<sensor>`，再加 `/simulated_image` 作为 ROS 输出。嵌套模型还包含父模型路径。

将深度转换为沿视线的距离时，依次采用显式镜头投影、显式内参，最后才使用图像尺寸和水平视场角。主点坐标可以为零。焦距必须为正，镜头参数必须有限，以米为单位的裁剪距离须满足 `0 < near < far`。无效配置会报错并停用输出。

## 帧同步与示例值

彩色帧和深度帧按采集时间戳配对，与到达顺序无关。ROS 保留彩色帧的时间戳和坐标系标识。缓冲区满时丢弃未配对帧；仿真时间重置会清空待配对帧，允许时间戳重新开始。

示例设置为 **320 × 240、10 Hz**，RGB 衰减系数 **0.8/0.5/0.2**，背景 **85/107/47**。SDF 配置优先于代码默认值。

## 调整与比较

将容易辨认的物体放在已知距离处。固定位置姿态、光照、材质和相机设置，每次只修改一个通道。除图像外，也检查采集时间戳和实际接收频率。

每个通道采用指数衰减并混合背景色。需要无衰减对照时，将表中的**六个参数全部设为零**。省略标签会恢复默认值。

[场景渲染器](underwater-rendering.md)为 PBR 表面和相机提供共同效果。该系统运行时，`POSIM_UNDERWATER_PLUGIN` 会让已着色像素直接通过，避免二次衰减。输入输出连接见[相机模型](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/underwater_camera/model.sdf)。

## 悬浮颗粒

场景渲染器可在 RGB/RGBD 彩色图像中显示悬浮颗粒。默认关闭，可通过[外观设置服务](underwater-rendering.md#turn-on-marine-snow)开启。

- 各相机共享颗粒的世界位置。颗粒随仿真时间匀速漂移，与模拟海流相互独立。
- 不透明表面会遮挡颗粒。可见距离为 **0.05–3 m**，并受场景和平均水面限制。
- 数值深度和声学射线保持不变。

这是程序生成的视觉效果。定量研究散射需要适合的光学模型。评估视觉导航时，将颗粒视作运动观测对象，并在对比实验中保持外观设置一致。

## 比较水下图像

{{MEDIA:camera}}

图片来源：POSIM Notion Wiki；DAVE 署名见[引用与许可证](citation.md)。执行时使用本页代码块中的命令和话题名称。
