# 위도·경도 좌표

`SphericalCoords` ROS 플러그인은 월드의 지리 기준 좌표를 조회·변경하고 로컬 직교 좌표와 위도·경도·고도를 변환하는 서비스를 제공합니다.

## 기준 좌표가 있는 월드 실행

[설치 방식에 맞게 준비한 터미널 A](terminals.md)에서 실행합니다.

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_bimanual_example headless:=false
```

준비한 터미널 B에서 확인합니다.

```bash
ros2 service list -t
ros2 service call /gz/get_origin_spherical_coordinates \
  posim_interfaces/srv/GetOriginSphericalCoord '{}'
```

예제 SDF의 기준은 위도 35.074823°, 경도 129.084798°입니다. 실제 실행 중인 기준 좌표는 응답으로 확인하세요.

## SDF 구성과 설정값

두 블록을 `<world>` 바로 안에 넣습니다. 플러그인은 월드의 구면 좌표 설정을 읽으며 플러그인 자체의 SDF 매개변수는 없습니다. 네 ROS 서비스 경로는 `/gz/` 아래로 고정됩니다. 월드가 아닌 곳에 플러그인을 넣거나 월드에 구면 좌표 설정이 없으면 오류를 기록하고 서비스를 생성하지 않습니다.

```xml
<spherical_coordinates>
  <surface_model>EARTH_WGS84</surface_model>
  <world_frame_orientation>ENU</world_frame_orientation>
  <latitude_deg>35.074823</latitude_deg>
  <longitude_deg>129.084798</longitude_deg>
  <elevation>0</elevation>
  <heading_deg>0</heading_deg>
</spherical_coordinates>
<plugin filename="SphericalCoords"
        name="posim_ros_gz_plugins::SphericalCoords"/>
```

| 월드 SDF 항목 | 의미 | 예제 값 |
| --- | --- | --- |
| `surface_model` | 기준 타원체 | `EARTH_WGS84` |
| `world_frame_orientation` | 월드 축 규칙 | `ENU` |
| `latitude_deg`, `longitude_deg` | 원점의 위도·경도(도) | 35.074823, 129.084798 |
| `elevation` | 원점 고도(m) | 0 |
| `heading_deg` | 방위 오프셋(도) | 0 |

월드에 원점을 명시하세요. 서비스 연결은 [소스](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc)와 [예제 월드](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world)를 참고하세요.

## 서비스 목록

| `/gz/` 아래 서비스 이름 | POSIM 서비스 자료형 | 요청 |
| --- | --- | --- |
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | 없음 |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

자료형은 `posim_interfaces/srv`에 있습니다. 각도는 도, 로컬 좌표와 고도는 미터입니다.

요청에는 유한한 수를 사용해야 합니다. 위도는 −90°~90°, 경도는 −180°~180° 범위입니다. 잘못된 원점 변경 요청은 `success: false`를 반환하고 기존 원점을 유지합니다. 좌표 변환에 실패하면 `NaN` 좌표를 반환하므로 제어기에 사용하기 전에 결과를 확인하세요.

## 로컬 위치 변환

```bash
ros2 service call /gz/transform_to_spherical_coordinates \
  posim_interfaces/srv/TransformToSphericalCoord \
  '{input: {x: 100.0, y: 200.0, z: 3.0}}'
```

응답에는 위도, 경도와 고도가 있습니다. 반환 값을 역변환 서비스에 넣고 `output`을 원래 벡터와 비교합니다. 설정된 기준 좌표와 좌표계 규칙을 확인할 수 있습니다.

## 기준 좌표 변경

```bash
ros2 service call /gz/set_origin_spherical_coordinates \
  posim_interfaces/srv/SetOriginSphericalCoord \
  '{latitude_deg: 35.074823, longitude_deg: 129.084798, altitude: 0.0}'
```

기준 좌표 변경은 로컬 위치의 위도·경도 해석에 적용됩니다. 지형·월드 방향·고도 기준·제어기도 같은 좌표계로 설정하세요.

내부에서는 Gazebo의 구면 좌표 변환을 사용합니다. 모든 월드의 로컬 축이 같다고 가정하지 말고 SDF의 heading·방향 설정을 함께 확인하세요.
