# Projected-area range control

[한국어](README.ko.md)

This engineering record checks the range dependence of the surface-scattering
kernel. An angular cell has projected area `r² Δθ Δφ`. The received pressure
uses the square root of this area and two-way spreading `1/r²`. At constant
incidence, reflectivity and angular footprint, doubling range therefore gives
one quarter of the integrated received power. A fixed physical patch subtends
one quarter the solid angle at double range and gives one sixteenth the power.
These ratios exclude absorption; the separate absorption control includes it.
The area and spreading terms follow [Choi et al. (2021), equations 2, 8 and
14](https://kerednoslo.github.io/papers/Choi2021.pdf).

The prior CPU, CUDA and WGPU expressions used `sqrt(r Δθ Δφ)`. They now use
`r sqrt(Δθ Δφ)`. The existing ray-local normal is preserved: its third component
already contains the incidence cosine. Applying a second sensor-frame dot
product would be incorrect.

| Controlled change from 5 m | Expected power ratio | Before | After |
| --- | ---: | ---: | ---: |
| Fixed angular cell at 10 m | 0.2500 | 0.1250 | 0.2500 |
| Fixed physical patch at 10 m | 0.0625 | 0.03125 | 0.0625 |
| Fixed angular cell, absorption 0.05 Np/m, 10 m / 5 m | 0.091970 | 0.045985 | 0.091970 |

The retained control uses the actual WGPU C ABI with three beams, three rays,
599 range samples, one active central cell, identity beam mixing, a normalized
rectangular spectrum, 15 m maximum range and two matched RNG frames. Every
complex sample from 24 calls per binary is retained. Before/after amplitudes
differ by the expected `sqrt(r)` factor, with maximum relative L2 error
`2.93e-7`. Area, reflectivity, empty-return and ray-local-normal controls pass.
These are supplied-geometry calculations, not newly rendered scenes.

Run the independent stored-data check with Python and NumPy:

```sh
python verify_record.py
```

The production C++ regression tests both fixed-angular and fixed-physical
footprints at 3, 5, 10 and 15 m, absorption and an empty return. It refuses a
backend fallback. With the package dependencies and WGPU vendor built:

```sh
cmake -S gazebo/posim_gz_multibeam_sonar/multibeam_sonar -B build/sonar \
  -DBUILD_TESTING=ON
cmake --build build/sonar --target scattering_range_test
ctest --test-dir build/sonar -R scattering_range_cpu --output-on-failure
build/sonar/scattering_range_test wgpu
```

CPU and WGPU/Metal execution is recorded in `result.json`. CUDA receives the
same formula correction but is not executed on this Apple Silicon host.
The 599-sample record includes the independently integrated #31–36 fixes;
those branches are not merged into #6 by this correction. The owning branch
is checked separately with the 64-sample C++ control.

This restores a published area term; it is not a new research method or a
hardware calibration. The paper's equations 7 and 14 use different incidence
powers. The CPU fallback and GPU also retain different angular laws, so this
record does not establish backend-equivalent scattering. Physical beam width,
per-beam normalization, sampling convergence, temporal scattering, new field
data and useful sonar-SLAM maps remain separate validation work. No installed
SDK or active mission is replaced by these controls.
