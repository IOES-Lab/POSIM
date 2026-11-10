# 高度图地形

高度图用规则网格上的高程值表示地形。应同时配置视觉几何与碰撞几何，使相机看到的海底与接触表面一致。

## 工作环境

使用 Ubuntu 原生安装、WSL 或 Docker 中[准备好的 Linux 终端](terminals.md)。macOS 用户在 RDP 桌面内操作。编辑和编译都在该环境中完成；`POSIM_WS` 表示其工作空间。

## 参考示例

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) 使用 Fuel 模型 `Santorini Scaled` 构建地理场景。[分级海底](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world)是另一个世界组合示例。

高度图几何的具体定义请参考 [Gazebo 教程](https://gazebosim.org/api/sim/10/heightmap_dem.html)和 [SDFormat 几何参考](http://sdformat.org/spec?ver=1.12&elem=geometry)。

## 将资源放进功能包

世界保存在 `models/posim_worlds/worlds`，数据放在 `models/posim_worlds/media`。检查功能包的 CMake 安装规则和资源搜索路径，使用可移植的已安装资源 URI。

视觉或碰撞几何中的基本结构如下：

{{CODE:heightmaps:0}}

请将 URI 替换为自己的资源。栅格格式和采样方式需要得到渲染器及物理引擎支持。先确认高程编码，再设置垂直尺寸和偏移。

## 保持坐标与比例

`size` 给出水平范围和高程缩放，单位为米；`pos` 指定偏移。归一化图像不会自动获得地理参考。应记录原始覆盖范围、高程范围、海平面基准，以及重投影或重采样过程。

需要地理位置时，在世界中设置 `<spherical_coordinates>`。[球面坐标](coordinates.md)介绍了经纬度与局部坐标之间的转换。

## 编译并检查

添加资源后重新编译并加载环境，从仓库目录之外启动世界，检查：

- 可辨识的地形高程，以及视觉与碰撞面的对齐。
- 传感器测得的深度。
- 受控落体实验中的接触位置。
- 海平面基准和机器人初始高度。

在 Jetty 中，非零的 `<heightmap><pos>` 可能只改变渲染地形的位置，而没有对碰撞面产生相同偏移。将位移移到模型位姿中，也可能仍有错位。请使用实际渲染器和物理引擎验证两种设置。如果仍不一致，应先选用通过接触与视觉检查的几何形式或物理引擎，再开展导航实验。
