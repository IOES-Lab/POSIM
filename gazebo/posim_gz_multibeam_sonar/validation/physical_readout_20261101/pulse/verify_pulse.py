# SPDX-License-Identifier: Apache-2.0
"""Source-bound native controls against independent analytic baseband pressure."""
from pathlib import Path
import ctypes as C
import hashlib
import json
import sys
import time
import numpy as np

BINARY, OUT = Path(sys.argv[1]).resolve(), Path(sys.argv[2])
OUT.mkdir()
lib = C.CDLL(str(BINARY))
ptr = C.POINTER(C.c_float)
compute = lib.sonar_wgpu_compute_v2
compute.argtypes = [ptr] * 6 + [C.c_uint32] * 4 + [C.c_float] * 6 + [C.c_uint64] * 2 + [C.c_bool]
compute.restype = ptr
lib.sonar_wgpu_free.argtypes = [ptr, C.c_size_t]


class DlInfo(C.Structure):
    _fields_ = [
        ("path", C.c_char_p),
        ("base", C.c_void_p),
        ("symbol", C.c_char_p),
        ("address", C.c_void_p),
    ]


info = DlInfo()
dl = C.CDLL(None).dladdr
dl.argtypes = [C.c_void_p, C.POINTER(DlInfo)]
assert dl(C.cast(compute, C.c_void_p), C.byref(info))
assert Path(info.path.decode()).resolve() == BINARY
B, SPEED = 29900.0, 1500.0
DR = SPEED / (2 * B)
N = 599
RANGES = np.arange(N) * DR
arrays = {"published_ranges_m": RANGES}
rows = []
calls = 0


def noise(index):
    # Independent integer Philox4x32-10; no fitted native amplitude.
    mask = 2**32 - 1
    c = [20, 0, index, 0]
    k0, k1 = 73, 0
    for _ in range(10):
        a, b = 0xD2511F53 * c[0], 0xCD9E8D57 * c[2]
        c = [((b >> 32) ^ c[1] ^ k0) & mask, b & mask, ((a >> 32) ^ c[3] ^ k1) & mask, a & mask]
        k0, k1 = (k0 + 0x9E3779B9) & mask, (k1 + 0xBB67AE85) & mask
    u1, u2 = (np.array(c[:2], np.float64) + 0.5) / 2**32
    radius, angle = np.sqrt(-2 * np.log(u1)), 2 * np.pi * u2
    return radius * (np.sin(angle) + 1j * np.cos(angle)) / np.sqrt(2)


def call(depths, window, source=180.0, maximum=15.0, allow_reject=False):
    global calls
    depth = np.array(depths, np.float32).reshape(1, -1)
    normal = np.zeros((*depth.shape, 3), np.float32)
    normal[..., 2] = 1
    values = [
        depth,
        normal,
        np.ones_like(depth),
        np.full_like(depth, 1e-4),
        np.ones((1, 1), np.float32),
        np.array(window, np.float32),
    ]
    values = [np.ascontiguousarray(v) for v in values]
    m = len(window)
    t = time.monotonic()
    raw = compute(
        *(v.ctypes.data_as(ptr) for v in values),
        1,
        depth.shape[1],
        1,
        m,
        SPEED,
        maximum,
        source,
        0.03,
        1.0,
        B,
        20,
        73,
        False,
    )
    elapsed = time.monotonic() - t
    calls += 1
    if not raw:
        assert allow_reject, "unexpected native rejection"
        return None, elapsed
    try:
        z = np.ctypeslib.as_array(raw, shape=(m * 2,)).copy().reshape(m, 2)
    finally:
        lib.sonar_wgpu_free(raw, m * 2)
    z = z[:, 0] + 1j * z[:, 1]
    assert np.isfinite(z).all()
    return z, elapsed


def reference(depths, m, source):
    f = B * ((np.arange(m) + 0.5) / m - 0.5)
    shape = ((0.54 + 0.46 * np.cos(2 * np.pi * f / B)) / 0.54).astype(np.float32)
    amplitudes = []
    for index, d in enumerate(depths):
        r = float(np.float32(d))
        amplitudes.append(
            noise(index)
            * 10 ** (source / 20)
            * 1e-6
            * np.exp(-2 * float(np.float32(0.03)) * r)
            / r
            * np.sqrt(float(np.float32(1e-4)))
        )
    spectrum = sum(
        a * np.exp(4j * np.pi * f * float(np.float32(r)) / SPEED)
        for r, a in zip(depths, amplitudes)
    )
    k = np.arange(m)
    discrete = np.fft.fft(spectrum * shape) / m * (-1.0) ** k * np.exp(-1j * np.pi * k / m)
    continuous = np.zeros(N, np.complex128)
    for r, a in zip(depths, amplitudes):
        x = 2 * B * (float(np.float32(r)) - RANGES) / SPEED
        continuous += a * (np.sinc(x) + 0.23 / 0.54 * (np.sinc(x - 1) + np.sinc(x + 1)))
    return shape, discrete, continuous


started = time.monotonic()
for source in [180.0, 220.0]:
    for m in [1024, 2048, 4096]:
        for depths in [[0.2], [0.21], [4.0], [14.95], [14.99], [15.0], [4.0, 15.0]]:
            shape, expected, continuous = reference(depths, m, source)
            z, elapsed = call(depths, shape, source)
            key = f"s{int(source)}_m{m}_r" + "_".join(map(str, depths))
            arrays[key] = z
            arrays[key + "_discrete"] = expected
            arrays[key + "_continuous"] = continuous
            arrays[f"m{m}_shape"] = shape
            power = abs(z[:N].astype(np.complex128)) ** 2
            reference_error = float(np.linalg.norm(z - expected) / np.linalg.norm(expected))
            physical_error = float(np.linalg.norm(z[:N] - continuous) / np.linalg.norm(continuous))
            rows.append(
                dict(
                    key=key,
                    source_db=source,
                    m=m,
                    ranges_m=depths,
                    relative_complex_discrete_error=reference_error,
                    relative_complex_continuous_error=physical_error,
                    passed=reference_error < 0.003 and physical_error < 0.003,
                    peak_range_m=float(RANGES[np.argmax(power)]),
                    total_output_power=float(power.sum()),
                    total_transform_power=float(np.sum(abs(z.astype(np.complex128)) ** 2)),
                    near_half_meter_power_fraction=float(power[RANGES < 0.5].sum() / power.sum()),
                    elapsed_seconds=elapsed,
                )
            )
# Full-period violation rejected before backend buffers are read.
shape, _, _ = reference([15.4], 512, 180.0)
invalid, _ = call([15.4], shape, maximum=15.5, allow_reject=True)
invalid_rejected = invalid is None
if invalid is not None:
    arrays["invalid_contract"] = invalid
ratios = []
for source in [180.0, 220.0]:
    sample = [r for r in rows if r["source_db"] == source and r["ranges_m"] == [4.0]]
    baseline = next(r["total_transform_power"] for r in sample if r["m"] == 2048)
    ratios.append(
        dict(
            source_db=source,
            full_transform_energy_ratios={
                r["m"]: r["total_transform_power"] / baseline for r in sample
            },
        )
    )
np.savez_compressed(OUT / "arrays.npz", **arrays)
result = dict(
    calls=calls,
    binary=str(BINARY),
    binary_sha256=hashlib.sha256(BINARY.read_bytes()).hexdigest(),
    symbol_library=info.path.decode(),
    elapsed_seconds=time.monotonic() - started,
    rows=rows,
    invalid_period_rejected=invalid_rejected,
    energy_ratios=ratios,
    pass_count=sum(r["passed"] for r in rows),
    case_count=len(rows),
    arrays_sha256=hashlib.sha256((OUT / "arrays.npz").read_bytes()).hexdigest(),
)
(OUT / "result.json").write_text(json.dumps(result, indent=2) + "\n")
print(json.dumps({k: v for k, v in result.items() if k != "rows"}, indent=2))
print(
    "failures",
    [
        (r["key"], r["relative_complex_discrete_error"], r["relative_complex_continuous_error"])
        for r in rows
        if not r["passed"]
    ],
)
raise SystemExit(0 if all(r["passed"] for r in rows) and invalid_rejected else 1)
