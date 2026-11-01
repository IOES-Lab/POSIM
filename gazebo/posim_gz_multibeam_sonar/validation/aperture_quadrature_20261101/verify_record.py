# SPDX-License-Identifier: Apache-2.0
"""Check every retained point transfer, beam moment and stride observation."""
import csv
import hashlib
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parent
MODELS = ("source", "fixed_global", "fixed_row_l2", "fixed_peak_abs", "fixed_peak_signed")


def main():
    manifest = json.loads((ROOT / "manifest.json").read_text())
    for name, digest in manifest["files"].items():
        assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest, name
    result = json.loads((ROOT / "result.json").read_text())
    protocol = json.loads((ROOT / "predeclared.json").read_text())
    arrays = np.load(ROOT / "profiles-and-moments.npz")
    point_rows = {(r["n"], r["model"]): r for r in result["rows"] if r["kind"] == "point"}
    for n in (65, 129, 257):
        angles = arrays[f"angles_{n}"]
        center = n // 2
        # The C++ source subtracts float angles before evaluating its sinc.
        delta = (angles[:, None] - angles[None, :]).astype(float)
        source = np.abs(np.sinc(0.884 / (protocol["hfov_rad"] / (n - 1)) * np.sin(delta)))
        delta = angles[:, None].astype(float) - angles[None, :].astype(float)
        scale = 1.39155737825151 / math.sin(protocol["synthetic_power_fwhm_rad"] / 2)
        signed = np.sinc(scale * np.sin(delta) / np.pi)
        magnitude = np.abs(signed)
        models = {
            "source": source / point_rows[n, "source"]["normalization"],
            "fixed_global": magnitude / np.linalg.norm(magnitude),
            "fixed_row_l2": magnitude / np.linalg.norm(magnitude, axis=1)[:, None],
            "fixed_peak_abs": magnitude,
            "fixed_peak_signed": signed,
        }
        for frame in (20, 21):
            baseline = arrays[f"baseline_{n}_{frame}"]
            assert baseline.shape == (64, 2) and np.isfinite(baseline).all()
            for name, operator in models.items():
                actual = arrays[f"transfer_{n}_{frame}_{name}"]
                expected = operator[:, center]
                assert np.max(np.abs(actual - expected)) < 5e-5 * np.max(np.abs(expected))
        # Independently evaluate the conditional Gaussian energy expectation.
        depth = (5.0 / np.cos(angles.astype(float))).astype(np.float32).astype(float)
        cosine = np.cos(angles).astype(float)
        area = protocol["hfov_rad"] / (n - 1) * protocol["vfov_rad"] / 2
        weights = np.ones(n)
        weights[[0, -1]] = 0.5
        variance = 1e6 * area * weights * cosine**2 / depth**2
        for name, operator in models.items():
            expected = 64 * 50**2 * (operator**2 @ variance)
            saved = arrays[f"expected_{n}_{name}"]
            np.testing.assert_allclose(saved, expected, rtol=2e-5)
            selected = arrays[f"selected_energy_{n}_{name}"]
            assert selected.shape == (256, 3) and np.isfinite(selected).all()
            np.testing.assert_allclose(
                selected.mean(axis=0),
                arrays[f"mean_energy_{n}_{name}"][[n // 4, center, 3 * n // 4]],
            )
            np.testing.assert_allclose(
                selected.std(axis=0, ddof=1) / 16,
                arrays[f"sem_energy_{n}_{name}"][[n // 4, center, 3 * n // 4]],
            )
            ratios = arrays[f"mean_energy_{n}_{name}"] / expected
            assert np.all(ratios >= 1 - result["lower_relative_bound"])
            assert np.all(ratios <= 1 + result["upper_relative_bound"])
    for name in ("fixed_peak_abs", "fixed_peak_signed"):
        coarse = arrays[f"expected_65_{name}"][32]
        fine = arrays[f"expected_257_{name}"][128]
        assert abs(fine / coarse - 1) < 5e-5
    with (ROOT / "stride-results.csv").open() as stream:
        rows = list(csv.DictReader(stream))
    assert len(rows) == 42
    energies = {(r["backend"], int(r["stride"]), int(r["row"])): float(r["energy"]) for r in rows}
    for backend in ("cpu", "wgpu"):
        for stride in (1, 2, 3):
            for row in range(7):
                energy = energies[backend, stride, row]
                selected = backend == "cpu" or row % stride == 0
                assert (energy > 0) == selected
                if selected:
                    assert math.isclose(
                        energy / energies[backend, 1, row], (stride + 1) / 2, rel_tol=2e-5
                    )
    print(
        "PASS: complete point transfers, 2255 beam moments, selected frame energies and 42 stride cases"
    )


if __name__ == "__main__":
    main()
