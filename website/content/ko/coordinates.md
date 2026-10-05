# 위도·경도 좌표

`SphericalCoords` ROS 플러그인은 월드의 지리 기준 좌표를 조회·변경하고 로컬 직교 좌표와 위도·경도·고도를 변환하는 서비스를 제공합니다.

## 기준 좌표가 있는 월드 실행

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_bimanual_example headless:=true
```

같은 환경의 두 번째 터미널에서 확인합니다.

```bash
ros2 service list -t
ros2 service call /gz/get_origin_spherical_coordinates \
  posim_interfaces/srv/GetOriginSphericalCoord '{}'
```

예제 SDF의 기준은 위도 35.074823°, 경도 129.084798°입니다. 실제 실행 중인 기준 좌표는 응답으로 확인하세요.

## 서비스 목록

| `/gz/` 아래 서비스 이름 | POSIM 서비스 자료형 | 요청 |
| --- | --- | --- |
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | 없음 |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

자료형은 `posim_interfaces/srv`에 있습니다. 각도는 도, 로컬 좌표와 고도는 미터입니다.

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

기준 좌표를 바꾸면 좌표의 해석이 달라집니다. 지형을 내려받거나 모든 장면 물체를 다른 지리 영역으로 이동시키는 동작은 아닙니다. 지형, 월드 방향, 고도 기준과 항법 제어기를 일관되게 설정하세요.

내부에서는 Gazebo의 구면 좌표 변환을 사용합니다. 모든 월드의 로컬 축이 같다고 가정하지 말고 SDF의 heading·방향 설정을 함께 확인하세요.
