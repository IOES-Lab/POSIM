# POSIM marine GNSS / 해양 GNSS

`posim::Gnss` is an Apache-2.0 Gazebo Sim system. It measures the world pose of
`gnss_link`, applies Gaussian receiver errors, and publishes one atomic
`gz.msgs.Double_V` measurement. It uses the world's WGS 84 spherical coordinates.
It supports any model with a named antenna link; vehicle mounts and receiver
defaults are in `config/profiles.json`. BlueROV mounts are optional and disabled
by default. The WAM-V model requires the external Wave Sim `wam-v` model.

`posim::Gnss`는 Apache-2.0 Gazebo Sim 시스템입니다. `gnss_link`의 월드 위치에
가우시안 측정 오차를 적용하고 원자적 `gz.msgs.Double_V` 측정값을 발행합니다.
월드의 WGS 84 spherical coordinates를 사용합니다. 이름이 있는 안테나 링크를
갖춘 모든 모델에 적용할 수 있습니다. 장착 위치와 기본 설정은
`config/profiles.json`에 있습니다. BlueROV의 선택 GNSS는 기본적으로 꺼져
있습니다. WAM-V 모델은 외부 Wave Sim의 `wam-v` 모델이 필요합니다.

Build / 빌드:

```sh
source /opt/ros/lyrical/setup.bash
colcon build --packages-select posim_gz_gnss posim_robot_models
source install/setup.bash
```

The model plugin accepts the receiver keys in the profile plus `enabled`,
`link_name`, `topic` and `wave_topic`. An empty wave topic selects the mean
`water_level` plane. A nonempty wave topic requires a fresh periodic sampled
heightfield (version 1, count, tile X/Y, wind speed/angle, steepness, simulation
time, row-major heights). Wind metadata does not gate reception.

모델 플러그인은 프로파일의 수신기 설정 외에 `enabled`, `link_name`, `topic`,
`wave_topic`을 받습니다. 빈 wave 토픽은 평균 `water_level` 평면을 사용합니다.
wave 토픽을 지정하면 최신 주기 높이장 자료가 필요합니다. 형식은 버전 1,
격자 개수, 타일 X/Y 크기, 풍속·풍향, 파고 계수, 시뮬레이션 시각, 행 우선
높이 배열입니다. 바람 메타데이터는 수신 차단 조건에 사용하지 않습니다.

## Atomic measurement / 원자적 측정 형식

| Index | Field / 필드 |
|---|---|
| 0–3 | schema=1, reset epoch, acquisition simulation seconds, state |
| 4–6 | latitude degrees, longitude degrees, altitude metres |
| 7–9 | east, north, up velocity in m/s |
| 10–12 | horizontal position variance, vertical position variance, velocity variance |
| 13–14 | world antenna height, local surface height in metres |

States / 상태: `0 disabled`, `1 submerged`, `2 acquiring`, `3 fix`, `4 blocked`,
`5 surface unavailable`. Invalid measurements contain NaN coordinates and
velocities. Immersion or blockage clears delayed fixes. Acquisition needs
continuous clearance above the dry margin. Reset clears the acquisition timer
and delay queue and advances the epoch. `/model/{name}/gnss/blocked` accepts an
explicit `gz.msgs.Boolean` obstruction input.

무효 측정의 위치·속도는 NaN입니다. 침수나 가림은 지연 대기 fix를 폐기합니다.
재획득에는 dry margin 위에서 연속 유지 시간이 필요합니다. Reset은 획득
타이머와 지연 큐를 비우고 epoch를 증가시킵니다.
`/model/{name}/gnss/blocked`의 `gz.msgs.Boolean`으로 가림을 지정할 수 있습니다.

This is a simplified satellite receiver, with no acoustic UGPS, RF propagation,
satellite geometry or physical multipath solver. Nominal accuracy is configurable,
not a vendor product guarantee. The WWW-POSIM adapter owns ROS NavSatFix and
GPS_INPUT conversion, freshness, FCU source selection and UI state separation.
See the [EN specification](https://www-posim.vercel.app/guide/gnss.html) and
[한국어 사양](https://www-posim.vercel.app/guide/gnss.ko.html).

이 센서는 간단한 위성 수신기 모델입니다. 음향식 UGPS, RF 전파, 위성 배치,
물리 다중 경로 계산은 구현하지 않습니다. 정확도는 설정값이며 제품 보증이
아닙니다. ROS NavSatFix·GPS_INPUT 변환, 최신성 검사, FCU 항법 소스 선택,
화면 상태 구분은 WWW-POSIM 어댑터가 담당합니다.
