# Sonar public documentation evidence

## Source snapshot

- POSIM sonar PR: https://github.com/IOES-Lab/POSIM/pull/6
- Source head: `ad6165bd6c2b4e407f18ecb264e20bb76da8d4ca`.
- Input guides: `gazebo/DEMO_GUIDE.md` and `gazebo/DEMO_GUIDE_AppleSilicon_MacOSX.md`.
- Runtime report: https://github.com/IOES-Lab/POSIM/pull/6#issuecomment-6135206430.
- User clarified that these documentation changes belong in PR #6; PR #7 is the closed ocean-waves contribution.

## Source-backed user behavior

- `sonar_compute_cpu.cc`: `auto` tries CUDA, WGPU, CPU; explicit failures and invalid names return no backend.
- `MultibeamSonarSensor.cc`: environment name is `POSIM_SONAR_COMPUTE_BACKEND`; publishes Image and ProjectedSonarImage directly to ROS 2, and PointCloudPacked over Gazebo transport; rendering is required for GpuRays.
- Demo launch: declares `compute_backend` (default `auto`) and `rviz`, and sets the POSIM backend environment variable.
- Sonar CMake: WGPU is required, CUDA is optional, `CUDA_ARCHITECTURE` defaults to `native` and forwards to `CMAKE_CUDA_ARCHITECTURES`; Apple builds link Metal/QuartzCore.
- Rust pipeline: Vulkan is the Linux default, Metal is the macOS default; supports adapter substring, backend API and power-preference selection; GPU context is shared in-process.
- WGPU 0.20.1 power-preference values checked against https://github.com/gfx-rs/wgpu/blob/v0.20.1/wgpu/src/util/init.rs: `high`, `low`, `none`.

## Documentation decisions

Public pages describe present behavior and commands. The two demo files and sonar README point to the authoritative English/Korean guides. Root README and installation, requirements, overview and troubleshooting pages use the same supported backend facts.

The inherited demo guides refer to Ubuntu 24.04/ROS Rolling and macOS Jazzy/Harmonic setup. These version-specific setup commands are not copied into Lyrical/Jetty instructions. Native macOS instructions state the matching ROS/Gazebo/rendering prerequisites; they are not a turnkey host installer or a fresh native runtime validation.

The contributor reports Ubuntu 26.04.1, ROS 2 Lyrical, Gazebo 10.5.0, RTX 4060 and Intel iGPU runtime tests. Reported panel range errors at 2–8 m and 0–45 degrees were approximately 0.03–0.05 m for WGPU and 0.03–0.06 m for CUDA, with 2.5 cm range bins. These are attributed test observations, not new measurements by this documentation change or public accuracy guarantees.

The public text omits migration history, commit counts, implementation cleanup, shared-buffer allocation counts and before/after comparisons. Build/check and browser review results are recorded separately from user instructions.

## Documentation validation

- Node.js 24: `pnpm catalog`, `pnpm build`, `pnpm check`.
- Full repository check: 50 pages, 3,575 local links/anchors, 38 bilingual public media, bilingual search and 42 source launch references.
- Website-only build/check passes without the ROS/model source tree.
- EN/KO command blocks match for sonar, sonar tuning, installation, requirements, overview and troubleshooting. Sonar Bash blocks pass `bash -n`.
- Both demo wrappers pass the public-wording checker. `git diff --check` passes.
- WebKit desktop (1440 x 1000) and mobile (390 x 844) checks cover both sonar pages in both languages: no document overflow or JavaScript page errors, and language switching passes.
- Desktop Apple Silicon section and Korean mobile demo output visually inspected. Browser captures/results are local ignored files under `website/.validation/sonar/`.
- Website documentation-basis metadata uses the inspected PR source snapshot; no Notion review date or source media metadata is changed.
- These are documentation checks. This change does not claim a new sonar simulation run or a native Apple Silicon build.
