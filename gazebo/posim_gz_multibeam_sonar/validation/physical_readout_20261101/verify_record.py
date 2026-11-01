# SPDX-License-Identifier: Apache-2.0
"""Check retained native measurements without the private app or a GPU."""
from pathlib import Path
import hashlib
import json
import math
import numpy as np

root = Path(__file__).resolve().parent
result = json.loads((root / "result.json").read_text())
raw = root / "arrays.npz"
assert hashlib.sha256(raw.read_bytes()).hexdigest() == result["arrays_sha256"]
data = np.load(raw)
outputs = np.array(result["outputs"])
width = result["synthetic_power_fwhm_radians"]
expectations = []
for item in result["plane_cases"]:
    count, stride = item["geometry"], item["stride"]
    azimuth = np.linspace(-0.3, 0.3, count)
    rows = sorted(set(range(0, 7, stride)) | {6})
    elevation = np.linspace(-0.1, 0.1, 7)[rows]
    az_edges = np.r_[-0.3, (azimuth[:-1] + azimuth[1:]) / 2, 0.3]
    el_edges = np.r_[-0.1, (elevation[:-1] + elevation[1:]) / 2, 0.1]
    area = np.diff(az_edges)[:, None] * np.diff(np.sin(el_edges))[None, :]
    incidence = np.cos(azimuth)[:, None] * np.cos(elevation)[None, :]
    distance = 5 / incidence
    cell_power = 1e6 * np.exp(-4 * 0.03 * distance) / distance**2 * 0.7 * area * incidence**2
    phase = 1.39155737825151 / math.sin(width / 2) * np.sin(azimuth[None, :] - outputs[:, None])
    response = np.sinc(phase / np.pi)
    expected = 3200**2 / 64 * (response**2 @ cell_power.sum(axis=1))
    key = f"g{count}_s{stride}"
    np.testing.assert_allclose(expected, data[key + "_expected"], rtol=2e-7)
    powers = data[key + "_powers"]
    assert powers.shape == (result["frames_per_case"], len(outputs))
    assert np.isfinite(powers).all() and (powers >= 0).all()
    for frame in [0, result["frames_per_case"] - 1]:
        signal = data[f"{key}_complex_{frame}"].astype(np.complex128)
        np.testing.assert_allclose(np.sum(abs(signal) ** 2, axis=1), powers[frame], rtol=2e-7)
    np.testing.assert_allclose(powers.mean(axis=0), item["native_mean"], rtol=1e-12)
    ratio = powers.mean(axis=0) / expected - 1
    lower, upper = result["relative_mean_bounds"]
    assert (ratio > lower).all() and (ratio < upper).all()
    expectations.append(expected)
relative_change = float(np.max(abs(np.array(expectations) / expectations[-1] - 1)))
assert relative_change < 0.01
np.testing.assert_allclose(
    relative_change, result["maximum_conditional_quadrature_change"], rtol=1e-10
)
for count in [65, 129, 257]:
    signal = data[f"point_g{count}"]
    center = signal[len(outputs) // 2]
    peak = abs(center).argmax()
    expected = np.sinc(1.39155737825151 / math.sin(width / 2) * np.sin(-outputs) / np.pi)
    assert np.max(abs(signal[:, peak] / center[peak] - expected)) < 1e-5
print("PASS: all native plane energies, retained complex returns, signed response and quadrature")

comparison = json.loads((root / "comparison-result.json").read_text())
assert (
    hashlib.sha256((root / "comparison-data.npz").read_bytes()).hexdigest()
    == comparison["data_sha256"]
)
matched = np.load(root / "comparison-data.npz")
previous = {name[7:]: matched[name] for name in matched.files if name.startswith("before_")}
for item in comparison["after"]:
    count = item["geometry"]
    means = matched[f"energies_{count}"].mean(axis=0)
    np.testing.assert_allclose(means[150], item["native_mean_on_axis"], rtol=1e-12)
    np.testing.assert_allclose(
        matched[f"expected_{count}"][150],
        previous[f"expected_{count}_fixed_peak_signed"][count // 2],
        rtol=1e-6,
    )
    signal = matched[f"point_{count}"]
    expected = np.sinc(
        1.39155737825151
        / math.sin(math.radians(1.5) / 2)
        * np.sin(-matched["output_angles"])
        / np.pi
    )
    peak = abs(signal[150]).argmax()
    assert np.max(abs(signal[:, peak] / signal[150, peak] - expected)) < 1e-5
print("PASS: matched comparison data, geometry support and fixed point response")
