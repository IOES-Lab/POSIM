# 수중 장면 효과

POSIM의 `posim_gz_rendering`은 Ogre2 PBR 표면에 수중 경로 흐림, RGB 흡수, 수심별 햇빛 감쇠, 해저 햇빛 무늬와 부유 입자를 표현합니다. 부유 입자는 물속에서 눈송이처럼 보이는 작은 밝은 점으로, 마린 스노를 간단히 표현합니다. 공개 POSIM 소스에 포함되어 있으며 기본값은 **꺼짐**입니다.

Gazebo 장면, RGB 카메라와 RGBD 색 영상에 같은 렌더링 경로를 사용합니다. 수치 깊이와 충돌 형상은 유지됩니다. 깊이 기반 영상 감쇠 플러그인은 [수중 카메라](camera.md)를 참고하세요.

## 효과와 기본값

| 설정 | 표현 | 기본값 |
| --- | --- | --- |
| 수중 광학 효과 | 아래 광학 효과를 함께 켜고 끄는 설정 | 켜짐 |
| 흐림 | 수중 빛의 이동 거리에 따른 색 혼합 | 켜짐 |
| RGB 흡수 | 파장에 따른 색 감쇠 | 켜짐 |
| 햇빛 감쇠 | 수심에 따라 약해지는 방향광 | 켜짐 |
| 해저 햇빛 무늬 | 물속 표면에서 움직이는 햇빛 무늬 | 켜짐 |
| 부유 입자 / 마린 스노 | 시선 경로에 드문드문 나타나는 절차적 밝은 점 | 꺼짐 |
| 해저 텍스처 | 해저에 지정된 PBR 맵의 세부 표현 | 켜짐 |
| 해저 격자 | 기본 간격 10 m, 조절 범위 1–500 m | 꺼짐 |
| 수심 등고선 | 기본 간격 5 m, 조절 범위 1–50 m | 꺼짐 |

수중 광학 효과를 꺼도 텍스처·격자·등고선 선택은 유지됩니다. 해저 오버레이는 Gazebo 범위 이름을 포함해 `seabed` 또는 `shallow`라는 이름의 시각 형상을 인식합니다. 모래 맵과 지형은 [POSIM-Terrain](libraries.md)에서 제공합니다. 렌더러가 텍스처를 내려받지는 않습니다.

## 시스템 빌드와 연결

서버와 일치하는 Gazebo Jetty/Ogre2 개발 환경을 준비합니다. POSIM 체크아웃에서 시스템과 렌더링 보조 라이브러리를 함께 빌드합니다.

```bash
cmake -S gazebo/posim_gz_rendering -B build/underwater \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/install/underwater"
cmake --build build/underwater --parallel 2
cmake --install build/underwater
```

CMake에는 Gazebo 개발 패키지와 `OGRE-Next`, `OGRE-Next-Hlms` pkg-config 항목이 필요합니다. Ubuntu ROS vendor 구성에서는 `/opt/ros/lyrical/opt/gz_ogre_next_vendor/lib/pkgconfig`를 `PKG_CONFIG_PATH`에 포함합니다. 같은 빌드의 라이브러리·헤더·셰이더를 함께 사용하세요.

SDF의 `<world>` 안에 다음 플러그인을 넣습니다. `/absolute/underwater`는 위 설치 경로로 바꿉니다. Gazebo 프로세스당 하나만 연결하고 실행 세션마다 고유한 `session_nonce`를 지정합니다.

```xml
<plugin filename="/absolute/underwater/lib/posim_underwater.so"
        name="posim::Underwater">
  <renderer_library>/absolute/underwater/lib/posim_underwater_render.so</renderer_library>
  <shader_directory>/absolute/underwater/share/posim/underwater</shader_directory>
  <session_nonce>appearance_example</session_nonce>
</plugin>
```

macOS 라이브러리는 `.dylib` 확장자를 사용합니다. 화면 없는 영상 출력에도 렌더링 장면과 활성 카메라가 필요합니다. `UnderwaterCamera`도 사용한다면 서버 환경의 `POSIM_UNDERWATER_PLUGIN`을 불러온 시스템 경로로 지정하세요. 이 설정은 네이티브 효과가 적용된 픽셀을 호환 카메라 출력으로 그대로 전달해 감쇠의 중복 적용을 막습니다. 네이티브 시스템을 정상 설치한 경우에만 설정합니다.

## 부유 입자 켜기

월드를 실행한 뒤 적용된 설정을 읽습니다.

```bash
gz service -s /posim/appearance/appearance_example/state \
  --reqtype gz.msgs.Empty --reptype gz.msgs.Double_V --timeout 3000 --req ''
```

처음 13개 값의 순서는 다음과 같습니다. 인덱스는 0부터 시작합니다.

| 인덱스 | 값 |
| --- | --- |
| 0–1 | 스키마 `1`, 리비전 |
| 2–5 | 수중 광학 효과, 해저 텍스처, 격자, 등고선 |
| 6–10 | 흐림, 흡수, 햇빛 감쇠, 햇빛 무늬, 부유 입자 |
| 11–12 | 격자 간격과 등고선 간격(m) |

리비전 `0`으로 시작한 시스템에서 다음 요청은 다른 기본값을 유지하면서 부유 입자를 켭니다.

```bash
gz service -s /posim/appearance/appearance_example/set \
  --reqtype gz.msgs.Double_V --reptype gz.msgs.Double_V --timeout 3000 \
  --req 'data: [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 10, 5]'
```

이후 변경에서는 원하는 설정을 유지하고 리비전을 1씩 올립니다. 불리언 값은 `0` 또는 `1`입니다. 같은 요청은 그대로 재전송할 수 있습니다. 렌더링 이후 `/state`를 다시 읽으세요. 인덱스 13–15에는 `ready`, 카메라 수, `renderer_failed`가 붙습니다. 요청한 리비전과 `ready = 1`, `renderer_failed = 0`을 확인합니다. `/set` 응답은 요청 수신만 확인합니다.

시작부터 부유 입자를 켜려면 첫 요청 대신 플러그인 안에 다음 SDF 태그를 넣습니다.

```xml
<initial_settings>1 0 1 1 0 0 1 1 1 1 1 10 5</initial_settings>
```

WWW-POSIM은 이 공개 렌더러를 사용하며 외관 설정 패널, 세션 제어와 설정 저장 기능을 제공합니다.

## 모델의 범위와 확인 방법

수중 경로 길이는 **z = 0 m**의 평평한 평균 해수면을 기준으로 계산합니다. 개별 파도의 마루를 따라가지 않습니다. 햇빛 무늬와 부유 입자는 시각적 근사이며, 보정된 입자 농도나 광학 센서 잡음 모델은 아닙니다. 근거리 광원은 직접 조명을 유지합니다. 하늘에는 수중 경로 감쇠를 적용하고, 비조명 재질과 사용자 해수면 셰이더는 자체 렌더링을 사용합니다. 따라서 아래에서 본 파도 표면은 별도의 수면 재질로 다룹니다.

수면 위·아래 장면, RGB·RGBD 색 영상과 각 스위치를 비교하세요. 비교할 때 위치와 조명을 고정합니다. 깊이 바이트가 동일한지, 시간에 따른 효과가 시뮬레이션 일시 정지를 따르는지, 렌더러 오류가 보고되는지 확인합니다. [공개 렌더러 패키지](https://github.com/IOES-Lab/POSIM/tree/main/gazebo/posim_gz_rendering)에 설정·RGB/RGBD 검증 프로그램, 셰이더 소스와 빌드 안내가 있습니다.
