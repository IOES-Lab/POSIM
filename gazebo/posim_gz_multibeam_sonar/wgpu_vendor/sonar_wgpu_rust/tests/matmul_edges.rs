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

struct GpuMatmul {
    device: wgpu::Device,
    queue: wgpu::Queue,
    pipeline: wgpu::ComputePipeline,
}

async fn create_gpu() -> GpuMatmul {
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
    GpuMatmul {
        device,
        queue,
        pipeline,
    }
}

impl GpuMatmul {
    fn run(&self, beams: u32, freq: u32, norm: f32, a: &[f32], b: &[f32]) -> Vec<f32> {
        assert_eq!(a.len(), (beams * beams) as usize);
        assert_eq!(b.len(), (beams * freq) as usize);
        let uniform = self
            .device
            .create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&[beams, freq, norm.to_bits()]),
                usage: wgpu::BufferUsages::UNIFORM,
            });
        let storage = |v: &[f32], usage| {
            self.device
                .create_buffer_init(&wgpu::util::BufferInitDescriptor {
                    label: None,
                    contents: bytemuck::cast_slice(v),
                    usage: wgpu::BufferUsages::STORAGE | usage,
                })
        };
        let ab = storage(a, wgpu::BufferUsages::empty());
        let bb = storage(b, wgpu::BufferUsages::empty());
        let count = (beams * freq) as usize;
        // Poison the output to detect skipped stores. Include a trailing guard
        // large enough for every invocation in the padded dispatch, so an
        // invalid store is observable instead of hidden by robust buffer access.
        const SENTINEL: f32 = -1234567.0;
        let initial = vec![SENTINEL; count + 16 * (freq as usize + 1)];
        let out = storage(&initial, wgpu::BufferUsages::COPY_SRC);
        let size = initial.len() as u64 * 4;
        let buffers = [&uniform, &ab, &bb, &out];
        let entries: Vec<_> = buffers
            .iter()
            .enumerate()
            .map(|(binding, buffer)| wgpu::BindGroupEntry {
                binding: binding as u32,
                resource: buffer.as_entire_binding(),
            })
            .collect();
        let bind = self.device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &self.pipeline.get_bind_group_layout(0),
            entries: &entries,
        });
        let staging = self.device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size,
            usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
            mapped_at_creation: false,
        });
        let mut encoder = self.device.create_command_encoder(&Default::default());
        {
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: None,
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.pipeline);
            pass.set_bind_group(0, &bind, &[]);
            pass.dispatch_workgroups(freq.div_ceil(16), beams.div_ceil(16), 1);
        }
        encoder.copy_buffer_to_buffer(&out, 0, &staging, 0, size);
        self.queue.submit(Some(encoder.finish()));
        let (tx, rx) = mpsc::channel();
        staging
            .slice(..)
            .map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
        self.device.poll(wgpu::Maintain::Wait);
        rx.recv().unwrap().unwrap();
        let view = staging.slice(..).get_mapped_range();
        let values: &[f32] = bytemuck::cast_slice(&view);
        assert!(
            values[count..].iter().all(|&v| v == SENTINEL),
            "output guard overwritten: beams={beams}, freq={freq}"
        );
        let actual = values[..count].to_vec();
        drop(view);
        staging.unmap();
        actual
    }
}

async fn check() {
    let gpu = create_gpu().await;
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
                let actual = gpu.run(beams, freq, norm, &a, &b);
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
                let exact_mismatches = if ones {
                    actual
                        .iter()
                        .filter(|&&value| value != beams as f32)
                        .count()
                } else {
                    0
                };
                eprintln!("grid beams={beams}, freq={freq}, ones={ones}, relative_l2={relative_l2}, exact_mismatches={exact_mismatches}");
                if beams == 17 && freq == 17 && ones {
                    eprintln!(
                        "17x17 ones: last row first={}, expected=17, mismatched_elements={exact_mismatches}/289",
                        actual[16 * 17],
                    );
                }
                if !finite || relative_l2 > 2e-5 || exact_mismatches != 0 {
                    failures.push(format!(
                        "beams={beams}, freq={freq}, ones={ones}, relative_l2={relative_l2}"
                    ));
                }
                cases += 1;
            }
        }
    }
    eprintln!("cases={cases}, failures={}", failures.len());
    assert!(failures.is_empty(), "{}", failures.join("\n"));
}

#[test]
#[ignore = "requires a WGPU compute adapter"]
fn matmul_boundary_elements_are_exact() {
    pollster::block_on(check_exact_boundaries());
}

async fn check_exact_boundaries() {
    let gpu = create_gpu().await;
    let mut failures = Vec::new();
    let mut cases = 0;
    // (16,17) isolates the A-load column boundary; (17,16) isolates the
    // B-load row boundary. (17,17) crosses both. Square cases alone are
    // insufficient to establish that both input-load guards were fixed.
    let shapes = [
        (1u32, 1u32),
        (3, 1),
        (15, 15),
        (16, 16),
        (16, 17),
        (17, 16),
        (17, 17),
        (31, 33),
        (32, 32),
        (33, 31),
        (65, 17),
        (17, 65),
    ];
    for repeat in 0..3 {
        for (beams, freq) in shapes {
            for pattern in ["ones", "identity", "zero_a", "zero_b", "sparse", "signed"] {
                let mut a: Vec<f32> = (0..beams * beams)
                    .map(|i| ((i * 37 + 11) % 11) as f32 - 5.0)
                    .collect();
                let mut b: Vec<f32> = (0..beams * freq)
                    .map(|i| ((i * 19 + 7) % 13) as f32 - 6.0)
                    .collect();
                let norm = if pattern == "ones" { 1.0 } else { 2.0 };
                match pattern {
                    "ones" => {
                        a.fill(1.0);
                        b.fill(1.0);
                    }
                    "identity" => {
                        a.fill(0.0);
                        for row in 0..beams as usize {
                            a[row * beams as usize + row] = 1.0;
                        }
                    }
                    "zero_a" => a.fill(0.0),
                    "zero_b" => b.fill(0.0),
                    "sparse" => {
                        a.fill(0.0);
                        // For 17 beams, k=15 specifically requires loads by
                        // invocations with invalid outputs in the edge tiles.
                        let k = beams.saturating_sub(2) as usize;
                        for row in 0..beams as usize {
                            a[row * beams as usize + k] = (row % 7) as f32 - 3.0;
                        }
                    }
                    "signed" => (),
                    _ => unreachable!(),
                }
                let actual = gpu.run(beams, freq, norm, &a, &b);
                let mut bad = 0;
                let mut first = None;
                // Small integer operands and power-of-two normalization make
                // every product, sum, and result exactly representable in f32.
                // Check every element, not just a global relative norm, which
                // can dilute a localized error or be undefined for zero output.
                for row in 0..beams as usize {
                    for col in 0..freq as usize {
                        let expected = (0..beams as usize)
                            .map(|k| {
                                a[row * beams as usize + k] as f64
                                    * b[k * freq as usize + col] as f64
                            })
                            .sum::<f64>()
                            / norm as f64;
                        let value = actual[row * freq as usize + col];
                        if value as f64 != expected {
                            bad += 1;
                            first.get_or_insert((row, col, value, expected));
                        }
                    }
                }
                eprintln!("exact repeat={repeat}, beams={beams}, freq={freq}, pattern={pattern}, mismatched_elements={bad}");
                if let Some(first) = first {
                    failures.push(format!("repeat={repeat}, beams={beams}, freq={freq}, pattern={pattern}, bad={bad}, first(row,col,actual,expected)={first:?}"));
                }
                cases += 1;
            }
        }
    }
    eprintln!("exact_cases={cases}, failures={}", failures.len());
    assert!(failures.is_empty(), "{}", failures.join("\n"));
}
