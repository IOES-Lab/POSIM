# SPDX-License-Identifier: Apache-2.0
"""Verify every retained complex sample and the fixed angular-cell range law."""
import hashlib
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parent


def main():
    manifest = json.loads((ROOT / "manifest.json").read_text())
    for name, digest in manifest["files"].items():
        assert hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest, name
    records = {name: np.load(ROOT / f"{name}.npz") for name in ("before", "after")}
    report = json.loads((ROOT / "result.json").read_text())
    assert set(records["before"].files) == set(records["after"].files)
    assert len(records["before"].files) == 24
    maximum_error = 0.0
    for frame in (20, 21):
        powers = {}
        for version, arrays in records.items():
            powers[version] = {}
            for key in arrays.files:
                value = arrays[key]
                assert value.shape == (3, 599, 2) and np.isfinite(value).all(), key
                if key.startswith(f"{frame}_"):
                    powers[version][key.split("_", 1)[1]] = float(
                        np.square(value.astype(np.float64)).sum()
                    )
            saved = report[version]["rows"][frame - 20]["powers"]
            for name, value in powers[version].items():
                assert math.isclose(value, saved[name], rel_tol=1e-12, abs_tol=1e-30)
        old, new = powers["before"], powers["after"]
        for name, distance in (("range_3", 3.0), ("range_10", 10.0), ("range_15", 15.0)):
            assert math.isclose(old[name] / old["baseline"], (5 / distance) ** 3, rel_tol=5e-4)
            assert math.isclose(new[name] / new["baseline"], (5 / distance) ** 2, rel_tol=5e-4)
        assert math.isclose(new["fixed_surface_10"] / new["baseline"], 1 / 16, rel_tol=5e-4)
        assert math.isclose(
            new["absorbed_10"] / new["absorbed_5"], 0.25 * math.exp(-1), rel_tol=5e-4
        )
        for name in ("area_x4", "reflectivity_x4"):
            assert math.isclose(new[name] / new["baseline"], 4, rel_tol=5e-4)
        assert old["no_return"] == new["no_return"] == 0
        assert np.array_equal(
            records["after"][f"{frame}_baseline"], records["after"][f"{frame}_changed_steering"]
        )
    for key in records["before"].files:
        name = key.split("_", 1)[1]
        distance = {
            "range_3": 3.0,
            "range_10": 10.0,
            "range_15": 15.0,
            "fixed_surface_10": 10.0,
            "absorbed_10": 10.0,
            "no_return": 0.0,
        }.get(name, 5.0)
        expected = records["before"][key].astype(np.float64) * math.sqrt(distance)
        denominator = np.linalg.norm(expected)
        difference = np.linalg.norm(records["after"][key] - expected)
        relative_error = float(difference / denominator) if denominator else float(difference)
        maximum_error = max(maximum_error, relative_error)
        assert relative_error < 1e-6, key
    assert report["before"]["physical_range_contract_passed"] is False
    assert report["after"]["physical_range_contract_passed"] is True
    print(f"PASS: all 48 complex arrays; maximum relative amplitude error {maximum_error:.3g}")


if __name__ == "__main__":
    main()
