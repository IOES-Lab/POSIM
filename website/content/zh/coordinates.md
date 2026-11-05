# 球面坐标

ROS 插件 `SphericalCoords` 管理世界的地理原点，并在局部直角坐标与经纬度、海拔之间进行双向转换。

## 启动已配置原点的世界

在[按安装方式准备好的终端 A](terminals.md)中启动：

{{CODE:coordinates:0}}

在准备好的终端 B 中查看：

{{CODE:coordinates:1}}

示例 SDF 设置的纬度为 **35.074823°**，经度为 **129.084798°**。以服务响应为准，确认当前世界实际使用的原点。

## SDF 集成与参数

将以下两个配置块直接放在 `<world>` 下：

{{CODE:coordinates:2}}

插件读取世界的坐标设置，没有额外的 SDF 参数。四个服务的路径固定在 `/gz/` 下。若插件不属于世界，或世界没有地理坐标配置，会报错并保持服务停用。

| 世界配置项 | 含义 | 示例 |
|---|---|---|
| `surface_model` | 参考椭球 | `EARTH_WGS84` |
| `world_frame_orientation` | 坐标轴约定 | `ENU` |
| `latitude_deg`, `longitude_deg` | 原点经纬度，度 | `35.074823`, `129.084798` |
| `elevation` | 原点海拔，m | `0` |
| `heading_deg` | 航向偏移，度 | `0` |

请明确设置原点。参见[插件实现](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc)和[示例世界](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world)。

## 服务

类型位于 `posim_interfaces/srv`。角度单位为度，局部坐标和海拔单位为米。

| `/gz/` 下的服务名称 | 类型 | 请求 |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | 空 |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

所有数值必须有限。纬度范围为 −90° 到 90°，经度为 −180° 到 180°。无效的原点更新返回 `success: false`，并保留原点。转换失败时返回 `NaN` 坐标；交给控制器前应检查结果。

## 转换局部位置

{{CODE:coordinates:3}}

响应包含纬度、经度和海拔。将它们传给反向转换服务，再将 `output` 与原始向量比较，即可检查原点和坐标轴约定。

## 修改原点

{{CODE:coordinates:4}}

修改原点会改变局部位置与地理坐标的对应关系。请保持地形、世界方向、海拔基准和控制器配置一致。转换由 Gazebo 完成，应考虑 SDF 中设置的航向和坐标轴方向。
