//! C-FFI interface, Buffer initialization and write ups and reads + Bind groups created + encoder setup + memory free
//! + Compute passes for each shader

pub(crate) mod exact_fft;
mod pipeline; // GPU context + buffer management

use bytemuck::{Pod, Zeroable};
use std::fs::OpenOptions;
use std::io::Write;
use std::sync::atomic::{AtomicU64, Ordering};
use wgpu::util::DeviceExt;

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct BackscatterParams {
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    ray_skips: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    sensor_gain: f32,
    bandwidth: f32,
    seed_lo: u32,
    seed_hi: u32,
    frame_lo: u32, // WGSL has no native u64 type.
    frame_hi: u32, // Split frame index into two u32 - frame_lo and frame_hi
    area_scaler: f32,
    h_fov: f32,
    v_fov: f32,
    use_area_weights: u32,
    _pad1: u32,
    _pad2: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct MatmulParams {
    n_beams: u32,
    n_freq: u32,
    beam_corrector_sum: f32,
    n_outputs: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct FftParams {
    n_beams: u32,
    n_freq: u32,   // actual number of frequency bins (may not be power-of-2)
    padded_n: u32, // next power of 2 >= n_freq (actual FFT size, <= 4096)
    log2_n: u32,   // log2(padded_n)
}

fn emit_debug_timing(line: &str) {
    eprintln!("{line}");

    if let Ok(mut file) = OpenOptions::new()
        .create(true)
        .append(true)
        .open("wgpu_debug_timings.txt")
    {
        let _ = writeln!(file, "{line}");
        let _ = file.flush();
    }
}

/// The function receives flat arrays from C++ and returns a heap-allocated array.
///
/// # Safety
///
/// Each input pointer must be null or valid for reads of its full length:
/// `depth_flat` and `reflectivity_flat` `n_beams * n_rays` floats, `normal_flat`
/// `n_beams * n_rays * 3`, `beam_corrector_flat` `n_beams * n_beams` and `window_flat` `n_freq`.
/// Frequency lengths outside 1..=4096 are rejected before allocating GPU buffers.
/// A non-null result holds `n_beams * n_freq * 2` floats and must be released with
/// [`sonar_wgpu_free`].
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_compute(
    depth_flat: *const f32,
    normal_flat: *const f32,
    reflectivity_flat: *const f32,
    beam_corrector_flat: *const f32,
    window_flat: *const f32,
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    ray_skips: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    sensor_gain: f32,
    bandwidth: f32,
    beam_corrector_sum: f32,
    area_scaler: f32,
    h_fov: f32,
    v_fov: f32,
    frame_index: u64,
    seed: u64,
    debug_flag: bool,
) -> *mut f32 {
    compute_impl(
        depth_flat,
        normal_flat,
        reflectivity_flat,
        beam_corrector_flat,
        window_flat,
        n_beams,
        n_rays,
        n_freq,
        ray_skips,
        sound_speed,
        max_distance,
        source_level,
        attenuation,
        sensor_gain,
        bandwidth,
        beam_corrector_sum,
        area_scaler,
        h_fov,
        v_fov,
        frame_index,
        seed,
        debug_flag,
        n_beams,
        std::ptr::null(),
    )
}

/// Physical readout with independent geometry/output dimensions and cell solid angles.
///
/// # Safety
/// depth, reflectivity and angular_area have n_geometry*n_selected_rays floats;
/// normal has three times that length, pressure_response has n_receivers*n_geometry,
/// and window has n_freq. All pointers must be valid for those lengths.
/// window is dimensionless B*S(f), sampled at B*((j+0.5)/n_freq-0.5).
/// A unit-mean window has unit time-domain peak; normalization is 1/n_freq.
/// The result is complex baseband pressure at t=k/B, including the full guard.
/// The result has n_receivers*n_freq*2 floats, released with sonar_wgpu_free.
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_compute_v2(
    depth: *const f32,
    normal: *const f32,
    reflectivity: *const f32,
    angular_area: *const f32,
    pressure_response: *const f32,
    window: *const f32,
    n_geometry: u32,
    n_selected_rays: u32,
    n_receivers: u32,
    n_freq: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    receive_gain: f32,
    bandwidth: f32,
    frame_index: u64,
    seed: u64,
    debug_flag: bool,
) -> *mut f32 {
    // This integration includes the #34 exact-N transform for odd lengths.
    if angular_area.is_null() {
        return std::ptr::null_mut();
    }
    compute_impl(
        depth,
        normal,
        reflectivity,
        pressure_response,
        window,
        n_geometry,
        n_selected_rays,
        n_freq,
        1,
        sound_speed,
        max_distance,
        source_level,
        attenuation,
        receive_gain,
        bandwidth,
        1.0,
        0.0,
        0.0,
        0.0,
        frame_index,
        seed,
        debug_flag,
        n_receivers,
        angular_area,
    )
}

#[allow(clippy::too_many_arguments)]
unsafe fn compute_impl(
    depth_flat: *const f32,
    normal_flat: *const f32,
    reflectivity_flat: *const f32,
    beam_corrector_flat: *const f32,
    window_flat: *const f32,
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    ray_skips: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    sensor_gain: f32,
    bandwidth: f32,
    beam_corrector_sum: f32,
    area_scaler: f32,
    h_fov: f32,
    v_fov: f32,
    frame_index: u64,
    seed: u64,
    debug_flag: bool,
    n_outputs: u32,
    angular_area_flat: *const f32,
) -> *mut f32 {
    if depth_flat.is_null()
        || normal_flat.is_null()
        || reflectivity_flat.is_null()
        || beam_corrector_flat.is_null()
        || window_flat.is_null()
    {
        return std::ptr::null_mut();
    }

    let n_beams_us = n_beams as usize;
    let n_rays_us = n_rays as usize;
    let n_freq_us = n_freq as usize;

    if n_beams_us == 0 || n_rays_us == 0 || n_outputs == 0 || !(1..=4096).contains(&n_freq) {
        return std::ptr::null_mut();
    }

    use std::sync::Once;

    // Validate products before pointer arithmetic, WGSL u32 indexing or allocation.
    let lengths = (
        n_beams_us.checked_mul(n_rays_us),
        n_beams_us.checked_mul(n_outputs as usize),
        n_beams_us.checked_mul(n_freq_us),
        (n_outputs as usize).checked_mul(n_freq_us),
    );
    let (Some(depth_len), Some(bc_len), Some(spectrum_len), Some(output_len)) = lengths else {
        return std::ptr::null_mut();
    };
    let Some(normal_len) = depth_len.checked_mul(3) else {
        return std::ptr::null_mut();
    };
    if [normal_len, bc_len, spectrum_len, output_len]
        .iter()
        .any(|&n| n > (u32::MAX as usize / 4).min(isize::MAX as usize / 8))
    {
        return std::ptr::null_mut();
    }
    let refl_len = depth_len;
    let physical = !angular_area_flat.is_null();
    if physical
        && (!sound_speed.is_finite()
            || sound_speed <= 0.0
            || !max_distance.is_finite()
            || max_distance <= 0.0
            || !source_level.is_finite()
            || !attenuation.is_finite()
            || attenuation < 0.0
            || !sensor_gain.is_finite()
            || sensor_gain < 0.0
            || !bandwidth.is_finite()
            || bandwidth <= 0.0
            || f64::from(max_distance)
                >= f64::from(n_freq) * f64::from(sound_speed) / (2.0 * f64::from(bandwidth)))
    {
        return std::ptr::null_mut();
    }

    let depth = unsafe { std::slice::from_raw_parts(depth_flat, depth_len) };
    let normals = unsafe { std::slice::from_raw_parts(normal_flat, normal_len) };
    let reflectivity = unsafe { std::slice::from_raw_parts(reflectivity_flat, refl_len) };
    let beam_corrector = unsafe { std::slice::from_raw_parts(beam_corrector_flat, bc_len) };

    let ctx = match pipeline::get_or_init() {
        Some(c) => {
            static GPU_LOG: Once = Once::new();
            GPU_LOG.call_once(|| eprintln!("[sonar_wgpu] GPU device acquired -> running on GPU."));
            c
        }
        None => {
            static FALLBACK_LOG: Once = Once::new();
            FALLBACK_LOG.call_once(|| eprintln!("[sonar_wgpu] GPU init failed -> returning null."));
            return std::ptr::null_mut();
        }
    };
    let device = &ctx.device;
    let queue = &ctx.queue;
    let limits = device.limits();
    let byte_limit = u64::from(limits.max_storage_buffer_binding_size).min(limits.max_buffer_size);
    if [normal_len, bc_len, spectrum_len, output_len]
        .iter()
        .any(|&n| n as u64 * 4 > byte_limit)
        || (spectrum_len as u32).div_ceil(64) > limits.max_compute_workgroups_per_dimension
        || n_outputs > limits.max_compute_workgroups_per_dimension
        || n_beams.div_ceil(8) > limits.max_compute_workgroups_per_dimension
        || n_rays.div_ceil(8) > limits.max_compute_workgroups_per_dimension
    {
        return std::ptr::null_mut();
    }

    static GPU_FRAME_COUNT: AtomicU64 = AtomicU64::new(0);
    let gpu_frame = GPU_FRAME_COUNT.fetch_add(1, Ordering::Relaxed);
    let gpu_t0 = std::time::Instant::now();

    let window_vals = unsafe { std::slice::from_raw_parts(window_flat, n_freq_us) };
    let areas = if physical {
        unsafe { std::slice::from_raw_parts(angular_area_flat, depth_len) }
    } else {
        &[]
    };
    if physical
        && (window_vals.iter().any(|v| !v.is_finite())
            || beam_corrector.iter().any(|v| !v.is_finite())
            || areas.iter().any(|v| !v.is_finite() || *v < 0.0)
            || depth.iter().enumerate().any(|(i, &d)| {
                d > 0.0
                    && d <= max_distance
                    && (!normals[3 * i + 2].is_finite()
                        || normals[3 * i + 2].abs() > 1.00001
                        || !reflectivity[i].is_finite()
                        || reflectivity[i] < 0.0)
            }))
    {
        return std::ptr::null_mut();
    }

    // ---- Per-frame uniform buffers (tiny, always recreated) ----
    let backscatter_params = BackscatterParams {
        n_beams,
        n_rays,
        n_freq,
        ray_skips,
        sound_speed,
        max_distance,
        source_level,
        attenuation,
        sensor_gain,
        bandwidth,
        seed_lo: (seed & 0xFFFF_FFFF) as u32,
        seed_hi: (seed >> 32) as u32,
        frame_lo: (frame_index & 0xFFFF_FFFF) as u32,
        frame_hi: (frame_index >> 32) as u32,
        area_scaler,
        h_fov,
        v_fov,
        use_area_weights: u32::from(physical),
        _pad1: 0,
        _pad2: 0,
    };
    let bs_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("bs_param"),
        contents: bytemuck::bytes_of(&backscatter_params),
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
    });
    let mm_params = MatmulParams {
        n_beams,
        n_freq,
        beam_corrector_sum,
        n_outputs,
    };
    let mm_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("mm_param"),
        contents: bytemuck::bytes_of(&mm_params),
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
    });

    // REMOVE THIS, UNNCESSARY
    // ---- Persistent data buffers: allocate once per dimension set, reuse every frame ----
    // The lock is held for the whole frame, so sonars with equal dimensions can share a set.
    let mut buf_guard = ctx.buffers.lock().unwrap();
    let key = (n_beams, n_rays, n_freq, n_outputs);
    if !buf_guard.contains_key(&key) {
        if buf_guard.len() >= pipeline::MAX_BUFFER_SETS {
            buf_guard.clear();
        }
        buf_guard.insert(
            key,
            pipeline::SonarBuffers::new(device, n_beams, n_rays, n_freq, n_outputs, &ctx.exact_fft),
        );
        eprintln!(
            "[sonar_wgpu] Persistent buffers: {} geometry × {} rows × {} frequencies -> {} receivers",
            n_beams, n_rays, n_freq, n_outputs
        );
    }
    let buf = &buf_guard[&key];

    // Upload frame data into persistent buffers (DMA, no GPU alloc)
    queue.write_buffer(&buf.depth_buf, 0, bytemuck::cast_slice(depth));
    queue.write_buffer(&buf.normal_buf, 0, bytemuck::cast_slice(normals));
    queue.write_buffer(&buf.refl_buf, 0, bytemuck::cast_slice(reflectivity));
    queue.write_buffer(&buf.window_buf, 0, bytemuck::cast_slice(window_vals));
    queue.write_buffer(&buf.bc_buf, 0, bytemuck::cast_slice(beam_corrector));
    if physical {
        queue.write_buffer(&buf.area_buf, 0, bytemuck::cast_slice(areas));
    }
    // Clear inactive/invalid ray amplitudes in the persistent buffers.
    let zeros_amp = vec![0.0f32; n_beams_us * n_rays_us];
    queue.write_buffer(&buf.amp_re, 0, bytemuck::cast_slice(&zeros_amp));
    queue.write_buffer(&buf.amp_im, 0, bytemuck::cast_slice(&zeros_amp));

    // ---- Bind groups (metadata only -> no GPU allocation) ----
    let bs_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("bs_bg"),
        layout: &ctx.bs_bgl,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: bs_param_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: buf.depth_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: buf.normal_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: buf.refl_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 4,
                resource: buf.amp_re.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 5,
                resource: buf.amp_im.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 7,
                resource: buf.area_buf.as_entire_binding(),
            },
        ],
    });
    let conv_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("conv_bg"),
        layout: &ctx.convert_bgl,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: buf.amp_re.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: buf.amp_im.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: buf.mm_re_in.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: buf.mm_im_in.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 4,
                resource: buf.depth_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 5,
                resource: buf.window_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 6,
                resource: bs_param_buf.as_entire_binding(),
            },
        ],
    });
    let mm_re_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("mm_re_bg"),
        layout: &ctx.mm_bgl,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: mm_param_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: buf.bc_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: buf.mm_re_in.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: buf.mm_re_out.as_entire_binding(),
            },
        ],
    });
    let mm_im_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("mm_im_bg"),
        layout: &ctx.mm_bgl,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: mm_param_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: buf.bc_buf.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: buf.mm_im_in.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: buf.mm_im_out.as_entire_binding(),
            },
        ],
    });

    let spectrum_bytes = (output_len * std::mem::size_of::<f32>()) as u64;
    // Radix-2 path is used only for power-of-two lengths.
    let padded_n = n_freq_us.next_power_of_two() as u32;

    // ---- Single command encoder: all passes + staging copy in one GPU submission ----
    let mut enc = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("sonar_enc"),
    });

    // Backscatter dispatch: geometry/material → per-ray complex amplitude
    {
        let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("bs_pass"),
            timestamp_writes: None,
        });
        pass.set_pipeline(&ctx.bs_pipeline);
        pass.set_bind_group(0, &bs_bg, &[]);
        let gx = n_beams.div_ceil(8);
        let ray_step = std::cmp::max(1, ray_skips);
        let reduced_rays = n_rays.div_ceil(ray_step);
        let gy = reduced_rays.div_ceil(8);
        pass.dispatch_workgroups(gx, gy, 1);
    }

    // Synthesize the frequency spectrum by floating-point ray summation.
    {
        let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("conv_pass"),
            timestamp_writes: None,
        });
        pass.set_pipeline(&ctx.convert_pipeline);
        pass.set_bind_group(0, &conv_bg, &[]);
        let gx = (spectrum_len as u32).div_ceil(64);
        pass.dispatch_workgroups(gx, 1, 1);
    }

    // Matmul dispatch: beam-corrector matrix multiply, re and im in same encoder
    {
        let gx = n_freq.div_ceil(16);
        let gy = n_outputs.div_ceil(16);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("mm_re_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.mm_pipeline);
            pass.set_bind_group(0, &mm_re_bg, &[]);
            pass.dispatch_workgroups(gx, gy, 1);
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("mm_im_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.mm_pipeline);
            pass.set_bind_group(0, &mm_im_bg, &[]);
            pass.dispatch_workgroups(gx, gy, 1);
        }
    }

    // Both paths evaluate the original N-point forward DFT on the GPU.
    // Copy MM output -> FFT working buffers -> dispatch FFT shader -> copy to staging.
    enc.copy_buffer_to_buffer(&buf.mm_re_out, 0, &buf.p_re_buf, 0, spectrum_bytes);
    enc.copy_buffer_to_buffer(&buf.mm_im_out, 0, &buf.p_im_buf, 0, spectrum_bytes);

    if let Some(plan) = &buf.exact_fft {
        ctx.exact_fft.encode(&mut enc, plan);
    } else {
        let fft_params = FftParams {
            n_beams: n_outputs,
            n_freq,
            padded_n,
            log2_n: padded_n.ilog2(),
        };
        let fft_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("fft_param"),
            contents: bytemuck::bytes_of(&fft_params),
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        });
        let fft_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("fft_bg"),
            layout: &ctx.fft_bgl,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: fft_param_buf.as_entire_binding(),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: buf.p_re_buf.as_entire_binding(),
                },
                wgpu::BindGroupEntry {
                    binding: 2,
                    resource: buf.p_im_buf.as_entire_binding(),
                },
            ],
        });
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("fft_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.fft_pipeline);
            pass.set_bind_group(0, &fft_bg, &[]);
            pass.dispatch_workgroups(n_outputs, 1, 1);
        }
    }
    // Copy FFT output to staging for CPU readback.
    enc.copy_buffer_to_buffer(&buf.p_re_buf, 0, &buf.stg_re, 0, spectrum_bytes);
    enc.copy_buffer_to_buffer(&buf.p_im_buf, 0, &buf.stg_im, 0, spectrum_bytes);

    // Submit the entire frame in a single GPU command -> POLL 1
    queue.submit(std::iter::once(enc.finish()));
    device.poll(wgpu::Maintain::Wait);

    // map_async is asynchronous -> it requests CPU access to the buffer but doesn't block.
    // Map staging buffers (GPU work already done above) -> POLL 2
    let re_ok = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));
    let im_ok = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));
    let re_ok_cb = std::sync::Arc::clone(&re_ok);
    let im_ok_cb = std::sync::Arc::clone(&im_ok);

    buf.stg_re
        .slice(..)
        .map_async(wgpu::MapMode::Read, move |r| {
            re_ok_cb.store(r.is_ok(), std::sync::atomic::Ordering::Release);
        });
    buf.stg_im
        .slice(..)
        .map_async(wgpu::MapMode::Read, move |r| {
            im_ok_cb.store(r.is_ok(), std::sync::atomic::Ordering::Release);
        });
    device.poll(wgpu::Maintain::Wait);

    let re_mapped = re_ok.load(std::sync::atomic::Ordering::Acquire);
    let im_mapped = im_ok.load(std::sync::atomic::Ordering::Acquire);
    if !re_mapped || !im_mapped {
        eprintln!(
            "[sonar_wgpu] map_async failed: stg_re_ok={}, stg_im_ok={}",
            re_mapped, im_mapped
        );
        buf.stg_re.unmap();
        buf.stg_im.unmap();
        return std::ptr::null_mut();
    }

    let (mut p_re, mut p_im) = {
        let re_data = buf.stg_re.slice(..).get_mapped_range();
        let im_data = buf.stg_im.slice(..).get_mapped_range();
        let re = bytemuck::cast_slice::<u8, f32>(&re_data).to_vec();
        let im = bytemuck::cast_slice::<u8, f32>(&im_data).to_vec();
        (re, im)
    };
    buf.stg_re.unmap();
    buf.stg_im.unmap();

    // Physical v2 integrates dimensionless B*S(f) with 1/M, independent of
    // acquisition length. Undo the midpoint frequency origin after the DFT.
    // Legacy v1 retains its historical delta_f scaling and phase convention.
    for i in 0..output_len {
        if physical {
            let k = i % n_freq_us;
            let phase = -std::f32::consts::PI * k as f32 / n_freq as f32;
            let sign = if k % 2 == 0 { 1.0 } else { -1.0 };
            let (s, c) = phase.sin_cos();
            let re = p_re[i];
            let im = p_im[i];
            p_re[i] = (re * c - im * s) * sign / n_freq as f32;
            p_im[i] = (re * s + im * c) * sign / n_freq as f32;
        } else {
            let delta_f = bandwidth / n_freq as f32;
            p_re[i] *= delta_f;
            p_im[i] *= delta_f;
        }
    }

    // Pack into interleaved [re0, im0, re1, im1, ...] for C++ caller
    let mut out = vec![0.0f32; output_len * 2];
    for b in 0..n_outputs as usize {
        for f in 0..n_freq_us {
            let idx = b * n_freq_us + f;
            out[idx * 2] = p_re[idx];
            out[idx * 2 + 1] = p_im[idx];
        }
    }

    let elapsed_ms = gpu_t0.elapsed().as_secs_f64() * 1000.0;
    if debug_flag {
        emit_debug_timing(&format!(
            "Total WGPU Calculation Wrapper Time: {:.3} ms",
            elapsed_ms
        ));
    }

    // Log on the very first frame and then every 50 frames.
    if gpu_frame == 0 || gpu_frame % 50 == 49 {
        let active_rays = n_rays.div_ceil(std::cmp::max(1, ray_skips));
        eprintln!(
            "[sonar_wgpu] GPU #{:<5} | {:6.1} ms | {} geometry columns × {} selected rays × {} freq -> {} receivers",
            gpu_frame + 1,
            elapsed_ms,
            n_beams,
            active_rays,
            n_freq,
            n_outputs
        );
    }

    let mut boxed = out.into_boxed_slice();
    let ptr = boxed.as_mut_ptr();
    std::mem::forget(boxed);
    ptr
}

/// Frees an array returned by [`sonar_wgpu_compute`].
///
/// # Safety
///
/// `ptr` must be null or a pointer returned by `sonar_wgpu_compute` that has not been freed yet,
/// and `len` must be the length of that array (`n_beams * n_freq * 2`).
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_free(ptr: *mut f32, len: usize) {
    if ptr.is_null() || len == 0 {
        return;
    }
    unsafe {
        let _ = Vec::from_raw_parts(ptr, len, len);
    }
}

#[cfg(test)]
mod input_length_tests {
    use super::sonar_wgpu_compute;

    #[test]
    fn unsupported_lengths_are_rejected_before_gpu_initialization() {
        let depth = [1.0f32];
        let normal = [1.0f32, 0.0, 0.0];
        let reflectivity = [1.0f32];
        let corrector = [1.0f32];
        for n_freq in [0u32, 4097, 5000, 8192] {
            let window = vec![1.0f32; n_freq as usize];
            // Each pointer is valid for its documented length. Rejection must
            // happen before an adapter is requested or GPU buffers are allocated.
            let result = unsafe {
                sonar_wgpu_compute(
                    depth.as_ptr(),
                    normal.as_ptr(),
                    reflectivity.as_ptr(),
                    corrector.as_ptr(),
                    window.as_ptr(),
                    1,
                    1,
                    n_freq,
                    1,
                    1500.0,
                    10.0,
                    220.0,
                    0.0,
                    0.02,
                    29900.0,
                    1.0,
                    1.0,
                    1.0,
                    1.0,
                    0,
                    7,
                    false,
                )
            };
            assert!(result.is_null(), "unsupported frequency length {n_freq}");
        }
    }
    #[test]
    #[ignore = "requires a WGPU compute adapter"]
    fn valid_lengths_still_compute_after_rejected_input() {
        let depth = [1.0f32];
        let normal = [1.0f32, 0.0, 0.0];
        let reflectivity = [1.0f32];
        let corrector = [1.0f32];
        // Exercise the production FFI, plan cache, invalid-input recovery and both FFT routes.
        for n_freq in [1u32, 399, 4095, 4096, 4097, 399, 512] {
            let window = vec![1.0f32; n_freq as usize];
            let result = unsafe {
                sonar_wgpu_compute(
                    depth.as_ptr(),
                    normal.as_ptr(),
                    reflectivity.as_ptr(),
                    corrector.as_ptr(),
                    window.as_ptr(),
                    1,
                    1,
                    n_freq,
                    1,
                    1500.0,
                    10.0,
                    220.0,
                    0.0,
                    0.02,
                    29900.0,
                    1.0,
                    1.0,
                    1.0,
                    1.0,
                    0,
                    7,
                    false,
                )
            };
            if n_freq > 4096 {
                assert!(result.is_null());
            } else {
                assert!(
                    !result.is_null(),
                    "supported length {n_freq}; GPU must be available"
                );
                let values = unsafe { std::slice::from_raw_parts(result, n_freq as usize * 2) };
                let finite = values.iter().all(|v| v.is_finite());
                unsafe {
                    super::sonar_wgpu_free(result, n_freq as usize * 2);
                }
                assert!(finite, "non-finite output at length {n_freq}");
            }
        }
    }
}
