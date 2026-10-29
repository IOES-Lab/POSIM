# Current-main sonar lifecycle validation

The separate `codex/sonar-current-main-integration-20261029` branch contains
POSIM main `8c47601` and PRs #6, #31–#34. Follow-ups are **not** merged into #6,
and no sonar PR is merged into remote main. `source.json` records exact heads.
The native binaries are built at `de443a4`; the later integration changes only
the two test files, then adds this validation record.

On Apple Silicon / native macOS, both CPU and WGPU/Metal pass:

| Check | CPU | WGPU/Metal |
| --- | ---: | ---: |
| Static creation: actual raw messages | 5 | 5 |
| Removal/recreation: actual raw messages | 5 | 5 |
| Initial / total initialization count | 1 / 2 | 1 / 2 |
| SIGINT/SIGTERM × 0/5/50 ms × two repeats | 12/12 | 12/12 |
| Maximum shutdown | 3.175 s | 3.169 s |

All 26 accepted native process groups are absent at the cleanup audit; no
forced cleanup is needed. Each raw message has 513 beams, 399 range entries and
the expected float payload size. This checks lifecycle and message shape,
**not** configured-ray accuracy, intensity accuracy, geometry, SLAM or an
installed archive. In particular, these are the upstream BlueView fixture
settings, not the application's Sonoptix vehicle profile. The CPU binned-return
approximation is not used as a numerical reference for the WGPU pipeline.

## Dependency corrections and test boundaries

`upstream-patches` contains Apache-2.0 derivatives of Gazebo Sim 10.5.0 and
Gazebo Plugin 4.0.0. Complete original sources remain available at their tags.

- The existing sensor shutdown-lifetime correction is retained.
- Forced render demand initializes the macOS context on the main thread;
  PostUpdate waits for the scene. A camera-free, sonar-free and ROS-free
  `RenderDemand` control reproduces the original crash and verifies both
  Configure and PostUpdate demand after correction. No dummy camera is added.
- The loader checks `dlopen`'s returned handle. Optional symbol probes inside
  constructors can leave diagnostic text despite a successful load. The
  constructor control fails with the original loader and passes after the
  correction; missing libraries and missing plugin exports still fail. A
  merely preexisting error is a separate passing control, not the reproducer.

Only the corrected loader is selected over the existing SDK's other Plugin
libraries. Dynamic-library traces verify both dependency corrections and both
newly built sonar libraries in every accepted native case. The existing SDK
installation is unchanged. The dependency libraries are built through the
actual application SDK assembly functions in isolated output prefixes.
No private application implementation is included in this record.

PR #31's receiver uses volatile, best-effort QoS compatible with the actual
publisher, and a unique topic per test run. A former transient-local request
does not receive this publisher's data. Exact initialization counts, valid
messages and normal process exit replace the broad `already exists` substring
check, which also matches unrelated Protobuf diagnostics. Failed local harness
controls remain in the record. Neither numerical nor lifecycle time limits
are increased; the exit deadline remains 20 seconds.

The opt-in tests select native Metal on macOS and headless rendering elsewhere.
`--sensor-system-plugin` selects an explicit dependency build for development
validation. Source PRs #31 and #32 retain their independent changes. The
platform-renderer-only edits in the preceding review checkout are preserved.
Black 24.10.0 (99 columns), Flake8 and `git diff --check` pass for both test files.
Native Linux, CUDA and Windows are not rerun here.

Run `python3 verify.py` to check all retained hashes and measured records.
The standalone C++ controls need the corresponding Gazebo development packages.
The lifecycle tests also need ROS and `marine_acoustic_msgs`. Full logs are
compressed; no replacement raw acoustic dataset is collected by these tests.

한국어: 최신 main과 소나 PR 다섯 개를 별도 브랜치에서 검증합니다. CPU·Metal
모두 생성·제거·재생성 때 실제 원시 메시지를 수신하고 초기 종료 12건씩을
통과합니다. 공통 렌더러·로더 결함은 독립 재현 시험으로 분리해 수정합니다.
시험 토픽과 QoS도 실제 발행기와 맞춥니다. 이 결과를 소나 형상 정확도나
SLAM 지도·설치 패키지의 통과로 확대하지 않습니다.
