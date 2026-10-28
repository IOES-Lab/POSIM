// Exact N-point forward DFT by Bluestein convolution, without range-grid interpolation.
// Global-memory radix-2 stages keep N <= 4096 within portable workgroup-memory limits.
// Each phase is a separate dispatch / compute pass: no cross-workgroup barrier assumed.
struct Params { n: u32, beams: u32, m: u32, phase: u32 };
@group(0) @binding(0) var<uniform> p: Params;
@group(0) @binding(1) var<storage, read_write> re: array<f32>;
@group(0) @binding(2) var<storage, read_write> im: array<f32>;
@group(0) @binding(3) var<storage, read_write> ar: array<f32>;
@group(0) @binding(4) var<storage, read_write> ai: array<f32>;
// [negative chirp N][bit-reversed forward FFT of positive chirp M][negative roots M/2]
@group(0) @binding(5) var<storage, read> table: array<vec2<f32>>;
fn mul(a: vec2<f32>, b: vec2<f32>) -> vec2<f32> {
  return vec2<f32>(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x);
}
@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
  let i=id.x; let beam=id.y;
  if (beam>=p.beams || i>=p.m) { return; }
  let base=beam*p.m;
  let levels=firstLeadingBit(p.m);
  if (p.phase==0u) {
    var a=vec2<f32>(0.0);
    if (i<p.n) { a=mul(vec2<f32>(re[beam*p.n+i],im[beam*p.n+i]),table[i]); }
    ar[base+i]=a.x; ai[base+i]=a.y;
  } else if (p.phase==levels+1u) {
    let a=mul(vec2<f32>(ar[base+i],ai[base+i]),table[p.n+i]);
    ar[base+i]=a.x; ai[base+i]=a.y;
  } else if (p.phase==2u*levels+2u) {
    if (i<p.n) {
      let a=mul(vec2<f32>(ar[base+i],ai[base+i])/f32(p.m),table[i]);
      re[beam*p.n+i]=a.x; im[beam*p.n+i]=a.y;
    }
  } else {
    if (i>=p.m/2u) { return; }
    let forward=p.phase<=levels;
    var stride: u32;
    if (forward) { stride=p.m>>(p.phase-1u); }
    else { stride=1u<<(p.phase-levels-1u); }
    let half=stride/2u; let j=i%half;
    let aidx=base+(i/half)*stride+j; let bidx=aidx+half;
    let a=vec2<f32>(ar[aidx],ai[aidx]);
    let b=vec2<f32>(ar[bidx],ai[bidx]);
    let w=table[p.n+p.m+j*(p.m/stride)];
    var u: vec2<f32>; var v: vec2<f32>;
    if (forward) { u=a+b; v=mul(a-b,w); }
    else { let t=mul(b,vec2<f32>(w.x,-w.y)); u=a+t; v=a-t; }
    ar[aidx]=u.x; ai[aidx]=u.y; ar[bidx]=v.x; ai[bidx]=v.y;
  }
}
