# SPDX-License-Identifier: Apache-2.0
"""Verify preserved full native arrays and the published pulse comparison."""
from pathlib import Path
import hashlib
import json
import numpy as np

HERE = Path(__file__).resolve().parent
for kind, count in [("before", 118), ("after-owner", 43), ("after-integrated", 43)]:
    result = json.loads((HERE / kind / "result.json").read_text())
    path = HERE / kind / "arrays.npz"
    assert hashlib.sha256(path.read_bytes()).hexdigest() == result["arrays_sha256"]
    assert result["calls"] == count
    if kind == "before":
        assert result["invalid_contract"]["accepted"]
        continue
    assert result["invalid_period_rejected"]
    a = np.load(path)
    passed = 0
    for row in result["rows"]:
        z, expected = a[row["key"]], a[row["key"] + "_discrete"]
        continuous = a[row["key"] + "_continuous"]
        discrete_error = np.linalg.norm(z - expected) / np.linalg.norm(expected)
        physical_error = np.linalg.norm(z[:599] - continuous) / np.linalg.norm(continuous)
        assert abs(discrete_error - row["relative_complex_discrete_error"]) < 1e-12
        assert abs(physical_error - row["relative_complex_continuous_error"]) < 1e-12
        passed += discrete_error < 0.003 and physical_error < 0.003
        assert len(z) == row["m"]
    assert passed == result["pass_count"] == (33 if kind == "after-owner" else 42)
    assert result["case_count"] == 42
native = json.loads((HERE / "native.json").read_text())
assert native["passed"] and native["error"] is None
assert len(native["matched_stamps_ns"]) >= 4
assert all(r["exit"] == 0 and not r["forced"] for r in native["cleanup"])
assert (
    hashlib.sha256((HERE / "native.npz").read_bytes()).hexdigest()
    == native["native_arrays_sha256"]
)
a = np.load(HERE / "native.npz")
for row in native["rows"]:
    v = a[row["key"]]
    assert list(v.shape) == row["shape"]
    if row["kind"] != "depth":
        assert hashlib.sha256(v.tobytes()).hexdigest() == row["raw_payload_sha256"]
    else:
        valid = np.isfinite(v).all(axis=-1)
        assert abs(v[..., 0][valid] - 4).max() < 0.05
print(
    "PASS: full pulse arrays, independent references, retained integer failures, native packet provenance"
)
