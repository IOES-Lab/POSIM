# 물체와 작업 장면

POSIM에는 조작, 검사와 상호작용 실험용 물체 모델이 있습니다. `posim_object_models` 패키지에서 모델과 메시·설정 리소스를 함께 설치합니다.

## 물체 목록

로컬 모델은 POSIM 패키지에 설치됩니다. 예제 월드의 다른 작업 물체는 Gazebo Fuel에서 참조합니다. 아래 로컬 목록은 현재 소스의 `models/posim_object_models/description`에서 생성합니다. 디렉터리를 열어 SDF, 링크, 관절과 리소스를 확인하세요.

{{OBJECT_CATALOG}}

## 구성된 장면부터 실행

```bash
ros2 launch posim_demos posim_world.launch.py \
  world_name:=posim_plug_and_socket headless:=true
```

다른 작업 장면에는 `posim_electrical_mating`과 `posim_bimanual_example`이 있습니다. Fuel 참조로 방폭 장치 패널, 암·수 플러그와 수중 항아리 검사 대상을 배치합니다. 각 자산 URI와 위치는 월드 SDF에 있습니다. 로봇 배치, 접촉과 관절 제어 실험을 선택하기 전에 살펴보세요.

## 자신의 월드에 물체 배치

환경을 불러온 패키지 리소스에서 찾을 수 있는 모델 URI를 사용합니다. 기본 구조는 다음과 같습니다. URI는 목록에서 선택한 모델의 실제 리소스 이름으로 바꿉니다.

```xml
<include>
  <uri>model://YOUR_OBJECT</uri>
  <name>task_object_1</name>
  <pose>3 0 -5 0 0 0</pose>
</include>
```

위치는 미터, 회전각은 라디안입니다. 여러 개를 넣을 때는 각 인스턴스에 다른 이름을 지정합니다. 고정된 검사 대상과 접촉으로 움직이는 물체는 동역학 조건이 다르므로 `static` 설정을 목적에 맞게 선택하세요.

## 상호작용 확인

Visual 형상은 외관을, collision 형상은 접촉을 결정합니다. 두 형상의 크기와 위치를 함께 확인하세요. 조작 결과를 해석하기 전에 질량, 관성, 관절 범위와 마찰을 검토합니다. 짧은 제어 실험으로 배치와 접촉을 확인한 뒤 [ROS 2](ros.md)로 로봇·물체 상태를 기록합니다.

모델·리소스 규칙은 [로봇 추가](custom-robots.md), 장면 구성은 [월드 목록](worlds.md)을 참고하세요.

## 오브젝트 하나 실행하기

작업 공간을 불러온 뒤 설치된 `mossy_cinder_block`을 실행합니다.

```bash
ros2 launch posim_demos posim_object.launch.py \
  namespace:=mossy_cinder_block paused:=false gui:=false headless:=true
```

객체 launch는 자체 월드를 실행합니다. 다른 예제와 동시에 실행하려면 월드의 `<include>`로 장면을 구성하세요.

## Fuel 모델을 로컬에서 사용하기

[Gazebo Fuel 모델 목록](https://app.gazebosim.org/fuel/models)에서 모델을 내려받고 압축을 풉니다. 예를 들어 `~/posim_models/my_object/`에 `model.sdf`, `model.config`, `meshes/`, `materials/`를 함께 둡니다. Gazebo를 시작할 터미널에서 상위 모델 폴더를 등록합니다.

```bash
export GZ_SIM_RESOURCE_PATH="$HOME/posim_models${GZ_SIM_RESOURCE_PATH:+:$GZ_SIM_RESOURCE_PATH}"
```

Gazebo GUI에서 **Resource Spawner**를 열고 로컬 모델을 선택해 배치합니다. 월드 SDF에 `<uri>model://my_object</uri>`를 포함하면 같은 모델을 재사용할 수 있습니다. Fuel 페이지에서 제공하는 SDF 조각의 HTTPS URI도 사용할 수 있으며, 최초 실행에는 다운로드가 필요합니다. 오프라인 실험에서는 모델 파일과 그 사용 허가를 함께 보관하세요.

## Fuel에 모델 게시하기

[Gazebo Fuel](https://app.gazebosim.org/home)에 로그인하고 모델 생성 메뉴를 엽니다. 이름·설명·라이선스를 입력하고 `model.sdf`, `model.config`와 참조하는 메시·재질을 올립니다. 게시한 모델을 다시 내려받아 외부 경로에 의존하는 파일이 없는지 확인합니다. 필요한 컬렉션에 모델을 추가하세요.

Gazebo 플러그인의 파일명과 클래스는 설치된 Jetty 환경에서 사용 가능한 것으로 설정합니다. [로봇 추가](custom-robots.md)의 현재 모델을 참고해 부력·유체역학·추력·센서를 연결하세요.

## Fuel 모델 가져오기와 게시하기

<figure><a href="{{ASSET_PREFIX}}media/notion/objects-18ec9419.png"><img width="1847" height="948" src="{{ASSET_PREFIX}}media/notion/objects-18ec9419.png" alt="Gazebo Resource Spawner에서 모델 선택" loading="lazy" decoding="async"></a><figcaption>Gazebo Resource Spawner에서 모델 선택</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/objects-f56c9419.png"><img width="1089" height="871" src="{{ASSET_PREFIX}}media/notion/objects-f56c9419.png" alt="Fuel 모델 업로드 화면" loading="lazy" decoding="async"></a><figcaption>Fuel 모델 업로드 화면</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/objects-699c9419.png"><img width="414" height="534" src="{{ASSET_PREFIX}}media/notion/objects-699c9419.png" alt="Fuel 모델의 파일과 정보 입력" loading="lazy" decoding="async"></a><figcaption>Fuel 모델의 파일과 정보 입력</figcaption></figure>
<figure><a href="{{ASSET_PREFIX}}media/notion/objects-993c9419.png"><img width="563" height="419" src="{{ASSET_PREFIX}}media/notion/objects-993c9419.png" alt="Fuel 모델 게시 확인 화면" loading="lazy" decoding="async"></a><figcaption>Fuel 모델 게시 확인 화면</figcaption></figure>

그림·영상 출처: POSIM Notion Wiki. DAVE 문서에서 이어받은 그림의 저자 표시는 [인용과 라이선스](citation.md)를 참고하세요. 실행 명령과 토픽 이름은 이 페이지의 코드 블록을 기준으로 사용하세요.
