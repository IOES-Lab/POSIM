//! Opt-in GPU regression. Run: cargo test --test matmul_edges -- --ignored --nocapture
//! Set WGPU_BACKEND=metal/vulkan/dx12 to select a backend. Missing GPU is a failure,
//! not a silent skip, when explicitly running this test.
use std::sync::mpsc;
use wgpu::util::DeviceExt;

#[test]
#[ignore = "requires a WGPU compute adapter"]
fn matmul_partial_tiles_match_cpu() {
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
    let (device, queue) = adapter
        .request_device(
            &wgpu::DeviceDescriptor {
                label: None,
                required_features: wgpu::Features::empty(),
                required_limits: wgpu::Limits::default(),
            },
            None,
        )
        .await
        .unwrap();
    let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("production matmul"),
        source: wgpu::ShaderSource::Wgsl(include_str!("../src/shaders/matmul.wgsl").into()),
    });
    let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: None,
        layout: None,
        module: &shader,
        entry_point: "main",
        compilation_options: Default::default(),
    });
    let mut failures = Vec::new();
    let mut cases = 0;
    for beams in [1u32, 3, 7, 15, 16, 17, 31, 32, 33, 64, 65, 513] {
        for freq in [3u32, 15, 16, 17, 31, 32, 399, 511, 512] {
            for ones in [true, false] {
                let norm = if ones { 1.0f32 } else { 2.5f32 };
                // Deterministic non-symmetric inputs: catch indexing and transpose mistakes.
                let a: Vec<f32> = (0..beams * beams)
                    .map(|i| {
                        if ones {
                            1.0
                        } else {
                            (((i as u64 * 37 + 11) % 101) as f32 - 50.0) / 51.0
                        }
                    })
                    .collect();
                let b: Vec<f32> = (0..beams * freq)
                    .map(|i| {
                        if ones {
                            1.0
                        } else {
                            (((i as u64 * 19 + 7) % 97) as f32 - 48.0) / 49.0
                        }
                    })
                    .collect();
                let uniform = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                    label: None,
                    contents: bytemuck::cast_slice(&[beams, freq, norm.to_bits()]),
                    usage: wgpu::BufferUsages::UNIFORM,
                });
                let storage = |v: &[f32]| {
                    device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                        label: None,
                        contents: bytemuck::cast_slice(v),
                        usage: wgpu::BufferUsages::STORAGE,
                    })
                };
                let ab = storage(&a);
                let bb = storage(&b);
                let size = u64::from(beams) * u64::from(freq) * 4;
                let out = device.create_buffer(&wgpu::BufferDescriptor {
                    label: None,
                    size,
                    usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
                    mapped_at_creation: false,
                });
                let buffers = [&uniform, &ab, &bb, &out];
                let entries: Vec<_> = buffers
                    .iter()
                    .enumerate()
                    .map(|(binding, buffer)| wgpu::BindGroupEntry {
                        binding: binding as u32,
                        resource: buffer.as_entire_binding(),
                    })
                    .collect();
                let bind = device.create_bind_group(&wgpu::BindGroupDescriptor {
                    label: None,
                    layout: &pipeline.get_bind_group_layout(0),
                    entries: &entries,
                });
                let staging = device.create_buffer(&wgpu::BufferDescriptor {
                    label: None,
                    size,
                    usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
                    mapped_at_creation: false,
                });
                let mut encoder = device.create_command_encoder(&Default::default());
                {
                    let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                        label: None,
                        timestamp_writes: None,
                    });
                    pass.set_pipeline(&pipeline);
                    pass.set_bind_group(0, &bind, &[]);
                    pass.dispatch_workgroups(freq.div_ceil(16), beams.div_ceil(16), 1);
                }
                encoder.copy_buffer_to_buffer(&out, 0, &staging, 0, size);
                queue.submit(Some(encoder.finish()));
                let (tx, rx) = mpsc::channel();
                staging
                    .slice(..)
                    .map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
                device.poll(wgpu::Maintain::Wait);
                rx.recv().unwrap().unwrap();
                let view = staging.slice(..).get_mapped_range();
                let actual: &[f32] = bytemuck::cast_slice(&view);
                let mut error = 0.0;
                let mut energy = 0.0;
                let mut finite = true;
                for row in 0..beams as usize {
                    for col in 0..freq as usize {
                        let expected = if ones {
                            beams as f64
                        } else {
                            (0..beams as usize)
                                .map(|k| {
                                    a[row * beams as usize + k] as f64
                                        * b[k * freq as usize + col] as f64
                                })
                                .sum::<f64>()
                                / norm as f64
                        };
                        let value = actual[row * freq as usize + col] as f64;
                        finite &= value.is_finite();
                        error += (value - expected).powi(2);
                        energy += expected.powi(2);
                    }
                }
                let relative_l2 = (error / energy.max(1e-30)).sqrt();
                if beams == 17 && freq == 17 && ones {
                    eprintln!(
                        "17x17 ones: last row first={}, expected=17",
                        actual[16 * 17]
                    );
                }
                if !finite || relative_l2 > 2e-5 {
                    failures.push(format!(
                        "beams={beams}, freq={freq}, ones={ones}, relative_l2={relative_l2}"
                    ));
                }
                drop(view);
                staging.unmap();
                cases += 1;
            }
        }
    }
    eprintln!("cases={cases}, failures={}", failures.len());
    assert!(failures.is_empty(), "{}", failures.join("\n"));
}
