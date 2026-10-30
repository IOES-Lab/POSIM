# Sonar geometry and image controls

[한국어](README.ko.md)

The sensor constructs its depth grid from the configured horizontal beams and
vertical rays, and keeps the corresponding angles and field of view together.
Radial ranges are samples on a spherical fan. Surface normals are recovered
from Cartesian neighborhoods and expressed in the local ray basis, so their
forward component supplies the acoustic incidence cosine. A bounded angular
neighborhood limits resolution-dependent range quantization while preserving
nearby sides at discontinuities. Unsupported isolated returns supply no normal.

The configured acoustic visibility mask is applied to the depth camera. Metric
fan preview width includes the horizontal field of view, and an empty return
does not divide its preview by zero. These controls do not estimate missing
elevation or qualify a SLAM map.

The CPU-only analytic regression checks front/oblique planes and seabed at
64×32 and 256×256 resolution, an isolated return and a 5 cm step. Plane and
step incidence-cosine error must remain below 0.001. This is a geometry
calculation tolerance, not sensor noise or a marine-map acceptance threshold.

With ROS/Gazebo/OpenCV dependencies sourced, enable the test in this package:

```sh
cmake -S gazebo/posim_gz_multibeam_sonar/multibeam_sonar -B build/sonar \
  -DBUILD_TESTING=ON
cmake --build build/sonar --target test_ray_surface_normals
ctest --test-dir build/sonar -R ray_surface_normals --output-on-failure
```

The spherical normal helper and analytic regression are Apache-2.0. Existing
DAVE/WGPU contribution history and notices remain. Related source corrections
belong to their focused PRs: initial sensor pose in #31, captured simulation
stamp ownership in #32, and frequency-window application/locked Rust builds
in #34. The partial-matrix correction in #33 remains independent. Validate their
combination on a separate current-main integration branch before discussing
their eventual merge into #6.
