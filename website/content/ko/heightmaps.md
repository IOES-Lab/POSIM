# 높이맵 지형

높이맵은 격자마다 고도를 저장하여 지형을 표현합니다. 카메라에 보이는 해저와 접촉 계산에 사용되는 해저가 일치하도록 화면·충돌 형상을 함께 설정합니다.

## 예제 지형

지리 장면은 [Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world), 해저 장면은 [graded seabed](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world)를 참고하세요. Santorini는 Fuel의 `Santorini Scaled` 모델을 사용합니다.

이 예제는 월드 구성 참고용입니다. 높이맵 형상은 Gazebo의 [높이맵 튜토리얼](https://gazebosim.org/api/sim/10/heightmap_dem.html)과 [SDFormat 형상 참고](http://sdformat.org/spec?ver=1.12&elem=geometry)를 따라 설정합니다.

## 패키지 안에 지형 자산 보관

새 월드는 `models/posim_worlds/worlds`, 지형 데이터는 `models/posim_worlds/media`에 넣습니다. 패키지의 CMake 설치 규칙과 리소스 훅을 확인하세요. 개발자 컴퓨터의 절대 경로 대신 설치 후에도 찾을 수 있는 URI를 사용합니다.

Visual 또는 collision 안에 들어가는 형상의 기본 구조는 다음과 같습니다.

```xml
<geometry>
  <heightmap>
    <uri>model://media/meshes/my_heightmap.png</uri>
    <size>1000 1000 80</size>
    <pos>0 0 -80</pos>
  </heightmap>
</geometry>
```

사용자가 추가하는 자산의 예시입니다. 렌더러와 물리 엔진에서 읽을 수 있는 래스터 형식과 샘플링을 선택합니다. 고도 인코딩을 확인한 뒤 수직 크기와 오프셋을 정하세요.

## 좌표와 크기 유지

`size`는 수평 범위와 수직 크기(m), `pos`는 위치 오프셋입니다. 정규화된 영상은 그 자체로 지리 좌표가 있는 고도 자료가 아닙니다. 원본 범위, 고도 범위, 해수면 기준과 투영·재샘플링 과정을 기록하세요.

지리 위치가 필요하면 월드의 `<spherical_coordinates>`에 기준 좌표를 설정합니다. 위도·경도와 로컬 좌표 변환은 [위도·경도 좌표](coordinates.md)를 참고하세요.

## 빌드와 확인

리소스를 추가한 뒤 다시 빌드하고 환경을 불러옵니다. 소스 디렉터리 밖에서 새 월드를 실행하세요. 특징적인 고도, 화면·충돌 형상의 정렬, 센서 깊이와 작은 접촉 실험을 확인합니다. 항법 실험 전에 해수면 기준과 로봇 시작 높이도 확인하세요.
