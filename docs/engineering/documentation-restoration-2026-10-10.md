# Notion documentation restoration — 2026-10-10

Read all 21 public reference pages through their public `loadPageChunk` endpoint, following cursors to completion. The authenticated Notion connector returned NOT_FOUND for this workspace; public-page reads succeeded. Downloaded source files through public signed URLs, then stored the actual bytes locally. Signed URLs are not committed or required at runtime.

The asset registry is `website/media-sources.json`. It records page/block IDs, source-page URLs, original filenames, deployment audience and SHA-256 digests. 46 image/video blocks were recovered. 38 are used in both public languages; eight are retained here because they contain profiling history or a Classic-to-Gazebo migration error. These engineering records are excluded from the generated public site under website/AGENTS.md.

Substantive public material restored: Fuel/local object spawning and publishing, RGBD configuration and unattenuated/murky comparison, sonar descriptor/world setup and raw-data logging, uniform/stratified/tidal current configuration and service tables, common sensor launch arguments. Current source names, defaults and units replace stale command text; source illustrations retain their pixels and attribution. Corrected current service namespace to `/hydrodynamics` based on OceanCurrentPlugin.cc.

The home-page graphical abstract is semantic HTML/CSS composed of four source sensor/scene illustrations and two real POSIM robot captures from WWW-POSIM, with links to their guides. No synthetic scientific observations are shown. Install/Run/Extend cards use simple white boxes and a thin border.

## Source figures retained for engineering reference

- [Screenshot from 2024-06-05 12-38-27.png](notion-media/objects-6e7c9419.png) — [objects](https://caring-dibble-be5.notion.site/p/POSIM-Object-Models-2c9c941998988216b7a1816fce54894d?pvs=24)
- [CUDA_INITIAL_TIMES.png](notion-media/sonar-optimization-214c9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [CUDA_OLD_KERNEL.png](notion-media/sonar-optimization-7dac9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [CUDA_FINAL_INTRINSICS.png](notion-media/sonar-optimization-fa4c9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [FCUDA_FINAL_INTRINSIC_COMPARE.png](notion-media/sonar-optimization-f3cc9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [CUDA_FINAL_KERN_INTRINSICS.png](notion-media/sonar-optimization-782c9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [resultados_geforce.png](notion-media/sonar-optimization-a57c9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)
- [results_t4.png](notion-media/sonar-optimization-7d0c9419.png) — [sonar-optimization](https://caring-dibble-be5.notion.site/p/POSIM-CUDA-Sonar-Optimization-Reference-ab3c941998988356a7b481b10d998268?pvs=24)

## Robot captures in the graphical abstract

- `website/public/media/overview/bluerov2.png`: copied unchanged from WWW-POSIM `docs/validation/gui-capture/gazebo-bluerov2-close.png`; SHA-256 `873561dd59b599f8d89e1c3328dfcb2f0889a1c46e13e706d91a1f32f74a587e`.
- `website/public/media/overview/wamv.jpg`: copied unchanged from WWW-POSIM `docs/validation/world-water-2026-10-06/robot-after.jpg`; SHA-256 `002ce24e88dd2764b6d190c40442be1026abab2f084f24c99b778bb3869f3d2b`.

## Validation

- POSIM build: 50 English/Korean pages. Documentation checks passed for 3,553 local references, 38 source-media files in both languages, descriptive image alternatives, media SHA-256 digests, engineering-asset exclusion, search coverage and 38 source launch references.
- WWW-POSIM public guide regeneration passed: 28 pages, 1,040 local references, no audit errors. Repository document reachability passed for 192 documents. Existing voyage and runtime edits in that checkout were preserved.
- Native Chrome review: POSIM home page in English/Korean at desktop width and 390 × 844 mobile emulation; WWW-POSIM ROS setup table and example in desktop/mobile layouts.
- WAM-V example checked with a simulated clock and ROS publisher double for normal completion, Ctrl+C and missing-connector timeout. Each case ended with neutral input and node cleanup. The English/Korean code blocks match the runnable example byte-for-byte.
- Actual native Gazebo/Metal WAM-V test passed using the released `www_posim_ros-0.3.0-py3-none-any.whl` and the unmodified documented example. Private engine ROS domain 88, client domain 87, a dedicated transport partition, disposable data and loopback services isolated the public voyage. At requested 1× simulation speed, 78 commands were applied; horizontal displacement was 0.8327 m and counterclockwise yaw change during the observed turn interval was 0.5341 rad. The final controller was inactive with both thrusters at 0 N. Example exit code 0; adapter and gateway exited cleanly. Detailed trace is in WWW-POSIM `.local/validation/ros-doc-example-20261010/result.json`. Fresh Windows/WSL and Ubuntu installations were not exercised.
- English/Korean build and checks repeated before publication. Production deployment verification is recorded separately after publishing.

## Production publication

Published documentation content at commit `116ffe4`. [Documentation workflow 38027299258](https://github.com/IOES-Lab/POSIM/actions/runs/38027299258) completed successfully, including website-only build validation and GitHub Pages deployment.

Public English/Korean home pages, sonar pages and stylesheet match generated output byte-for-byte. All 38 public Notion source-media assets were fetched from GitHub Pages and their SHA-256 digests match the manifest (88,287,235 bytes total). Live Chrome review confirms that the graphical abstract loads above Try WWW-POSIM. Public URLs: [English](https://ioes-lab.github.io/POSIM/) and [Korean](https://ioes-lab.github.io/POSIM/ko/).

WWW-POSIM production deployment `dpl_BsMAANFM5T1yJQT9G7ugsqByqq2k` contains the updated bilingual ROS guide from clean source `a91ccba`; both public HTML pages match the staged files. The release wheel URL was exercised during the actual isolated ROS motion test. Public voyage remains sailing with no error. Its frontend-only document publication did not require an engine restart. WWW-POSIM main PR merge is pending GitHub organization repository access; source changes are pushed to dev.
