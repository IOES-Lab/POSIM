//! Opt-in GPU test: cargo test --release --test fft_lengths -- --ignored --nocapture
//! Set WGPU_BACKEND=metal/vulkan/dx12. This tests production FFT sources against
//! an independent f64 forward DFT, not against the old interpolation path.
#[path = "../src/exact_fft.rs"]
mod exact_fft;
use std::sync::mpsc;
use wgpu::util::DeviceExt;

#[test]
#[ignore = "requires a WGPU compute adapter with 32 KiB workgroup storage"]
fn original_length_dft_matches_reference() {
    pollster::block_on(check());
}

async fn check() {
    let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
        backends: wgpu::util::backend_bits_from_env().unwrap_or(wgpu::Backends::all()),
        ..Default::default()
    });
    let adapter = instance
        .request_adapter(&Default::default())
        .await
        .expect("GPU adapter required");
    eprintln!("adapter: {:?}", adapter.get_info());
    let mut limits = wgpu::Limits::default();
    limits.max_compute_workgroup_storage_size = 32768;
    let (d, q) = adapter
        .request_device(
            &wgpu::DeviceDescriptor {
                label: None,
                required_features: wgpu::Features::empty(),
                required_limits: limits,
            },
            None,
        )
        .await
        .unwrap();
    let shader = d.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: None,
        source: wgpu::ShaderSource::Wgsl(include_str!("../src/shaders/fft.wgsl").into()),
    });
    let radix = d.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: None,
        layout: None,
        module: &shader,
        entry_point: "main",
        compilation_options: Default::default(),
    });
    let exact = exact_fft::Pipeline::new(&d);
    let mut cases = 0;
    let mut worst = 0.0f64;
    for n in [
        1u32, 2, 3, 7, 15, 16, 17, 31, 32, 63, 127, 128, 255, 256, 399, 511, 512, 513, 1023, 1024,
        2047, 2048, 4095, 4096,
    ] {
        let beams = 2u32;
        let count = (n * beams) as usize;
        let size = count as u64 * 4;
        let make = || {
            d.create_buffer(&wgpu::BufferDescriptor {
                label: None,
                size,
                usage: wgpu::BufferUsages::STORAGE
                    | wgpu::BufferUsages::COPY_SRC
                    | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            })
        };
        let rb = make();
        let ib = make();
        let padded = n.next_power_of_two();
        let uniform = d.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: None,
            contents: bytemuck::cast_slice(&[beams, n, padded, padded.ilog2()]),
            usage: wgpu::BufferUsages::UNIFORM,
        });
        let buffers = [&uniform, &rb, &ib];
        let entries: Vec<_> = buffers
            .iter()
            .enumerate()
            .map(|(binding, b)| wgpu::BindGroupEntry {
                binding: binding as u32,
                resource: b.as_entire_binding(),
            })
            .collect();
        let bind = d.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &radix.get_bind_group_layout(0),
            entries: &entries,
        });
        let plan = if n.is_power_of_two() {
            None
        } else {
            Some(exact.plan(&d, n, beams, &rb, &ib))
        };
        let staging = d.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size: size * 2,
            usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
            mapped_at_creation: false,
        });
        // Reuse the same plan and buffers with three different inputs.
        for kind in 0..3 {
            let mut re = vec![0f32; count];
            let mut im = vec![0f32; count];
            for beam in 0..beams as usize {
                for j in 0..n as usize {
                    let idx = beam * n as usize + j;
                    match kind {
                        0 => {
                            if j == (beam + 1) % n as usize {
                                re[idx] = 1.0;
                            }
                        }
                        1 => {
                            let k = (n as usize / 3 + beam) % n as usize;
                            let phase = std::f64::consts::TAU * (k * j) as f64 / n as f64;
                            re[idx] = phase.cos() as f32;
                            im[idx] = phase.sin() as f32;
                        }
                        _ => {
                            re[idx] = (((idx as u64 * 37 + 11) % 101) as f32 - 50.0) / 51.0;
                            im[idx] = (((idx as u64 * 19 + 7) % 97) as f32 - 48.0) / 49.0;
                        }
                    }
                }
            }
            q.write_buffer(&rb, 0, bytemuck::cast_slice(&re));
            q.write_buffer(&ib, 0, bytemuck::cast_slice(&im));
            let mut enc = d.create_command_encoder(&Default::default());
            if let Some(plan) = &plan {
                exact.encode(&mut enc, plan);
            } else {
                let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                    label: None,
                    timestamp_writes: None,
                });
                pass.set_pipeline(&radix);
                pass.set_bind_group(0, &bind, &[]);
                pass.dispatch_workgroups(beams, 1, 1);
            }
            enc.copy_buffer_to_buffer(&rb, 0, &staging, 0, size);
            enc.copy_buffer_to_buffer(&ib, 0, &staging, size, size);
            q.submit(Some(enc.finish()));
            let (tx, rx) = mpsc::channel();
            staging
                .slice(..)
                .map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
            d.poll(wgpu::Maintain::Wait);
            rx.recv().unwrap().unwrap();
            let view = staging.slice(..).get_mapped_range();
            let actual: &[f32] = bytemuck::cast_slice(&view);
            let mut error = 0.0;
            let mut energy = 0.0;
            for beam in 0..beams as usize {
                for k in 0..n as usize {
                    let mut expected = [0.0f64; 2];
                    for j in 0..n as usize {
                        let phase = -std::f64::consts::TAU * (k * j) as f64 / n as f64;
                        let (s, c) = phase.sin_cos();
                        let idx = beam * n as usize + j;
                        expected[0] += re[idx] as f64 * c - im[idx] as f64 * s;
                        expected[1] += re[idx] as f64 * s + im[idx] as f64 * c;
                    }
                    let idx = beam * n as usize + k;
                    for component in 0..2 {
                        let value = actual[component * count + idx] as f64;
                        assert!(value.is_finite(), "n={n}, kind={kind}");
                        error += (value - expected[component]).powi(2);
                        energy += expected[component].powi(2);
                    }
                }
                if kind == 1 {
                    let peak = (0..n as usize)
                        .max_by(|&a, &b| {
                            let power = |k: usize| {
                                let idx = beam * n as usize + k;
                                actual[idx].powi(2) + actual[count + idx].powi(2)
                            };
                            power(a).total_cmp(&power(b))
                        })
                        .unwrap();
                    assert_eq!(
                        peak,
                        (n as usize / 3 + beam) % n as usize,
                        "tone peak n={n}"
                    );
                }
            }
            let relative_l2 = (error / energy.max(1e-30)).sqrt();
            worst = worst.max(relative_l2);
            assert!(
                relative_l2 < 2e-5,
                "n={n}, kind={kind}, relative_l2={relative_l2}"
            );
            drop(view);
            staging.unmap();
            cases += 1;
        }
    }
    eprintln!("cases={cases}, worst_relative_l2={worst}");
}
