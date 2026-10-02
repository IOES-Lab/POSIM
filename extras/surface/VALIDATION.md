# Validation of the external dependency redesign

This optional surface integration is based on POSIM main `e05a9fb7d25ec21399f897e9bddfcabc4493b703`; it redesigns the wave/WAM-V portion of [PR #7](https://github.com/IOES-Lab/POSIM/pull/7) without importing its copied wave solver. Existing BlueROV launch, repository source license and CI configuration are unchanged. Standard Docker recipes and the Ubuntu stack installer now build Wave Sim automatically.

## Actual local checks

The shared `extras/install-waves.sh` was additionally tested in a fresh Docker layer on `wwos-runtime:e05a9fb-arm64` (before any Wave Sim dependency was installed). It fetched the pinned SHA, applied the compatibility edits, compiled/installed all libraries, retained source/notices and correctly reused the result on a second invocation. `gz plugin --info --plugin` loaded the newly installed WavesModel and Hydrodynamics shared objects and enumerated their Gazebo interfaces. This validates the shared installer; it does not claim a fresh rebuild of every standard POSIM image layer or an AMD64 run.

On ARM64 Ubuntu 26.04 / ROS 2 Lyrical / Gazebo Jetty:

- The pinned external Wave Sim source built and installed with the supplied CMake adaptation. Core, FFT WavesModel/WavesVisual and wave-aware hydrodynamics libraries loaded successfully under Ogre2 software rendering.
- The same image contains working ArduSub and ArduRover binaries. Only one controller runs per single-vehicle world. ArduRover boat uses differential channels 1/3, with scoped IMU transport and explicit simulated external navigation.
- The exact standalone `wamv.launch.py` mounted into the built downstream runtime started a floating WAM-V and connected ArduRover/MAVROS. An actual GUIDED 5 m outbound/return test completed and disarmed; final horizontal error was 0.385 m.
- The downstream geographic-world GUI completed a 10 m ArduRover route, hold and return, reaching 10.36 m from start and returning within 0.684 m. Duration: 36.55 simulation seconds / 53.74 wall seconds. The browser GIF contains actual time-stamped captures and real Gazebo sensor frames were also collected.
- Downstream BlueROV2 floated with the same external waves/hydrodynamics dependency. Real RGBD seabed hits and physical terrain contact were verified; this PR does not alter the original POSIM BlueROV launch.
- Python syntax, XML construction and whitespace checks pass. [Machine-readable summary](validation.json).

The standalone test reused the compiled external-dependency/runtime layers and mounted this launch/configuration directory. A complete fresh build from the new optional Dockerfile on the exact `posim:dev-arm64-rdp` tag has not been repeated; its build steps match the locally compiled dependency, with explicit MAVROS/scipy runtime dependencies. AMD64 uses a different POSIM user/workspace layout and is not supported by this optional Dockerfile yet.

## Remaining limits

No native GPU or AMD64 result is inferred from software-rendered ARM64. Hull/propulsion/drag calibration, more severe waves, endurance and multi-vehicle instances need further validation. The downstream browser shows mean-sea-level shading, while actual FFT geometry is in Gazebo and sensor rendering. Wave transport publication is not an acknowledgement/readback of plugin state.

High SR0 rates in Rover boot defaults produced a startup floating-point exception in one test; defaults now avoid those changes and navigation message intervals are requested after FCU connection. The post-connection standalone run completed. A shutdown regression remains: some MAVROS exits report `-11` and Gazebo may require supervisor termination during cleanup. Sessions are still reaped and ownership released, but clean library/process teardown needs follow-up before production use. This is a draft integration, not a calibrated or production-ready maritime stack.

Externalizing Wave Sim does not automatically exempt combined/distributed software from GPL obligations. The normal and optional images retain modified upstream source/notices; review its full intended distribution, including CGAL/FFTW. Nothing here changes POSIM's Apache-2.0 license or asserts a legal conclusion about every possible distribution.
