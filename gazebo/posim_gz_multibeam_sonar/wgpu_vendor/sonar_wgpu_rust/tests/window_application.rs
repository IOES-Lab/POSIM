// SPDX-License-Identifier: Apache-2.0
// Exercise the production pipeline, not only the isolated FFT implementation.
#[allow(dead_code)]
#[path = "../src/lib.rs"]
mod sonar_backend;

fn response(n: usize, window: &[f32], empty: bool) -> Vec<f32> {
    response_with_amplitude(n, window, empty, 180.0, 1.0)
}

fn response_with_amplitude(
    n: usize,
    window: &[f32],
    empty: bool,
    source_level: f32,
    reflection: f32,
) -> Vec<f32> {
    assert_eq!(window.len(), n);
    let depth = [if empty { 0.0 } else { 5.213 }];
    let normal = [0.0, 0.0, 1.0];
    let reflectivity = [reflection];
    let correction = [1.0];
    let pointer = unsafe {
        sonar_backend::sonar_wgpu_compute(
            depth.as_ptr(),
            normal.as_ptr(),
            reflectivity.as_ptr(),
            correction.as_ptr(),
            window.as_ptr(),
            1,
            1,
            n as u32,
            1,
            1500.0,
            15.0,
            source_level,
            0.0,
            1.0,
            n as f32 * 50.0,
            1.0,
            0.01,
            0.2,
            0.1,
            7,
            20261030,
            false,
        )
    };
    assert!(!pointer.is_null(), "native sonar control returned null");
    let output = unsafe { std::slice::from_raw_parts(pointer, n * 2).to_vec() };
    unsafe { sonar_backend::sonar_wgpu_free(pointer, n * 2) };
    assert!(output.iter().all(|value| value.is_finite()));
    output
}

#[test]
#[ignore = "requires a usable WGPU adapter; run explicitly on Metal/Vulkan/DX12"]
fn production_pipeline_preserves_scaled_reflector_amplitudes() {
    for n in [64, 599] {
        let mut window: Vec<f32> = (0..n)
            .map(|i| {
                (0.54 - 0.46 * (2.0 * std::f64::consts::PI * (i + 1) as f64 / n as f64).cos())
                    as f32
            })
            .collect();
        let norm = window.iter().map(|v| v * v).sum::<f32>().sqrt();
        window.iter_mut().for_each(|value| *value /= norm);
        let baseline = response_with_amplitude(n, &window, false, 180.0, 1.0);
        for (level, reflection, ratio) in [
            (140.0, 1.0, 0.01f64),
            (100.0, 1.0, 0.0001),
            (220.0, 1.0, 100.0),
            (180.0, 1e-8, 0.0001),
        ] {
            let actual = response_with_amplitude(n, &window, false, level, reflection);
            let mut error = 0.0f64;
            let mut power = 0.0f64;
            for (value, original) in actual.iter().zip(&baseline) {
                let expected = *original as f64 * ratio;
                error += (*value as f64 - expected).powi(2);
                power += expected * expected;
            }
            let relative = (error / power).sqrt();
            assert!(
                relative < 2e-4,
                "source/reflectivity amplitude scaling: N={n}, level={level}, reflection={reflection}, relative L2={relative}"
            );
        }
    }
}

#[test]
#[ignore = "requires a usable WGPU adapter; run explicitly on Metal/Vulkan/DX12"]
fn production_pipeline_applies_the_frequency_window() {
    for n in [64, 63, 598] {
        let rectangular = vec![(n as f32).sqrt().recip(); n];
        let mut hamming: Vec<f32> = (0..n)
            .map(|i| {
                (0.54 - 0.46 * (2.0 * std::f64::consts::PI * (i + 1) as f64 / n as f64).cos())
                    as f32
            })
            .collect();
        let energy = hamming.iter().map(|v| v * v).sum::<f32>().sqrt();
        hamming.iter_mut().for_each(|value| *value /= energy);
        let original = response(n, &rectangular, false);
        let repeated = response(n, &rectangular, false);
        let blank = response(n, &rectangular, true);
        let zero = response(n, &vec![0.0; n], false);
        let tapered = response(n, &hamming, false);
        assert!(
            original.iter().any(|v| *v != 0.0),
            "missing reflector control"
        );
        assert_eq!(original, repeated, "identical input must reproduce");
        assert!(
            blank.iter().all(|v| *v == 0.0),
            "empty geometry must be silent"
        );
        assert!(
            zero.iter().all(|v| *v == 0.0),
            "zero frequency window must produce zero return (N={n})"
        );
        let difference = tapered
            .iter()
            .zip(&original)
            .map(|(a, b)| (a - b) * (a - b))
            .sum::<f32>();
        let magnitude = original.iter().map(|v| v * v).sum::<f32>();
        assert!(
            difference > 0.01 * magnitude,
            "Hamming aperture must affect N={n}"
        );
    }
}
