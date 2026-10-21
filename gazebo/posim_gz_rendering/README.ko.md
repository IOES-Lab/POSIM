# 수중 표면 렌더링

[English](README.md)

Gazebo Jetty / Ogre2 시스템이 PBR 표면을 그릴 때 수중 경로 안개, RGB 흡수, 수심에 따른 햇빛 감쇠, 움직이는 해저면 햇빛 무늬와 선택 가능한 부유 입자를 적용합니다. RGB 카메라와 RGBD 컬러 영상은 같은 재질 경로를 사용합니다. 수치 깊이와 충돌 형상은 측정값을 유지합니다. Gazebo의 클리핑, 단색 센서 출력과 지형 그림자 기능을 유지합니다.

수면은 z = 0 m 평면으로 근사합니다. 햇빛 무늬와 부유 입자는 계산량이 적은 절차적 근사입니다. 하늘 배경에도 수중 경로 감쇠가 적용되며 깊이 측정은 유지됩니다. 비조명 객체와 사용자 수면 셰이더는 자체 표현을 사용합니다. 로봇 조명의 직접 조명은 유지하며, 표면과 카메라 사이의 물을 통과하는 빛은 감쇠합니다.

서버와 같은 Gazebo/Ogre SDK에서 CMake로 빌드합니다. 시스템 라이브러리는 Gazebo가 렌더링 장면을 만든 뒤 렌더링 보조 라이브러리를 불러옵니다. 라이브러리는 `lib`, 셰이더는 `share/posim/underwater`에 설치합니다.

```xml
<plugin filename="/absolute/lib/posim_underwater.so" name="posim::Underwater">
  <renderer_library>/absolute/lib/posim_underwater_render.so</renderer_library>
  <shader_directory>/absolute/share/posim/underwater</shader_directory>
  <session_nonce>unique_session_id</session_nonce>
</plugin>
```

macOS 라이브러리 확장자는 `.dylib`입니다. Gazebo 프로세스마다 한 인스턴스를 사용하세요. 렌더러는 엔진의 PBS 리스너를 확장하고 렌더링 종료 시 복원합니다.

`/posim/appearance/<session_nonce>/set`은 `gz.msgs.Double_V`를 받습니다. 13개 값의 순서는 스키마(1), 리비전, 수중 광학 효과, 해저면 질감, 격자선, 등고선, 안개, 색상 흡수, 햇빛 감쇠, 햇빛 무늬, 부유 입자, 격자 간격, 등고선 간격입니다. 논리값은 0 또는 1입니다. 리비전은 정확히 1씩 증가하며 같은 요청의 재전송은 중복 적용하지 않습니다. `/state`는 `gz.msgs.Empty`를 받아 13개 값 뒤에 준비 여부, 카메라 수, 렌더러 실패 여부를 반환합니다. 렌더링 후 적용 상태를 확인합니다.

- 기본 켜짐: 광학 효과, 질감, 안개, 색상 흡수, 햇빛 감쇠, 햇빛 무늬.
- 기본 꺼짐: 격자선, 등고선, 부유 입자.
- 격자 간격: 기본 10 m, 범위 1–500 m.
- 등고선 간격: 기본 5 m, 범위 1–50 m.
- 수중 광학 효과를 꺼도 질감과 측정선 선택은 유지됩니다.

해저면 시각 요소의 이름은 `seabed` 또는 `shallow`이며 Gazebo 범위 이름도 지원합니다. 지형 내보내기에서 PBR 맵을 제공합니다. 질감과 측정선은 이 시각 요소의 해수면 아래 부분에 적용됩니다. `POSIM_UNDERWATER_PLUGIN`이 설정되면 기존 `UnderwaterCamera` 호환 출력은 네이티브 컬러 영상을 그대로 전달합니다. 호스트는 네이티브 시스템을 설치한 경우에만 이 값을 설정하고 렌더러 실패를 사용자에게 표시해야 합니다.

`underwater-settings-test`는 설정과 수중 경로 경계를 검사합니다. `underwater-render-test SHADER_DIRECTORY OUTPUT_DIRECTORY SAND_MAP_DIRECTORY`는 RGB·RGBD 비교 영상을 만들고 설정을 바꾸어도 깊이 데이터가 바이트 단위로 같은지 확인합니다. POSIM-Terrain의 모래 맵과 설정된 Gazebo 환경에서 실행하세요.
