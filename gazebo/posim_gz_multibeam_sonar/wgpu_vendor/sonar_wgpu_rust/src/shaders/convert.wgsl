/// Convert per-ray complex amplitudes into the sampled frequency spectrum.
/// Each invocation owns one beam/frequency output and sums rays in a stable
/// order. This avoids integer quantization and needs no floating-point atomics.
struct Params {
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
  frame_lo: u32,
  frame_hi: u32,
  area_scaler: f32,
  h_fov: f32,
  v_fov: f32,
  _pad0: u32,
  _pad1: u32,
  _pad2: u32,
};

@group(0) @binding(0) var<storage, read> amplitude_re: array<f32>;
@group(0) @binding(1) var<storage, read> amplitude_im: array<f32>;
@group(0) @binding(2) var<storage, read_write> out_re: array<f32>;
@group(0) @binding(3) var<storage, read_write> out_im: array<f32>;
@group(0) @binding(4) var<storage, read> depth: array<f32>;
@group(0) @binding(5) var<storage, read> window_vals: array<f32>;
@group(0) @binding(6) var<uniform> params: Params;

const PI: f32 = 3.14159265358979323846;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out_re)) { return; }
    let beam = i / params.n_freq;
    let f = i % params.n_freq;
    let delta_f = params.bandwidth / f32(params.n_freq);
    let n = f32(params.n_freq);
    var freq: f32;
    if ((params.n_freq % 2u) == 0u) {
        freq = delta_f * (-n + 2.0 * (f32(f) + 1.0)) / 2.0;
    } else {
        freq = delta_f * (-(n - 1.0) + 2.0 * (f32(f) + 1.0)) / 2.0;
    }
    let kw = 2.0 * PI * freq / params.sound_speed;
    let ray_step = max(1u, params.ray_skips);
    var re: f32 = 0.0;
    var im: f32 = 0.0;
    for (var ray: u32 = 0u; ray < params.n_rays; ray = ray + ray_step) {
        let dr = beam * params.n_rays + ray;
        let d = depth[dr];
        if (d <= 0.0 || d > params.max_distance) { continue; }
        let ar = amplitude_re[dr];
        let ai = amplitude_im[dr];
        let phase = 2.0 * d * kw;
        let c = cos(phase);
        let s = sin(phase);
        re = re + (ar * c - ai * s);
        im = im + (ar * s + ai * c);
    }
    // The same aperture multiplies every ray and beam at this frequency.
    // Apply it in floating point before beam correction and the original-N FFT.
    out_re[i] = re * window_vals[f];
    out_im[i] = im * window_vals[f];
}
