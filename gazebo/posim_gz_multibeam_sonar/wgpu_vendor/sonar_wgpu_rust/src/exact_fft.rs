//! Bluestein's identity evaluates the original N-point forward DFT, not an
//! interpolated padded spectrum. Only dimension-dependent constants are built
//! on the CPU; every frame's signal, convolution and transform stay on the GPU.
use wgpu::util::DeviceExt;

pub struct Pipeline {
    pipeline: wgpu::ComputePipeline,
    layout: wgpu::BindGroupLayout,
}
pub struct Plan {
    passes: Vec<wgpu::BindGroup>,
    m: u32,
    beams: u32,
}

fn mul(a: [f64; 2], b: [f64; 2]) -> [f64; 2] {
    [a[0] * b[0] - a[1] * b[1], a[0] * b[1] + a[1] * b[0]]
}

/// Forward DIF transform: natural input, bit-reversed output. This table is
/// cached with the dimension-specific buffers, not recomputed per frame.
fn kernel_table(n: usize) -> Vec<[f32; 2]> {
    let m = (2 * n - 1).next_power_of_two();
    let chirp: Vec<[f64; 2]> = (0..n)
        .map(|i| {
            // Reduce before conversion: avoid large-angle f32 sin/cos errors.
            let angle =
                std::f64::consts::PI * ((i as u64 * i as u64) % (2 * n as u64)) as f64 / n as f64;
            [angle.cos(), -angle.sin()]
        })
        .collect();
    let roots: Vec<[f64; 2]> = (0..m / 2)
        .map(|i| {
            let angle = -std::f64::consts::TAU * i as f64 / m as f64;
            [angle.cos(), angle.sin()]
        })
        .collect();
    let mut kernel = vec![[0.0; 2]; m];
    kernel[0] = [1.0, 0.0];
    for i in 1..n {
        let value = [chirp[i][0], -chirp[i][1]];
        kernel[i] = value;
        kernel[m - i] = value;
    }
    let mut stride = m;
    while stride > 1 {
        let half = stride / 2;
        for base in (0..m).step_by(stride) {
            for j in 0..half {
                let a = kernel[base + j];
                let b = kernel[base + j + half];
                kernel[base + j] = [a[0] + b[0], a[1] + b[1]];
                kernel[base + j + half] = mul([a[0] - b[0], a[1] - b[1]], roots[j * (m / stride)]);
            }
        }
        stride /= 2;
    }
    chirp
        .into_iter()
        .chain(kernel)
        .chain(roots)
        .map(|v| [v[0] as f32, v[1] as f32])
        .collect()
}

impl Pipeline {
    pub fn new(device: &wgpu::Device) -> Self {
        let entries: Vec<_> = (0..6)
            .map(|binding| wgpu::BindGroupLayoutEntry {
                binding,
                visibility: wgpu::ShaderStages::COMPUTE,
                ty: wgpu::BindingType::Buffer {
                    ty: if binding == 0 {
                        wgpu::BufferBindingType::Uniform
                    } else {
                        wgpu::BufferBindingType::Storage {
                            read_only: binding == 5,
                        }
                    },
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            })
            .collect();
        let layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("bluestein_layout"),
            entries: &entries,
        });
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("bluestein"),
            source: wgpu::ShaderSource::Wgsl(include_str!("shaders/bluestein.wgsl").into()),
        });
        let pl = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("bluestein"),
            bind_group_layouts: &[&layout],
            push_constant_ranges: &[],
        });
        let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
            label: Some("bluestein"),
            layout: Some(&pl),
            module: &shader,
            entry_point: "main",
            compilation_options: Default::default(),
        });
        Self { pipeline, layout }
    }
    pub fn plan(
        &self,
        device: &wgpu::Device,
        n: u32,
        beams: u32,
        re: &wgpu::Buffer,
        im: &wgpu::Buffer,
    ) -> Plan {
        assert!((1..=4096).contains(&n) && !n.is_power_of_two() && beams > 0);
        let m = (2 * n - 1).next_power_of_two();
        let scratch = || {
            device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("bluestein_scratch"),
                size: u64::from(m) * u64::from(beams) * 4,
                usage: wgpu::BufferUsages::STORAGE,
                mapped_at_creation: false,
            })
        };
        let ar = scratch();
        let ai = scratch();
        let table = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("bluestein_constants"),
            contents: bytemuck::cast_slice(&kernel_table(n as usize)),
            usage: wgpu::BufferUsages::STORAGE,
        });
        let passes = (0..=2 * m.ilog2() + 2)
            .map(|phase| {
                let params = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                    label: Some("bluestein_params"),
                    contents: bytemuck::cast_slice(&[n, beams, m, phase]),
                    usage: wgpu::BufferUsages::UNIFORM,
                });
                let buffers = [&params, re, im, &ar, &ai, &table];
                let entries: Vec<_> = buffers
                    .iter()
                    .enumerate()
                    .map(|(binding, buffer)| wgpu::BindGroupEntry {
                        binding: binding as u32,
                        resource: buffer.as_entire_binding(),
                    })
                    .collect();
                device.create_bind_group(&wgpu::BindGroupDescriptor {
                    label: Some("bluestein_phase"),
                    layout: &self.layout,
                    entries: &entries,
                })
            })
            .collect();
        Plan { passes, m, beams }
    }
    pub fn encode(&self, encoder: &mut wgpu::CommandEncoder, plan: &Plan) {
        for bind in &plan.passes {
            // A workgroup barrier cannot synchronize independent workgroups.
            // Separate compute passes provide storage ordering between stages.
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("bluestein_stage"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.pipeline);
            pass.set_bind_group(0, bind, &[]);
            pass.dispatch_workgroups(plan.m.div_ceil(256), plan.beams, 1);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn cached_kernel_matches_direct_dft_and_chirp_symmetry() {
        for n in [3usize, 5, 7, 15, 31, 63, 127, 399, 511] {
            let m = (2 * n - 1).next_power_of_two();
            let table = kernel_table(n);
            assert_eq!(table.len(), n + m + m / 2);
            for k in [0usize, 1, m / 3, m / 2, m - 1] {
                let mut sum = [0.0, 0.0];
                for j in 0..m {
                    let t = if j < n {
                        j
                    } else if j > m - n {
                        m - j
                    } else {
                        continue;
                    };
                    let phase = std::f64::consts::PI * ((t * t) % (2 * n)) as f64 / n as f64
                        - std::f64::consts::TAU * (k * j) as f64 / m as f64;
                    sum[0] += phase.cos();
                    sum[1] += phase.sin();
                }
                let index = k.reverse_bits() >> (usize::BITS - m.ilog2());
                let got = table[n + index];
                assert!((f64::from(got[0]) - sum[0]).abs() < 1e-5);
                assert!((f64::from(got[1]) - sum[1]).abs() < 1e-5);
            }
        }
    }
}
